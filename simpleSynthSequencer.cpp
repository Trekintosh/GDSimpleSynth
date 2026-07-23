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

    if(samples_until_next_step>=0){
        samples_until_next_step--;
        if(samples_until_next_step<=0){
            samples_until_next_step = -1;
            advance_step();
        }
    }

    if(samples_until_note_on>=0){
        samples_until_note_on--;
        if(samples_until_note_on<=0){
            patch->note_on();
        }
    }

    if(samples_until_note_off>=0){
        samples_until_note_off--;
        if(samples_until_note_off<=0){
            patch->note_off();
        }
    }
}

void SimpleSynthSequencer::calculate_step_samples(){
    //FILL THIS IDIOT!
}


void SimpleSynthSequencer::advance_step(){
    int thisStepOn = 0;
    int thisStepOff = step_hold_samples;
    
    int randomOnOffset = random_delay_samples>0?UtilityFunctions::randi_range(0,random_delay_samples):0;
    int randomOffOffset = random_hold_samples>0?UtilityFunctions::randi_range(-random_hold_samples/2,random_hold_samples/2):0;

    //Reset incoming step thingies
    samples_until_next_step = -1;
    samples_until_note_off = -1;
    samples_until_note_on = -1;

    Vector3 newStepDetails = Vector3(0,0,0);

    if(step_details.size()>0){//For if we do or don't have fancy steps
        newStepDetails = step_details[current_step];
    }

    //Scale step logic is completely different from non scale steps
    if(scale_steps){
            thisStepOn += randomOnOffset+(newStepDetails.y*samples_per_step);
            thisStepOff += randomOffOffset+(newStepDetails.y*samples_per_step);
    }
    else{
        thisStepOn += randomOnOffset+(newStepDetails.y*synthLocals->sampleRate);
        thisStepOff += randomOffOffset+(newStepDetails.y*synthLocals->sampleRate);
    }

    //Apply the new thingies
    samples_until_next_step = auto_step?samples_per_step:-1; //Don't set next step timer unless auto step is on.
    samples_until_note_on = thisStepOn;
    samples_until_note_off = auto_step?thisStepOff:-1;

    //Update current step
     if(step_details.size()>0){//For if we do or don't have fancy steps
        current_step++;
        if(current_step>=step_details.size()){
            current_step = 0;
        }
    }

}

