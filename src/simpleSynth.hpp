#pragma once

#include "godot_cpp/classes/audio_frame.hpp"
#include "godot_cpp/classes/audio_server.hpp"
#include "godot_cpp/classes/audio_stream.hpp"
#include "godot_cpp/classes/audio_stream_playback.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/string.hpp"
#include "simpleSynthPatch.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include <cstdint>



class SimpleSynthSequencerSettings: public godot::Resource{
    GDCLASS(SimpleSynthSequencerSettings,godot::Resource)
protected:
    static void _bind_methods();
public:
    int BPM = 60;//How many times per minute we repeat
    int notes_per_beat = 4;
    float note_on_duration = 0.5; //How long to hold the note (1.0 = never let go) IF SCALE NOTE ON IS TRUE. Otherwise, how long to hold the note IN SECONDS.
    bool scale_note_on = true;
    float random_pitch = 0.0f;//How much to let the pitch vary per step
    float random_amplitude = 0.0f;//How much do we want random volume per step
    float random_delay = 0.0f;//How much do we want to delay the note per step randomly
    godot::PackedVector3Array fixed_step_variation = godot::PackedVector3Array({godot::Vector3(0,0,0)}); //A set of manually configurable offsets for the steps. x = pitch y = volume z = delay.

    // Setters
    void set_bpm(int new_bpm);
    void set_notes_per_beat(int new_notes_per_beat);
    void set_note_on_duration(float new_duration);
    void set_scale_note_on(bool new_scale);
    void set_random_pitch(float new_random_pitch);
    void set_random_amplitude(float new_random_amplitude);
    void set_random_delay(float new_random_delay);
    void set_fixed_step_variation(const godot::PackedVector3Array &new_steps);

    // Getters
    int get_bpm() const;
    int get_notes_per_beat() const;
    float get_note_on_duration() const;
    bool get_scale_note_on() const;
    float get_random_pitch() const;
    float get_random_amplitude() const;
    float get_random_delay() const;
    godot::PackedVector3Array get_fixed_step_variation() const;
};



class SimpleSynthStream: public godot::AudioStream{
    GDCLASS(SimpleSynthStream, godot::AudioStream)
protected:
    static void _bind_methods();
public:
    bool sequencer_enabled = true;
    godot::Ref<SimpleSynthSequencerSettings> sequencerSettings;
    godot::Ref<SimpleSynthPatch> myPatch;

    //Setgets
    void set_sequencer_enabled(const bool set);
    bool get_sequencer_enabled() const;

    void set_sequencer_settings(const godot::Ref<SimpleSynthSequencerSettings>);
    godot::Ref<SimpleSynthSequencerSettings> get_sequencer_settings() const;

    void set_patch(const godot::Ref<SimpleSynthPatch> newPatch);
    godot::Ref<SimpleSynthPatch> get_patch() const;

    //BIG PILE OF OVERRIDES WOOO
    godot::Ref<godot::AudioStreamPlayback> _instantiate_playback() const override; // The only important one here.
    
    bool _is_monophonic() const override {return true;};
    double _get_length() const override {return 0.0;};
    godot::String _get_stream_name() const override {return "Simple Synth";};
    double _get_bpm() const override {return sequencerSettings->BPM;};
    int32_t _get_beat_count() const override {return sequencerSettings->notes_per_beat;};
    void set_position(uint64_t pos){return;};
    void reset()const{return;};


};

class SimpleSynthPlayback: public godot::AudioStreamPlayback{
    GDCLASS(SimpleSynthPlayback,godot::AudioStreamPlayback)
protected:
    static void _bind_methods();
public:
    const SimpleSynthStream *myStream = nullptr;
    void set_stream(const SimpleSynthStream *p_stream) {
        godot::print_line(p_stream);
        myStream = p_stream;
    }

    godot::Ref<SimpleSynthSequencerSettings> sequencerSettings;
    godot::Ref<SimpleSynthPatch> patch;
    int _mix(godot::AudioFrame *buffer, float rate_scale, int frames) override;

    bool sequencer_enabled = true;
    int currentSequencerStep = 0;
    bool sequencerNoteIsOn = false;
    float currentSequencerPitch = 0.0f;
    float currentSequencerAmplitude = 1.0f;
    float currentSequencerDelayPhase = 0.0f;

    float sequencerStepsPerSecond = 0.0f;
    void advanceStep(int currentStep);

    //Mandatory overrides
    int _get_loop_count() const override{return total_notes;};
    double _get_playback_position() const override{return 0.0;};
    void _start(double p_from_pos = 0.0) override{return;};
    void _stop() override{return;}


private:
    int sampleRate = godot::AudioServer::get_singleton()->get_mix_rate();

    int step_increment; //How many samples per step, updated when the sequencer steps are updated.

    float phase = 0.0f;
    int currentFixedSequenceStep = 0;

    float triggerPhase = 0.0f;
    float releasePhase = 0.0f;

    float targetAmplitudeOffset = 0.0f;
    float targetFrequencyOffset = 0.0f;

    float currentAmplitudeOffset = 0.0f;
    float currentFrequencyOffset = 0.0f;

    bool noteHasTriggeredThisStep = false;
    bool noteHasReleasedThisStep = false;

    int gate_time = 0;

    int total_notes = 0;

    void processSequencer();

};
