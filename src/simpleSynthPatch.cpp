#include "simpleSynthPatch.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/defs.hpp"
#include "godot_cpp/core/math.hpp"
#include "godot_cpp/core/property_info.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include <cstdlib>

using namespace godot;

void SynthParameterSource::initialize(SynthPatchLocals *l){
    synthLocals = l;
}

void SynthParameterSource::_bind_methods(){
    //no methods for godot
}

void SynthConstantParameter::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_output","output"), &SynthConstantParameter::set_output);
    ClassDB::bind_method(D_METHOD("get_output"), &SynthConstantParameter::get_output);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Parameter Value"),"set_output","get_output");
}


void SynthLFO::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_rate","LFO Rate"), &SynthLFO::set_rate);
    ClassDB::bind_method(D_METHOD("get_rate"), &SynthLFO::get_rate);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"LFO rate (hz)"),"set_rate","get_rate");
}

void SynthOscillator::initialize(SynthPatchLocals *l){
    synthLocals = l;
}

void SynthOscillator::note_on(){
    active = true;
}

void SynthOscillator::note_off(){
    active=false;
}

void SynthOscillator::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_gain","gain"),&SynthOscillator::set_gain);
    ClassDB::bind_method(D_METHOD("get_gain"),&SynthOscillator::get_gain);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Gain"),"set_gain","get_gain");
    //OTHERWISE_BLANK BECAUSE IT IS AN ABSTRACT!
}

// static float peak = 0.0f;
// int counter = 0;
float SynthNoiseOscillator::process(){
    white = ((float)rand()/RAND_MAX)*2.0f-1.0f;
    
    brown *= 0.9;
	brown += white * 0.01;
    
    //Paul Kellett economy pink noise filter.
    pink_b0 = 0.99765 * pink_b0 + white * 0.0990460;
    pink_b1 = 0.96300 * pink_b1 + white * 0.2965164;
    pink_b2 = 0.57000 * pink_b2 + white * 1.0526913;
    pink = pink_b0 + pink_b1 + pink_b2 + white * 0.1848;

    pink *= 0.12f; //normalize pink noise - number pulled out of slopGeminiPT's ass but seems to work?

    float out = white*noise_mix.x + pink*noise_mix.y + brown*noise_mix.z;

    // peak = MAX(peak, fabs(out));

    // if(counter++ % 5000 == 0)
    //     print_line(peak);

    return out;
    // return white*noise_mix.x+pink*noise_mix.y+brown*noise_mix.z;
};

void SynthNoiseOscillator::set_noise_mix(const Vector3 newMix){
    noise_mix = newMix;
}

Vector3 SynthNoiseOscillator::get_noise_mix(){
    return noise_mix;
}

void SynthNoiseOscillator::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_noise_mix","Noise Mix (W,P,B)"), &SynthNoiseOscillator::set_noise_mix);
    ClassDB::bind_method(D_METHOD("get_noise_mix"), &SynthNoiseOscillator::get_noise_mix);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3,"Noise Mix (W,P,B)"),"set_noise_mix","get_noise_mix");
}


float SynthFrequencyOscillator::processPitch(){
    float lfoPitch = 0.0f;
    if(lfo.is_valid()){lfoPitch = lfo->process();} //if no LFO then don't LFO.
    lfoPitch *= lfo_depth;
    float pitchbendTotal = 0.0f;
    if(synthLocals)pitchbendTotal = synthLocals->pitchBend*pitchbendRange;

    float semitones = lfoPitch+semitone_offset+pitchbendTotal;

    //Convert LFO result to semitones.
    if(semitones==0.0f){return 1.0f;}
    return SynthHelper::semitones_to_ratio(semitones);
}



void SynthFrequencyOscillator::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_freq","Oscillator Frequency (hz)"), &SynthFrequencyOscillator::set_frequency);
    ClassDB::bind_method(D_METHOD("get_freq"), &SynthFrequencyOscillator::get_frequency);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Oscillator Frequency (hz)"),"set_freq","get_freq");

    ClassDB::bind_method(D_METHOD("set_lfo","LFO"), &SynthFrequencyOscillator::set_lfo);
    ClassDB::bind_method(D_METHOD("get_lfo"),&SynthFrequencyOscillator::get_lfo);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"LFO(Low Frequency Oscillator)",PROPERTY_HINT_RESOURCE_TYPE,"SynthLFO"), "set_lfo", "get_lfo");
    
    ClassDB::bind_method(D_METHOD("set_lfo_depth","LFO Depth"), &SynthFrequencyOscillator::set_lfo_depth);
    ClassDB::bind_method(D_METHOD("get_lfo_depth"), &SynthFrequencyOscillator::get_lfo_depth);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"LFO Frequency span (semitones)"),"set_lfo_depth","get_lfo_depth");
    
    ClassDB::bind_method(D_METHOD("set_note","Note"), &SynthFrequencyOscillator::set_note);
    ClassDB::bind_method(D_METHOD("get_note"), &SynthFrequencyOscillator::get_note);
    ADD_PROPERTY(PropertyInfo(Variant::INT,"Oscillator Note",PROPERTY_HINT_ENUM,"C,C#,D,D#,E,F,F#,G,G#,A,A#,B"),"set_note","get_note");

    ClassDB::bind_method(D_METHOD("set_oct","Octave"), &SynthFrequencyOscillator::set_octave);
    ClassDB::bind_method(D_METHOD("get_oct"), &SynthFrequencyOscillator::get_octave);
    ADD_PROPERTY(PropertyInfo(Variant::INT,"Octave",godot::PROPERTY_HINT_RANGE,"1,8,1"),"set_oct","get_oct");

    ClassDB::bind_method(D_METHOD("set_bend_range","Octave"), &SynthFrequencyOscillator::set_bend_range);
    ClassDB::bind_method(D_METHOD("get_bend_range"), &SynthFrequencyOscillator::get_bend_range);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Pitch Bend Range (semitones)"),"set_bend_range","get_bend_range");
}   

void SynthPhaseOscillator::_bind_methods(){    
    ClassDB::bind_method(D_METHOD("set_wf","Waveform"), &SynthPhaseOscillator::set_waveform);
    ClassDB::bind_method(D_METHOD("get_wf"), &SynthPhaseOscillator::get_waveform);
    ADD_PROPERTY(PropertyInfo(Variant::INT,"Waveform Shape",PROPERTY_HINT_ENUM,"Sine,Square,Triangle,Sawtooth,Pulse"),"set_wf","get_wf");
}

void SynthResonantOscillator::initialize(SynthPatchLocals *l){
    synthLocals = l;
    if(excitor.is_valid()){excitor->initialize(l);}
}

void SynthResonantOscillator::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_excitor","excitor oscillator"), &SynthResonantOscillator::set_excitor);
    ClassDB::bind_method(D_METHOD("get_excitor"),&SynthResonantOscillator::get_excitor);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"Excitor Oscillator",PROPERTY_HINT_RESOURCE_TYPE,"SynthOscillator"), "set_excitor", "get_excitor");


    ClassDB::bind_method(D_METHOD("set_decay","Decay Time"),&SynthResonantOscillator::set_decay_time);
    ClassDB::bind_method(D_METHOD("get_decay"),&SynthResonantOscillator::get_decay_time);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Decay Time(seconds)"),"set_decay","get_decay");
    ClassDB::bind_method(D_METHOD("set_excitation_strength", "Excitation Strength"), &SynthResonantOscillator::set_excitation_strength);
    ClassDB::bind_method(D_METHOD("get_excitation_strength"), &SynthResonantOscillator::get_excitation_strength);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Excitation Strength"),"set_excitation_strength","get_excitation_strength");

}

void SynthGroupOscillator::initialize(SynthPatchLocals *l){
    synthLocals = l;
    for(auto v : oscillators){
        Ref<SynthPhaseOscillator> osc = v;
        osc->initialize(l);
    }
}

void SynthGroupOscillator::note_on(){
    active = true;
    frequencyADSR->note_on();
    // print_line("hi");
    for(int i=0;i<oscillators.size();i++){
        Ref<SynthPhaseOscillator> osc = oscillators[i];
        osc->note_on();
    }
}
void SynthGroupOscillator::note_off(){
    frequencyADSR->note_off();
    // print_line("bye");
}

// int counter = 0;
// float biggestOutput = 0.0f;
float SynthGroupOscillator::process(){
    float output = 0.0f;
    float adsrResponse = frequencyADSR->process();
    float adsrSemitones =  Math::lerp(min_semitones,max_semitones,adsrResponse);
    
    bool deactivate = true;
    if(adsrResponse>0.0f){
        deactivate = false;
    }
    //First iterate through the oscillators and get the results
    for(int i=0; i<oscillators.size();i++){
        Ref<SynthPhaseOscillator> osc = oscillators[i];
        if(deactivate){
            osc->note_off();
        }
        else{
            osc->semitone_offset = adsrSemitones;
            output+=osc->process();
        }
    }
    if(deactivate){
        active=false;
        return 0.0f;
    }
    if(oscillators.size()>0){
        output/=std::sqrt(oscillators.size());
    }
    // if(abs(output)>biggestOutput){biggestOutput = abs(output);}
    // if(counter%5000==1 && active){print_line(adsrSemitones);}
    // counter++;
    return output;
}

void SynthGroupOscillator::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_min_adsr_semi","Minimum ADSR Semitones"), &SynthGroupOscillator::set_min_semitones);
    ClassDB::bind_method(D_METHOD("get_min_adsr_semi"), &SynthGroupOscillator::get_min_semitones);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Minimum Oscillator frequency offset in semitones"),"set_min_adsr_semi","get_min_adsr_semi");
    
    ClassDB::bind_method(D_METHOD("set_max_adsr_semi","Maximum ADSR Semitones"), &SynthGroupOscillator::set_max_semitones);
    ClassDB::bind_method(D_METHOD("get_max_adsr_semi"), &SynthGroupOscillator::get_max_semitones);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Maximum Oscillator frequency offset in semitones"),"set_max_adsr_semi","get_max_adsr_semi");
    
    ClassDB::bind_method(D_METHOD("set_oscillators","Oscillator Array"),&SynthGroupOscillator::set_oscillators);
    ClassDB::bind_method(D_METHOD("get_oscillators"),&SynthGroupOscillator::get_oscillators);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY,"Oscillators",PROPERTY_HINT_ARRAY_TYPE,String::num(Variant::OBJECT)+"/"+String::num(PROPERTY_HINT_RESOURCE_TYPE)+":SynthPhaseOscillator"),"set_oscillators","get_oscillators");
   
    ClassDB::bind_method(D_METHOD("set_freq_adsr","ADSR"), &SynthGroupOscillator::set_freq_adsr);
    ClassDB::bind_method(D_METHOD("get_freq_adsr"),&SynthGroupOscillator::get_freq_adsr);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"Frequency ADSR",PROPERTY_HINT_RESOURCE_TYPE,"SynthADSR"), "set_freq_adsr", "get_freq_adsr");

}

void SynthFeedbackOscillator::initialize(SynthPatchLocals *l){
    synthLocals = l;
    if(lowPass.is_valid()){lowPass->initialize(l);}
    if(dcBlock.is_valid()){dcBlock->initialize(l);}
}


float SynthFeedbackOscillator::read_delay(){

    float readPos = (float)writeIndex - delayCurrent;

    while(readPos < 0.0f){
        readPos += (float)buffer.size();
    }

    int i0 = (int)readPos;
    int i1 = (i0 + 1) % buffer.size();

    float frac = readPos - (float)i0;

    return buffer[i0] + (buffer[i1]-buffer[i0])*frac;
}

float SynthFeedbackOscillator::process(){

    delayCurrent += (delayTarget-delayCurrent)*0.001f;//slight smoothing - avoids notches.

    delayTarget = sampleRate/(frequency*processPitch());
    delayTarget = CLAMP(delayTarget,1.0f,(float)buffer.size()-2.0f);

    float delayed = read_delay();
    float noise = (((float)rand()/RAND_MAX)*2.0f-1.0f)*breath; //Can't get Math::randf_range to work for some reason.

    float sample = delayed * feedback;
    sample+=noise;

    sample = std::tanh(sample);

    sample = lowPass->process(sample,1.0f);
    sample = dcBlock->process(sample,1.0f);
    
    buffer[writeIndex] = sample;

    writeIndex++;
    if(writeIndex>=buffer.size()) writeIndex=0;

    return sample;
}

void SynthFeedbackOscillator::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_feedback","feedback"),&SynthFeedbackOscillator::set_feedback);
    ClassDB::bind_method(D_METHOD("get_feedback"),&SynthFeedbackOscillator::get_feedback);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"feedback"),"set_feedback","get_feedback");

    ClassDB::bind_method(D_METHOD("set_breath","breath"),&SynthFeedbackOscillator::set_breath);
    ClassDB::bind_method(D_METHOD("get_breath"),&SynthFeedbackOscillator::get_breath);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"breath"),"set_breath","get_breath");
    
    ClassDB::bind_method(D_METHOD("set_cutoff","cutoff"),&SynthFeedbackOscillator::set_cutoff);
    ClassDB::bind_method(D_METHOD("get_cutoff"),&SynthFeedbackOscillator::get_cutoff);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"cutoff",PROPERTY_HINT_RANGE,"0,1,0.01"),"set_cutoff","get_cutoff");
}

void SynthFilter::initialize(SynthPatchLocals *l){
    synthLocals = l;
}

void SynthFilter::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_gain","gain"),&SynthFilter::set_gain);
    ClassDB::bind_method(D_METHOD("get_gain"),&SynthFilter::get_gain);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Gain"),"set_gain","get_gain");
}

float SynthFrequencyFilter::processCutoff(float envelopeRatio){
    float lfoPitch = 0.0f;
    if(lfo.is_valid()){lfoPitch = lfo->process();} //if no LFO then don't LFO.
    lfoPitch *= lfo_depth;
    float pitchbendTotal = synthLocals->pitchBend*pitchBendRange;

    float semitones = lfoPitch+pitchbendTotal+(envelopeAmount*envelopeRatio);
    if(semitones==0.0f){return 1.0f;}
    return SynthHelper::semitones_to_ratio(semitones);
}

void SynthFrequencyFilter::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_cutoff","Filter Cutoff"),&SynthFrequencyFilter::set_cutoff);
    ClassDB::bind_method(D_METHOD("get_cutoff"),&SynthFrequencyFilter::get_cutoff);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Filter Cutoff Frequency (hz)"),"set_cutoff","get_cutoff");

    ClassDB::bind_method(D_METHOD("set_envelope_amount","Envelope Amount"),&SynthFrequencyFilter::set_envelope_amount);
    ClassDB::bind_method(D_METHOD("get_envelope_amount"),&SynthFrequencyFilter::get_envelope_amount);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Filter Envelope Amount (semitones)"),"set_envelope_amount","get_envelope_amount");

    ClassDB::bind_method(D_METHOD("set_lfo","LFO"),&SynthFrequencyFilter::set_lfo);
    ClassDB::bind_method(D_METHOD("get_lfo"),&SynthFrequencyFilter::get_lfo);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"LFO(Low Frequency Oscillator)",PROPERTY_HINT_RESOURCE_TYPE,"SynthLFO"),"set_lfo","get_lfo");

    ClassDB::bind_method(D_METHOD("set_lfo_depth","LFO Depth"),&SynthFrequencyFilter::set_lfo_depth);
    ClassDB::bind_method(D_METHOD("get_lfo_depth"),&SynthFrequencyFilter::get_lfo_depth);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"LFO Frequency Span (semitones)"),"set_lfo_depth","get_lfo_depth");

    ClassDB::bind_method(D_METHOD("set_bend_range","Pitch Bend Range"),&SynthFrequencyFilter::set_bend_range);
    ClassDB::bind_method(D_METHOD("get_bend_range"),&SynthFrequencyFilter::get_bend_range);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Pitch Bend Range (semitones)"),"set_bend_range","get_bend_range");
}


void SynthBasicLowPassFilter::_bind_methods(){
    //No methods for me!
}

void SynthDCBlockFilter::_bind_methods(){
    //Also no methods for me neither!
}

void SynthSVF::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_q","Q (resonant factor)"),&SynthSVF::set_q);
    ClassDB::bind_method(D_METHOD("get_q"),&SynthSVF::get_q);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Q Ratio (Resonance Factor)"),"set_q","get_q");
    ClassDB::bind_method(D_METHOD("set_pass_mix","Pass filter mix ratio (Low Pass, Band Pass, High Pass)"),&SynthSVF::set_pass_mix);
    ClassDB::bind_method(D_METHOD("get_pass_mix"),&SynthSVF::get_pass_mix);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3,"Pass filter mix ratio (Low Pass, Band Pass, High Pass)"),"set_pass_mix","get_pass_mix");
}

void SynthParallelFilter::initialize(SynthPatchLocals *l){
    synthLocals = l;
    for(auto v : filters){
        Ref<SynthFilter> filter = v;
        filter->initialize(l);
    }
}

float SynthParallelFilter::process(float input, float envelopeRatio){
    if(filters.size()==0){return input;}//Bail if no filters.
    //Iterate through filters and apply
    float output = 0.0f;
    for(int i=0;i<filters.size();i++){
        Ref<SynthFilter> filter = filters[i];
        output += filter->process(input,envelopeRatio)*filter->gain;
    }
    return output;
}

void SynthParallelFilter::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_filters","Filter Array"),&SynthParallelFilter::set_filters);
    ClassDB::bind_method(D_METHOD("get_filters"),&SynthParallelFilter::get_filters);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY,"Filters",PROPERTY_HINT_ARRAY_TYPE,String::num(Variant::OBJECT)+"/"+String::num(PROPERTY_HINT_RESOURCE_TYPE)+":SynthFilter"),"set_filters","get_filters");
}

// void SynthHarmonicParallelFilter::set_cutoff(const float newFreq){
//     cutoff = newFreq;
//     if(filters.size()==0){return;}//Bail if no filters.
//     //Set the cutoff of the child filters to the multiples of the new frequency
//     for(int i=0;i<filters.size();i++){
//         Ref<SynthFrequencyFilter> filter = filters[i];
//         if(!filter.is_valid()){continue;}
//         if(i<filterResonanceRatio.size()){ //set frequency as the individuals if we have individuals
//             filter->set_cutoff(cutoff*filterResonanceRatio[i]);
//         }
//         else{ // Otherwise fallback to, well, teh fallback.
//             filter->set_cutoff(cutoff + (cutoff*fallbackFrequencyRatio*i));
//         }
//     }
// }

// void SynthHarmonicParallelFilter::_bind_methods(){
//     ClassDB::bind_method(D_METHOD("set_fallback_ratio","Resonance Ratio"),&SynthHarmonicParallelFilter::set_fallback_frequency_ratio);
//     ClassDB::bind_method(D_METHOD("get_fallback_ratio"),&SynthHarmonicParallelFilter::get_fallback_frequency_ratio);
//     ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Fallback Resonance Ratio"),"set_fallback_ratio","get_fallback_ratio");

//     ClassDB::bind_method(D_METHOD("set_filter_ratios", "Filter Resonance Ratios"), &SynthHarmonicParallelFilter::set_filter_resonance_ratios);
//     ClassDB::bind_method(D_METHOD("get_filter_ratios"), &SynthHarmonicParallelFilter::get_filter_resonance_ratios);
//     ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "Individual Filter Resonance Ratios (MUST MATCH FILTER COUNT)"), "set_filter_ratios", "get_filter_ratios");

// }

void SynthResonantFilter::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_decay","Decay"),&SynthResonantFilter::set_decay);
    ClassDB::bind_method(D_METHOD("get_decay"),&SynthResonantFilter::get_decay);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Resonator Decay (0.9-1.0)"),"set_decay","get_decay");
    ClassDB::bind_method(D_METHOD("set_cutoff","Resonance(hz)"), &::SynthResonantFilter::set_cutoff);
    ClassDB::bind_method(D_METHOD("set_excitation_strength", "Excitation Strength"), &SynthResonantFilter::set_excitation_strength);
    ClassDB::bind_method(D_METHOD("get_excitation_strength"), &SynthResonantFilter::get_excitation_strength);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Excitation Strength"),"set_excitation_strength","get_excitation_strength");

    ClassDB::bind_method(D_METHOD("set_wet", "Wet"), &SynthResonantFilter::set_wet);
    ClassDB::bind_method(D_METHOD("get_wet"), &SynthResonantFilter::get_wet);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Wet",PROPERTY_HINT_RANGE, "0,1,0.01"),"set_wet","get_wet");

    ClassDB::bind_method(D_METHOD("set_dry", "Dry"), &SynthResonantFilter::set_dry);
    ClassDB::bind_method(D_METHOD("get_dry"), &SynthResonantFilter::get_dry);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Dry",PROPERTY_HINT_RANGE, "0,1,0.01"),"set_dry","get_dry");

    ClassDB::bind_method(D_METHOD("set_note","Note"), &SynthResonantFilter::set_note);
    ClassDB::bind_method(D_METHOD("get_note"), &SynthResonantFilter::get_note);
    ADD_PROPERTY(PropertyInfo(Variant::INT,"Resonator Note",PROPERTY_HINT_ENUM,"C,C#,D,D#,E,F,F#,G,G#,A,A#,B"),"set_note","get_note");

    ClassDB::bind_method(D_METHOD("set_oct","Octave"), &SynthResonantFilter::set_octave);
    ClassDB::bind_method(D_METHOD("get_oct"), &SynthResonantFilter::get_octave);
    ADD_PROPERTY(PropertyInfo(Variant::INT,"Octave",godot::PROPERTY_HINT_RANGE,"1,8,1"),"set_oct","get_oct");

}

//ADSR setters/getters:
void SynthADSR::set_attack(const float nVal){
    attack = (int)(nVal*sampleRate);
}
void SynthADSR::set_decay(const float nVal){
    decay = (int)(nVal*sampleRate);
}
void SynthADSR::set_sustain(const float nVal){
    sustain = nVal;
}
void SynthADSR::set_release(const float nVal){
    release = (int)(nVal*sampleRate);
}

float SynthADSR::get_attack() const{
    return (float)((float)attack/sampleRate);
}
float SynthADSR::get_decay() const{
    return (float)((float)decay/sampleRate);
}
float SynthADSR::get_sustain() const{
    return sustain;
}
float SynthADSR::get_release() const{
    return (float)((float)release/sampleRate);
}


void SynthADSR::note_on(){
    sampleTime = 0;
    currentState = Attack;
    attackValue = value;
}

void SynthADSR::note_off(){
    if (currentState==Release || currentState==Idle){
        return;
    }
    releaseValue = value;
    currentState = Release;
    sampleTime = 0;
}


float SynthADSR::process(){
    sampleTime += 1;
    // if ((sampleTime % 1000) == 0){
    //     print_line(value);
    // }
    switch(currentState){
        case Idle:
            return 0.0f;
        
        case Attack:
            value = Math::lerp(attackValue,1.0f,(float)sampleTime/(float)attack);
            if(sampleTime >= attack){
                currentState = decay>0 ? Decay :  sustain>0 ? Sustain : Release;
                sampleTime = 0;
                releaseValue = value;
            }
            return value;
            
        case Decay:
            value = Math::lerp(releaseValue,sustain,MIN((float)sampleTime/(float)decay, 1.0f));
            if(sampleTime>=decay){
                currentState = Sustain;
                sampleTime = 0;
            }
            return value;

        case Sustain:
            value = sustain;
            return value;

        case Release:
            value = Math::lerp(releaseValue,0.0f,MIN((float)sampleTime/(float)release,1.0f));
            if(sampleTime>=release){
                currentState = Idle;
                value = 0.0f;
            }
            return value;
        return value;
    }
}

void SynthADSR::_bind_methods(){
    //SETTERS
    ClassDB::bind_method(D_METHOD("set_attack","attack"), &SynthADSR::set_attack);
    ClassDB::bind_method(D_METHOD("set_decay","decay"), &SynthADSR::set_decay);
    ClassDB::bind_method(D_METHOD("set_sustain","sustain"), &SynthADSR::set_sustain);
    ClassDB::bind_method(D_METHOD("set_release","release"), &SynthADSR::set_release);
    //GETTERS
    ClassDB::bind_method(D_METHOD("get_attack"),&SynthADSR::get_attack);
    ClassDB::bind_method(D_METHOD("get_decay"),&SynthADSR::get_decay);
    ClassDB::bind_method(D_METHOD("get_sustain"),&SynthADSR::get_sustain);
    ClassDB::bind_method(D_METHOD("get_release"),&SynthADSR::get_release);
    //Properties
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Attack"),"set_attack","get_attack");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Decay"),"set_decay","get_decay");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Sustain"),"set_sustain","get_sustain");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"Release"),"set_release","get_release");
}


///////////////////// SIMPLE SYNTH PATCH SETTERS/GETTERS //////////////////

void SimpleSynthPatch::set_filter_adsr(const Ref<SynthParameterSource> newADSR){
    filterFrequencyModifier = newADSR;
}

Ref<SynthParameterSource> SimpleSynthPatch::get_filter_adsr() const{
    return filterFrequencyModifier;
}
void SimpleSynthPatch::set_pre_filter_adsr(const Ref<SynthParameterSource> newADSR){
    preFilterAmplitudeModifier = newADSR;
}

Ref<SynthParameterSource> SimpleSynthPatch::get_pre_filter_adsr() const{
    return preFilterAmplitudeModifier;
}
void SimpleSynthPatch::set_post_filter_adsr(const Ref<SynthParameterSource> newADSR){
    postFilterAmplitudeModifier = newADSR;
}

Ref<SynthParameterSource> SimpleSynthPatch::get_post_filter_adsr() const{
    return  postFilterAmplitudeModifier;
}

void SimpleSynthPatch::set_oscillators(const TypedArray<SynthOscillator> newOsc){
    oscillators = newOsc;
}

TypedArray<SynthOscillator> SimpleSynthPatch::get_oscillators() const{
    return oscillators;
}

void SimpleSynthPatch::set_filters(const TypedArray<SynthFilter> newFilters){
    filters = newFilters;
}

TypedArray<SynthFilter> SimpleSynthPatch::get_filters() const{
    return filters;
}

//////////////SIMPLE SYNTH PATCH INIT///////////////
void SimpleSynthPatch::initialize(){
    
    for(auto v : oscillators){
        Ref<SynthOscillator> osc = v;
        osc->initialize(&synthLocals);
    }
    
    for(auto v : filters){
        Ref<SynthFilter> filt = v;
        filt->initialize(&synthLocals);
    }
}


///////////////////// SIMPLE SYNTH PATCH ACTUAL PROCESSING ///////////////////
float SimpleSynthPatch::process(){
    if(oscillators.size()==0){
        return 0.0f;
    }
    ///OSCILLATOR BLOCK
    float output = 0.0f;
    for(int i = 0; i<oscillators.size();i++){
        Ref<SynthOscillator> osc = oscillators[i];
        if(!osc.is_null()){output += osc->process()*osc->gain;}
    }
    if(preFilterAmplitudeModifier.is_valid()){output *= preFilterAmplitudeModifier->process();}
    ///FILTER BLOCK
    float freqEnvelope = 1.0f;
    if(filterFrequencyModifier.is_valid()){freqEnvelope = filterFrequencyModifier->process();} //Get our frequency ratio first.
    for(int i=0; i<filters.size();i++){
        Ref<SynthFilter> filter = filters[i];
        // filter->cutoff = frequencyOffset; //Apply frequency offset for randomness in sequencer (or in general) //TODO: Replace this with something else for the sequencer randomness.
        output = filter->process(output,freqEnvelope)*filter->gain; //REMEMBER: FILTERS HAVE THEIR OWN FREQUENCY GATES. WE ONLY PASS A RATIO!
    }
    if(postFilterAmplitudeModifier.is_valid()){output *= postFilterAmplitudeModifier->process();}

    // output *= (1.0f-amplitudeOffset); TODO - FIX THIS WITH SOMETHING BETTER FOR SEQUENCER/OTHER RANDOMIZATION
    // print_line(output);
    return output;
}

void SimpleSynthPatch::note_on(){
    if(filterFrequencyModifier.is_valid()){
        filterFrequencyModifier->note_on();
    }
    if(preFilterAmplitudeModifier.is_valid()){
        preFilterAmplitudeModifier->note_on();
    }
    if(postFilterAmplitudeModifier.is_valid()){
        postFilterAmplitudeModifier->note_on();
    }
    if(oscillators.size()>0){
        for(int i=0;i<oscillators.size();i++){
            Ref<SynthOscillator> osc = oscillators[i];
            if(osc.is_valid()){osc->note_on();}

        }
    }
}

void SimpleSynthPatch::note_off(){
    if(filterFrequencyModifier.is_valid()){
        filterFrequencyModifier->note_off();
    }
    if(preFilterAmplitudeModifier.is_valid()){
        preFilterAmplitudeModifier->note_off();
    }
    if(postFilterAmplitudeModifier.is_valid()){
        postFilterAmplitudeModifier->note_off();
    }
    if(oscillators.size()>0){
        for(int i=0;i<oscillators.size();i++){
            Ref<SynthOscillator> osc = oscillators[i];
            if(osc.is_valid()){osc->note_off();}
        }
    }
}


void SimpleSynthPatch::_bind_methods(){
    //SetterGetters
    ClassDB::bind_method(D_METHOD("set_filter_adsr","ADSR"), &SimpleSynthPatch::set_filter_adsr);
    ClassDB::bind_method(D_METHOD("get_filter_adsr"),&SimpleSynthPatch::get_filter_adsr);

    ClassDB::bind_method(D_METHOD("set_pre_filter_adsr","ADSR"), &SimpleSynthPatch::set_pre_filter_adsr);
    ClassDB::bind_method(D_METHOD("get_pre_filter_adsr"),&SimpleSynthPatch::get_pre_filter_adsr);

    ClassDB::bind_method(D_METHOD("set_post_filter_adsr","ADSR"), &SimpleSynthPatch::set_post_filter_adsr);
    ClassDB::bind_method(D_METHOD("get_post_filter_adsr"),&SimpleSynthPatch::get_post_filter_adsr);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"Filter ADSR",PROPERTY_HINT_RESOURCE_TYPE,"SynthADSR"), "set_filter_adsr", "get_filter_adsr");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"Pre-filter ADSR",PROPERTY_HINT_RESOURCE_TYPE,"SynthADSR"), "set_pre_filter_adsr", "get_pre_filter_adsr");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"Post-filter ADSR",PROPERTY_HINT_RESOURCE_TYPE,"SynthADSR"), "set_post_filter_adsr", "get_post_filter_adsr");

    ClassDB::bind_method(D_METHOD("set_oscillators","Oscillator Array"),&SimpleSynthPatch::set_oscillators);
    ClassDB::bind_method(D_METHOD("get_oscillators"),&SimpleSynthPatch::get_oscillators);
    
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY,"Oscillators",PROPERTY_HINT_ARRAY_TYPE,String::num(Variant::OBJECT)+"/"+String::num(PROPERTY_HINT_RESOURCE_TYPE)+":SynthOscillator"),"set_oscillators","get_oscillators");
    
    ClassDB::bind_method(D_METHOD("set_filters","Filter Array"),&SimpleSynthPatch::set_filters);
    ClassDB::bind_method(D_METHOD("get_filters"),&SimpleSynthPatch::get_filters);
    
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY,"Filters",PROPERTY_HINT_ARRAY_TYPE,String::num(Variant::OBJECT)+"/"+String::num(PROPERTY_HINT_RESOURCE_TYPE)+":SynthFilter"),"set_filters","get_filters");

    //Actual functions
    ClassDB::bind_method(D_METHOD("note_on"),&SimpleSynthPatch::note_on);
    ClassDB::bind_method(D_METHOD("note_off"),&SimpleSynthPatch::note_off);

    ClassDB::bind_method(D_METHOD("set_pitch_bend","pitch bend (semitones)"),&SimpleSynthPatch::set_pitch_bend);
    ClassDB::bind_method(D_METHOD("get_pitch_bend"),&SimpleSynthPatch::get_pitch_bend);
    
}
