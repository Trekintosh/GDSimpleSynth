#include "simpleSynthPatch.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/defs.hpp"
#include "godot_cpp/core/math.hpp"
#include "godot_cpp/core/property_info.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include <cstdlib>

using namespace godot;

void SynthOscillator::_bind_methods(){
    //EMPTY BECAUSE THIS IS AN ABSTRACT.
}

float SynthNoiseOscillator::process(){
    white = ((float)rand()/RAND_MAX)*2.0f-1.0f;
    
    brown *= 0.9;
	brown += white * 0.01;
    
    pink_b0 = 0.99765 * pink_b0 + white * 0.0990460;
    pink_b1 = 0.96300 * pink_b1 + white * 0.2965164;
    pink_b2 = 0.57000 * pink_b2 + white * 1.0526913;
    pink = pink_b0 + pink_b1 + pink_b2 + white * 0.1848;
    
    return white*noise_mix.x+pink*noise_mix.y+brown*noise_mix.z;
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


float SynthADSR::process(int deltaSamples = 1){
    sampleTime += deltaSamples;
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

void SimpleSynthPatch::set_amp_adsr(const Ref<SynthADSR> newADSR){
    ampADSR = newADSR;
}
void SimpleSynthPatch::set_freq_adsr(const Ref<SynthADSR> newADSR){
    freqADSR = newADSR;
}

Ref<SynthADSR> SimpleSynthPatch::get_amp_adsr() const{
    return ampADSR;
}

Ref<SynthADSR> SimpleSynthPatch::get_freq_adsr() const{
    return freqADSR;
}

void SimpleSynthPatch::set_oscillators(const TypedArray<SynthOscillator> newOsc){
    oscillators = newOsc;
}

TypedArray<SynthOscillator> SimpleSynthPatch::get_oscillators() const{
    return oscillators;
}

///////////////////// SIMPLE SYNTH PATCH ACTUAL PROCESSING ///////////////////
float SimpleSynthPatch::process(){
    if(oscillators.size()==0){
        return 0.0f;
    }
    float output = 0.0f;
    for(int i = 0; i<oscillators.size();i++){
        Ref<SynthOscillator> osc = oscillators[i];
        output += osc->process();
    }
    output *= ampADSR->process();
    // print_line(output);
    return output;
}

void SimpleSynthPatch::note_on(){
    if(!ampADSR.is_null()){
        ampADSR->note_on();
    }
    if(!freqADSR.is_null()){
        freqADSR->note_on();
    }
}

void SimpleSynthPatch::note_off(){
    if(!ampADSR.is_null()){
        ampADSR->note_off();
    }
    if(!freqADSR.is_null()){
        freqADSR->note_off();
    }
}


void SimpleSynthPatch::_bind_methods(){
    //SetterGetters
    ClassDB::bind_method(D_METHOD("set_amp_adsr","ADSR"), &SimpleSynthPatch::set_amp_adsr);
    ClassDB::bind_method(D_METHOD("get_amp_adsr"),&SimpleSynthPatch::get_amp_adsr);

    ClassDB::bind_method(D_METHOD("set_freq_adsr","ADSR"), &SimpleSynthPatch::set_freq_adsr);
    ClassDB::bind_method(D_METHOD("get_freq_adsr"),&SimpleSynthPatch::get_freq_adsr);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"Amplitude ADSR",PROPERTY_HINT_RESOURCE_TYPE,"SynthADSR"), "set_amp_adsr", "get_amp_adsr");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"Frequency ADSR",PROPERTY_HINT_RESOURCE_TYPE,"SynthADSR"), "set_freq_adsr", "get_freq_adsr");

    ClassDB::bind_method(D_METHOD("set_oscillators","Oscillator Array"),&SimpleSynthPatch::set_oscillators);
    ClassDB::bind_method(D_METHOD("get_oscillators"),&SimpleSynthPatch::get_oscillators);
    
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY,"Oscillators",PROPERTY_HINT_ARRAY_TYPE,String::num(Variant::OBJECT)+"/"+String::num(PROPERTY_HINT_RESOURCE_TYPE)+":SynthOscillator"),"set_oscillators","get_oscillators");

    //Actual functions
    ClassDB::bind_method(D_METHOD("note_on"),&SimpleSynthPatch::note_on);
    ClassDB::bind_method(D_METHOD("note_off"),&SimpleSynthPatch::note_off);
}
