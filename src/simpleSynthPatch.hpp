#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include <array>


//This is a struct to hold ADSR stuff
struct ADSR {
    float attack = 0.03f;
    float decay = 0.05f;
    float sustain = 0.7f;
    float release = 0.5f;

    float value = 0.0f;

    enum state{
        Attack,
        Decay,
        Sustain,
        Release,
        Idle
    };
    state currentState = Idle;
    int sampleTime = 0;

    float releaseValue = 0.0f;
    float attackValue = 0.0f;

    void note_on();
    void note_off();

    void process(int deltaSamples);

};

struct NoiseState {
    float white = 0.0f;

    float pink = 0.0f;
    
    float brown = 0.0f;

    float pink_b0 = 0.0f;
    float pink_b1 = 0.0f;
    float pink_b2 = 0.0f;
};

NoiseState generateNoise(NoiseState &state);

class SimpleSynthPatch: public godot::RefCounted{
    GDCLASS(SimpleSynthPatch, godot::RefCounted)
protected:
    static void _bind_methods();
public:
    enum waveType{
        WHITE_NOISE,
        PINK_NOISE,
        BROWN_NOISE,
        COMBO_NOISE,
    };
    waveType waveform = WHITE_NOISE;
    std::array<float, 3> comboNoiseMix = {0.0f,0.0f,0.0f}; //WHITE, PINK, and BROWN, respectively.
    ADSR freqADSR;
    ADSR ampADSR;
    void updateFreqADSR(float a=0.0f,float d=0.0f, float s=0.0f, float r=0.0f);
    void updateAmpADSR(float a=0.0f,float d=0.0f, float s=0.0f, float r=0.0f);

private:
    NoiseState noise_state;
};

