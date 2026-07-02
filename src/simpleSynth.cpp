#include "simpleSynth.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/core/object.hpp"
#include "simpleSynthPatch.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

using namespace godot;

void SimpleSynthStream::set_sequencer_settings(const Ref<SimpleSynthSequencerSettings> newSets){
    sequencerSettings = newSets;
}

Ref<SimpleSynthSequencerSettings> SimpleSynthStream::get_sequencer_settings() const{
    return sequencerSettings;
}

void SimpleSynthStream::set_patch(const Ref<SimpleSynthPatch> newPatch){
    myPatch = newPatch;
}

Ref<SimpleSynthPatch> SimpleSynthStream::get_patch() const{
    return myPatch;
}

Ref<AudioStreamPlayback> SimpleSynthStream::_instantiate_playback() const{
    Ref<SimpleSynthPlayback> pb;
    pb.instantiate();
    pb->set_stream(this);
    pb->patch = myPatch;
    pb->sequencerSettings = sequencerSettings;
    pb->advanceStep(0);
    return pb;
}

void SimpleSynthStream::set_sequencer_enabled(bool x){
    sequencer_enabled  = x;
}

bool SimpleSynthStream::get_sequencer_enabled() const{
    return sequencer_enabled;
}

void SimpleSynthStream::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_sequencer_enabled", "enabled"), &SimpleSynthStream::set_sequencer_enabled);
    ClassDB::bind_method(D_METHOD("get_sequencer_enabled"), &SimpleSynthStream::get_sequencer_enabled);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "Sequencer Enabled"), "set_sequencer_enabled", "get_sequencer_enabled");
    
    ClassDB::bind_method(D_METHOD("set_patch","New Patch"),&SimpleSynthStream::set_patch);
    ClassDB::bind_method(D_METHOD("get_patch"),&SimpleSynthStream::get_patch);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"patch",PROPERTY_HINT_RESOURCE_TYPE,"SimpleSynthPatch"), "set_patch", "get_patch");

    ClassDB::bind_method(D_METHOD("set_sequencer_settings","New Sequencer"),&SimpleSynthStream::set_sequencer_settings);
    ClassDB::bind_method(D_METHOD("get_sequencer_settings"),&SimpleSynthStream::get_sequencer_settings);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"sequencer settings",PROPERTY_HINT_RESOURCE_TYPE,"SimpleSynthSequencerSettings"),"set_sequencer_settings","get_sequencer_settings");
};



///PLAYBACK///

int SimpleSynthPlayback::_mix(godot::AudioFrame *buffer, float rate_scale, int frames) 
{
    if(patch.is_null()){
        return 0;
    }
    sequencer_enabled = myStream->get_sequencer_enabled();
    for (int i=0; i<frames;i++){
        processSequencer(); // Check the sequencer for Stuff.
        float output = patch->process();
        buffer[i] = {output,output};
    }
    return frames;
};


void SimpleSynthPlayback::processSequencer(){

    //apply smoothed offsets, also naturally resets to zero when off.
    currentAmplitudeOffset += (targetAmplitudeOffset - currentAmplitudeOffset) * 0.005f;
    currentFrequencyOffset += (targetFrequencyOffset - currentFrequencyOffset) * 0.005f;
    
    patch->amplitudeOffset = currentAmplitudeOffset;
    patch->frequencyOffset = currentFrequencyOffset;



    if(!sequencer_enabled && !noteHasTriggeredThisStep && !noteHasReleasedThisStep){
        // targetAmplitudeOffset=0.0f;
        // targetFrequencyOffset=0.0f;
        return; //Do nuthin if we're turned off.
    }

    //Advance phase
    float step_increment = sequencerSettings->BPM*sequencerSettings->notes_per_beat/(60.0f*sampleRate);
    phase += step_increment;

    //trigger notes.
    if(phase>=triggerPhase && !noteHasTriggeredThisStep){
        //Advance sequencer adjustment
        currentFixedSequenceStep++;

        if(currentFixedSequenceStep>=sequencerSettings->fixed_step_variation.size()){
            currentFixedSequenceStep=0;
        }

        //Apply random variables. We do this now instead of on stepover because that'd cause sudden frequency and amplitude changes.
        targetAmplitudeOffset = UtilityFunctions::randf_range(-sequencerSettings->random_amplitude,sequencerSettings->random_amplitude)+sequencerSettings->fixed_step_variation[currentFixedSequenceStep].y;
        
        float r = UtilityFunctions::randf_range(-1.0f,1.0f);
        r*= Math::abs(r);
        float randomSemitones = r*sequencerSettings->random_pitch;
        float frequencyAdjust = randomSemitones+sequencerSettings->fixed_step_variation[currentFixedSequenceStep].x;
        float freqeuncyRatio =  std::pow(2.0f, frequencyAdjust / 12.0f);
        targetFrequencyOffset = freqeuncyRatio;

        noteHasTriggeredThisStep = true; //Disable repeat triggers
        patch->note_on();
        total_notes++;
    }
    if(phase>=releasePhase && !noteHasReleasedThisStep){
        noteHasReleasedThisStep = true; //Disable repeat releases
        patch->note_off();
    }

    //phase loop section
    if(phase>=1.0f){
        phase-=1.0f;
        currentSequencerStep++;
        if(currentSequencerStep>=sequencerSettings->notes_per_beat){
            currentSequencerStep = 0;
        }
        advanceStep(currentSequencerStep);
    }

}

void SimpleSynthPlayback::advanceStep(int newStep){
    //Calculate triggers
    float randomDelay = sequencerSettings->random_delay>0.0f? UtilityFunctions::randf_range(-sequencerSettings->random_delay,sequencerSettings->random_delay):0.0;
    float releasePhaseOffset;
    if(sequencerSettings->scale_note_on){
        releasePhaseOffset = sequencerSettings->note_on_duration;
    }
    else{
        releasePhaseOffset = sequencerSettings->note_on_duration*((sequencerSettings->BPM*sequencerSettings->notes_per_beat)/60.0f); //TODO: Double check this math is right.
    }
    triggerPhase = sequencerSettings->fixed_step_variation[currentFixedSequenceStep].z+randomDelay;

    releasePhase = triggerPhase+releasePhaseOffset + randomDelay;

    //Release note blockers.
    noteHasReleasedThisStep=false;
    noteHasTriggeredThisStep=false;
}


void SimpleSynthPlayback::_bind_methods(){
    
}


////////////// GIANT FUCKING BLOCK OF BULLSHIT SETTERS AND GETTERS FOR SEQUENCER SETTINGS /////////////
void SimpleSynthSequencerSettings::set_bpm(int new_bpm) {
    BPM = new_bpm;
}
void SimpleSynthSequencerSettings::set_notes_per_beat(int new_notes_per_beat) {
    notes_per_beat = new_notes_per_beat;
}
void SimpleSynthSequencerSettings::set_note_on_duration(float new_duration) {
    note_on_duration = new_duration;
}
void SimpleSynthSequencerSettings::set_scale_note_on(bool new_scale) {
    scale_note_on = new_scale;
}
void SimpleSynthSequencerSettings::set_random_pitch(float new_random_pitch) {
    random_pitch = new_random_pitch;
}
void SimpleSynthSequencerSettings::set_random_amplitude(float new_random_amplitude) {
    random_amplitude = new_random_amplitude;
}
void SimpleSynthSequencerSettings::set_random_delay(float new_random_delay) {
    random_delay = new_random_delay;
}
void SimpleSynthSequencerSettings::set_fixed_step_variation(
        const godot::PackedVector3Array &new_steps) {
    fixed_step_variation = new_steps;
}
int SimpleSynthSequencerSettings::get_bpm() const {
    return BPM;
}
int SimpleSynthSequencerSettings::get_notes_per_beat() const {
    return notes_per_beat;
}
float SimpleSynthSequencerSettings::get_note_on_duration() const {
    return note_on_duration;
}
bool SimpleSynthSequencerSettings::get_scale_note_on() const {
    return scale_note_on;
}
float SimpleSynthSequencerSettings::get_random_pitch() const {
    return random_pitch;
}
float SimpleSynthSequencerSettings::get_random_amplitude() const {
    return random_amplitude;
}
float SimpleSynthSequencerSettings::get_random_delay() const {
    return random_delay;
}
godot::PackedVector3Array SimpleSynthSequencerSettings::get_fixed_step_variation() const {
    return fixed_step_variation;
}

void SimpleSynthSequencerSettings::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_bpm", "Beats Per Minute"), &SimpleSynthSequencerSettings::set_bpm);
    ClassDB::bind_method(D_METHOD("get_bpm"), &SimpleSynthSequencerSettings::get_bpm);

    ClassDB::bind_method(D_METHOD("set_notes_per_beat", "Notes Per Beat"), &SimpleSynthSequencerSettings::set_notes_per_beat);
    ClassDB::bind_method(D_METHOD("get_notes_per_beat"), &SimpleSynthSequencerSettings::get_notes_per_beat);

    ClassDB::bind_method(D_METHOD("set_note_on_duration", "Duration"), &SimpleSynthSequencerSettings::set_note_on_duration);
    ClassDB::bind_method(D_METHOD("get_note_on_duration"), &SimpleSynthSequencerSettings::get_note_on_duration);

    ClassDB::bind_method(D_METHOD("set_scale_note_on", "Use Scaled Note Duration"), &SimpleSynthSequencerSettings::set_scale_note_on);
    ClassDB::bind_method(D_METHOD("get_scale_note_on"), &SimpleSynthSequencerSettings::get_scale_note_on);

    ClassDB::bind_method(D_METHOD("set_random_pitch", "Random Pitch"), &SimpleSynthSequencerSettings::set_random_pitch);
    ClassDB::bind_method(D_METHOD("get_random_pitch"), &SimpleSynthSequencerSettings::get_random_pitch);

    ClassDB::bind_method(D_METHOD("set_random_amplitude", "Random Amplitude"), &SimpleSynthSequencerSettings::set_random_amplitude);
    ClassDB::bind_method(D_METHOD("get_random_amplitude"), &SimpleSynthSequencerSettings::get_random_amplitude);

    ClassDB::bind_method(D_METHOD("set_random_delay", "Random Note Delay"), &SimpleSynthSequencerSettings::set_random_delay);
    ClassDB::bind_method(D_METHOD("get_random_delay"), &SimpleSynthSequencerSettings::get_random_delay);

    ClassDB::bind_method(D_METHOD("set_fixed_step_variation", "Fixed Step Variation"), &SimpleSynthSequencerSettings::set_fixed_step_variation);
    ClassDB::bind_method(D_METHOD("get_fixed_step_variation"), &SimpleSynthSequencerSettings::get_fixed_step_variation);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "Beats Per Minute", PROPERTY_HINT_RANGE, "1,1000,1"), "set_bpm", "get_bpm");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "Notes Per Beat", PROPERTY_HINT_RANGE, "1,32,1"), "set_notes_per_beat", "get_notes_per_beat");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Note On Duration", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_note_on_duration", "get_note_on_duration");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "Scale Note Duration as portion of step duration"), "set_scale_note_on", "get_scale_note_on");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Random Pitch Amount - Semitones", PROPERTY_HINT_RANGE, "0.0,24.0,0.01"), "set_random_pitch", "get_random_pitch");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Random Amplitude", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_random_amplitude", "get_random_amplitude");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Random Note Delay", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_random_delay", "get_random_delay");
    ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "Fixed Step Variation -Pitch, Amplitude, Delay"), "set_fixed_step_variation", "get_fixed_step_variation");
}
