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

    float step_random_hold_time = 0.0; //If scale_steps is TRUE, how long to randomly adjust note hold as portion of step length. Otherwise in SECONDS!

    float step_random_delay = 0.05;//If scale_steps is TRUE, how long to delay note as percentage of step length. Otherwise in SECONDS

    float random_pitch = 0.1;//SEMITONES!

    bool scale_steps = true;

    //Step count is derived from the below.
    PackedVector3Array step_details; //x = outch offset(semitones), y = delay offset(fraction/seconds, same as random_delay), z = amplitude offset.

    int calculate_step_samples();

    int find_next_step();

private:
    int samples_per_step = synthLocals? synthLocals->sampleRate/(steps_per_minute/60) : 999999;

    int random_delay_samples = 0;
    
    int random_hold_samples = 0;

    int current_step = 0;

    int step_count = 1;

    void schedule_step(int ideal, int on, int off);

    struct SequencerEvent{
        int samples_to_step = -1;
        int samples_to_on = -1;
        int samples_to_off = -1;

        SequencerEvent(int p_sts, int p_ston, int p_stoff):samples_to_step(p_sts),samples_to_on(p_ston),samples_to_off(p_stoff){}
    };

    std::vector<SequencerEvent> eventQueue;
};
