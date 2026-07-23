#pragma once

#include "godot_cpp/variant/packed_vector3_array.hpp"
#include<godot_cpp/classes/resource.hpp>
#include<simpleSynthPatch.hpp>

using namespace godot;

class SimpleSynthSequencer: public godot::Resource{
    GDCLASS(SimpleSynthSequencer,Resource)
protected:
    static void _bind_methods();
public:
    SynthPatchLocals *synthLocals = nullptr;
    SimpleSynthPatch *patch = nullptr;


    void reset();
    // void trigger_step(); TODO: Not sure if I even need this? Calling advance already resets the timers
    void advance_step();

    void process();

    void initialize(SynthPatchLocals *l, SimpleSynthPatch *p_patch);
    
    bool auto_step = false;
    
    float steps_per_minute = 60;

    float step_hold_time = 0.5; //If scale_steps is TRUE, how long to hold the note as a percentage of step length. Otherwise step duration in SECONDS!

    float step_random_hold_time = 0.0; //If scale_steps is TRUE, how long to randomly adjust note hold as portion of step length. Otherwise in SECONDS!

    float step_random_delay = 0.05;//If scale_steps is TRUE, how long to delay note as percentage of step length. Otherwise in SECONDS

    float random_pitch = 0.1;//SEMITONES!

    bool scale_steps = true;

    //Step count is derived from the below.
    PackedVector3Array step_details; //x = pitch offset(semitones), y = delay offset(fraction/seconds, same as random_delay), z = amplitude offset.

    void calculate_step_samples();

    void set_steps_per_minute(const float x){steps_per_minute = x;calculate_step_samples();}
    float get_steps_per_minute() const {return steps_per_minute;}

    void set_step_hold_time(const float x){step_hold_time = x;calculate_step_samples();}
    float get_step_hold_time() const {return step_hold_time;}

    void set_random_hold_time(const float x){step_random_hold_time = x;calculate_step_samples();}
    float get_random_hold_time() const {return step_random_hold_time;}

    void set_random_delay(const float x){step_random_delay = x;calculate_step_samples();}
    float get_random_delay() const {return step_random_delay;}

private:
    int samples_per_step = synthLocals? synthLocals->sampleRate/(steps_per_minute/60) : 44100; // Default to 44.1khz

    int step_hold_samples = 0;
    
    int random_delay_samples = 0;
    
    int random_hold_samples = 0;

    int samples_until_note_on = -1;

    int samples_until_note_off = -1;

    int samples_until_next_step = -1;

    int current_step = 0;

    int step_count = 1;
};
