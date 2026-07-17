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
    void trigger_step();
    void advance_step();

    void process();

    void initialize(SynthPatchLocals *l, SimpleSynthPatch *p_patch);
    
    bool auto_step = false;
    
    float steps_per_minute = 60;

    float step_hold_time = 0.5; //If scale_steps is TRUE, how long to hold the note as a percentage of step length. Otherwise step duration in SECONDS!

    float step_random_delay = 0.05;//If scale_steps is TRUE, how long to delay note as percentage of step length. Otherwise in SECONDS

    float random_pitch = 0.1;//SEMITONES!

    bool scale_steps = true;

    //Step count is derived from the below.
    PackedVector3Array step_details; //x = outch offset(semitones), y = delay offset(fraction/seconds, same as random_delay), z = TBD.

    

private:
    int sample_clock = 0;

    int samples_per_step = synthLocals? synthLocals->sampleRate/(steps_per_minute/60) : 999999;

    int samples_until_note_on = 999999;
    int samples_until_note_off = 999999;

    int current_step = 0;

};
