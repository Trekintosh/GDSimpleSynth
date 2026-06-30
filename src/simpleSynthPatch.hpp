#pragma once

#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/vector3.hpp"


//This is a class to hold ADSR stuff
class SynthADSR: public godot::Resource {
    GDCLASS(SynthADSR,Resource)
    //ADR ARE IN SAMPLES, S IS A FLOAT BECUASE IT'S A PERCENTAGE
private:
    int attack = 0;
    int decay = 0;
    float sustain = 0.0f;
    int release = 0;

    float value = 0.0f;

    enum state{
        Attack,
        Decay,
        Sustain,
        Release,
        Idle
    };
    state currentState = Idle;

    int sampleRate = 44100;
    int sampleTime = 0;

    float releaseValue = 0.0f;
    float attackValue = 0.0f;

public:
    // ALL IN FRACTIONS OF SECONDS, SAMPLE CONVERSION IN THE SETTERS AND GETTERS
    void set_attack(const float nVal);
    void set_decay(const float nVal);
    void set_sustain(const float nVal);
    void set_release(const float nVal);

    float get_attack() const;
    float get_decay() const;
    float get_sustain() const;
    float get_release() const;

    void note_on();
    void note_off();
    
    float process(int deltaSamples);

protected:
    static void _bind_methods();
};


class SynthOscillator: public godot::Resource{
    GDCLASS(SynthOscillator, godot::Resource)
public:
    virtual float process() = 0; //PROCESSES A SINGLE SAMPLE OF OSCILLATION

protected:
    static void _bind_methods();
};


class SynthNoiseOscillator: public SynthOscillator{
    GDCLASS(SynthNoiseOscillator, SynthOscillator)
private:
    float white = 0.0f;

    float pink = 0.0f;
    
    float brown = 0.0f;

    float pink_b0 = 0.0f;
    float pink_b1 = 0.0f;
    float pink_b2 = 0.0f;

public:
    float process() override;
    godot::Vector3 noise_mix = {1.0f,0.0f,0.0f};
    
    void set_noise_mix(const godot::Vector3 newMix);
    godot::Vector3 get_noise_mix();

protected:
    static void _bind_methods();
};




class SimpleSynthPatch: public godot::Resource{
    GDCLASS(SimpleSynthPatch, godot::Resource)
protected:
    static void _bind_methods();
public:

    godot::TypedArray<SynthOscillator> oscillators;
    godot::Ref<SynthADSR> freqADSR;
    godot::Ref<SynthADSR> ampADSR;

    // Setters and getters
    void updateFreqADSR(float a,float d, float s, float r);
    void updateAmpADSR(float a,float d, float s, float r);
    void set_amp_adsr(const godot::Ref<SynthADSR> newADSR);
    void set_freq_adsr(const godot::Ref<SynthADSR> newADSR);
    godot::Ref<SynthADSR> get_amp_adsr() const;
    godot::Ref<SynthADSR> get_freq_adsr() const;

    void note_on();
    void note_off();

    void set_oscillators(const godot::TypedArray<SynthOscillator> newOsc);
    godot::TypedArray<SynthOscillator> get_oscillators() const;


    //Processing
    float process();

};

