#include "simpleSynthSequencer.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "simpleSynthPatch.hpp"


//Clear out stuff
void SimpleSynthSequencer::reset(){
    current_step = 0;
}

void SimpleSynthSequencer::initialize(SynthPatchLocals *l, SimpleSynthPatch *p_patch){
    synthLocals = l;
    patch = p_patch;

    samples_per_step = synthLocals? synthLocals->sampleRate/(steps_per_minute/60) : 44100; //Fallback to 44.1khz

    reset();
}

void SimpleSynthSequencer::process(){
    if(!patch||!synthLocals){return;}//No patch no go.

}


void SimpleSynthSequencer::advance_step(){
    int nextStepSamples = samples_per_step;
    int nextStepOn = samples_per_step;
    int nextStepOff = samples_per_step+(!scale_steps? synthLocals->sampleRate/(steps_per_minute/60) : samples_per_step*step_hold_time);
    
    int randomOnOffset = random_delay_samples>0?UtilityFunctions::randi_range(-random_delay_samples,random_delay_samples):0;
    int randomOffOffset = random_hold_samples>0?UtilityFunctions::randi_range(-random_hold_samples,random_hold_samples):0;

    Vector3 newStepDetails = Vector3(0,0,0);

    if(step_details.size()>0){//For if we do or don't have fancy steps
        current_step++;
        if(current_step>=step_details.size()){
            current_step = 0;
        }
        newStepDetails = step_details[current_step];
    }

    //Scale step logic is completely different from non scale steps
    if(scale_steps){
            nextStepOn += randomOnOffset+(newStepDetails.y*samples_per_step);
            nextStepOff += randomOffOffset+(newStepDetails.y*samples_per_step);
    }
    else{
        nextStepOn += randomOnOffset+(newStepDetails.y*synthLocals->sampleRate);
        nextStepOff += randomOffOffset+(newStepDetails.y*synthLocals->sampleRate);
    }

}

