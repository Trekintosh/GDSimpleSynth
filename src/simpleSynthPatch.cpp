#include "simpleSynthPatch.hpp"
#include "godot_cpp/core/math.hpp"
#include <cstdlib>

using namespace godot;

NoiseState generateNoise(NoiseState &state){
    state.white = ((float)rand()/RAND_MAX)*2.0-1.0;
    
    state.brown *= 0.9;
	state.brown += state.white * 0.01;
    
    state.pink_b0 = 0.99765 * state.pink_b0 + state.white * 0.0990460;
    state.pink_b1 = 0.96300 * state.pink_b1 + state.white * 0.2965164;
    state.pink_b2 = 0.57000 * state.pink_b2 + state.white * 1.0526913;
    state.pink = state.pink_b0 + state.pink_b1 + state.pink_b2 + state.white * 0.1848;
    
    return state;
};

void ADSR::note_on(){
    sampleTime = 0;
    currentState = Attack;
    attackValue = value;
}

void ADSR::note_off(){
    if (currentState==Release || currentState==Idle){
        return;
    }
    releaseValue = value;
    currentState = Release;
    sampleTime = 0;
}

void ADSR::process(int deltaSamples){
    switch(currentState){
        case Idle:
            return;
        
        case Attack:
            sampleTime += deltaSamples;
            value = Math::lerp(attackValue,1.0f,(float)sampleTime/(float)attack);//TODO: CONVERT ADSR TO INTS FOR SAMPLES!!!
    }
}
