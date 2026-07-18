#include "simpleSynthSequencer.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "simpleSynthPatch.hpp"

void SimpleSynthSequencer::schedule_step(int ideal, int on, int off){
    SequencerEvent event = SequencerEvent(ideal,on,off);
    eventQueue.push_back(event); 
}

//Clear out stuff
void SimpleSynthSequencer::reset(){
    current_step = 0;
}

void SimpleSynthSequencer::initialize(SynthPatchLocals *l, SimpleSynthPatch *p_patch){
    synthLocals = l;
    patch = p_patch;

    eventQueue.reserve(1024);
    reset();
}

void SimpleSynthSequencer::process(){
    if(!patch){return;}//No patch no go.

    for(int i = eventQueue.size()-1;i>=0;--i){
        SequencerEvent &event = eventQueue[i];

        if(--event.samples_to_off<=0){
            //off and out
            patch->note_off();
            eventQueue[i] = eventQueue.back();
            eventQueue.pop_back();
        }

        if(event.samples_to_on>=0&&--event.samples_to_on<=0){trigger_step();}

        if(event.samples_to_step>=0&&--event.samples_to_step<=0){advance_step();}
    }
}

int SimpleSynthSequencer::find_next_step(){
    int smallestStep = 999999999;
    int result = -1;
    for(int i=0; i<eventQueue.size(); i++){
        SequencerEvent &event = eventQueue[i];
        int smallerStep = event.samples_to_step<event.samples_to_on?event.samples_to_step:event.samples_to_on;

        if(smallerStep<result){
            smallestStep=smallerStep;
            result = i;
        }
    }
    return result;
}

void SimpleSynthSequencer::advance_step(){
    int nextStepSamples = samples_per_step;
    int nextStepOn = samples_per_step;
    int nextStepOff = samples_per_step+(!scale_steps? synthLocals->sampleRate/(steps_per_minute/60) : samples_per_step*step_hold_time);
    
    int randomOnOffset = step_random_delay>0?UtilityFunctions::randi_range(-random_delay_samples,random_delay_samples):0;
    int randomOffOffset = randomOnOffset+step_random_hold_time>0?UtilityFunctions::randi_range(-random_hold_samples,random_hold_samples):0;

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

    schedule_step(nextStepSamples,nextStepOn,nextStepOff);
 
}



    // int closestStep = find_next_step();
    // if(closestStep>=0){//Case for if there are other steps in queue
    //     SequencerEvent &nextEvent = eventQueue[closestStep];
    //     int delta = 
    // }
