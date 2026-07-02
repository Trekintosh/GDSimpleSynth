#pragma once
#include "godot_cpp/core/math_defs.hpp"
#define _USE_MATH_DEFINES
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include "godot_cpp/classes/audio_server.hpp"


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

    int sampleRate = godot::AudioServer::get_singleton()->get_mix_rate();

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

//Extremely simple sine-wave LFO.
class SynthLFO: public godot::Resource{
    GDCLASS(SynthLFO,godot::Resource)
protected:
    static void _bind_methods();
public:
    float process(){
        phase += rate/sampleRate;
        while(phase>=1.0f){phase-=1.0f;}
        return std::sin(Math_TAU*phase);
    };
    float phase = 0.0f;
    float rate = 1.0f;
    //SetGets;
    void set_rate(const float x){rate = x;}
    float get_rate() const{return rate;}
private:
    float sampleRate = godot::AudioServer::get_singleton()->get_mix_rate();
};


class SynthOscillator: public godot::Resource{
    GDCLASS(SynthOscillator, godot::Resource)
public:
    virtual float process() = 0; //PROCESSES A SINGLE SAMPLE OF OSCILLATION
    bool active = false;
    virtual void note_on();
    virtual void note_off();
    float frequency_offset = 0.0f;
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


class SynthPhaseOscillator: public SynthOscillator{
    GDCLASS(SynthPhaseOscillator,SynthOscillator)
protected:
    static void _bind_methods();
protected:
    float phase = 0.0f;
    float frequency = 440.0f;
    float sampleRate = godot::AudioServer::get_singleton()->get_mix_rate();
    godot::Ref<SynthLFO> lfo;
    float lfo_depth = 0.0f;
    virtual float processLFO(); //For processing the LFO but can be overwritten if an oscillator needs to get WEIRD.
    //setgets
    void set_lfo(const godot::Ref<SynthLFO> newlfo){lfo = newlfo;}
    godot::Ref<SynthLFO> get_lfo() const{return lfo;}

    void set_lfo_depth(const float newdepth){lfo_depth = newdepth;}
    float get_lfo_depth() const{return lfo_depth;}
public:
    void updatePhase() {
        phase += frequency*(1.0f-frequency_offset)/sampleRate;
        while(phase>=1.0f){
            phase-=1.0f;
        }
    };
    enum wf{
        SINE,
        SQUARE,
        TRIANGLE,
        SAWTOOTH
    };
    wf waveform = SINE;

    float process() override{
        updatePhase();
        return phase * 2.0f-1.0f;
    }
};

class SynthTriangleOscillator: public SynthPhaseOscillator{
    GDCLASS(SynthTriangleOscillator,SynthPhaseOscillator)
protected:
    static void _bind_methods();
public:
    float process() override{
        updatePhase();
        return 4.0f * std::fabs(phase - 0.5f) - 1.0f;
    }
};
class SynthSineOscillator: public SynthPhaseOscillator{
    GDCLASS(SynthSineOscillator,SynthPhaseOscillator)
protected:
    static void _bind_methods();
public:
    float process() override{
        updatePhase();
        return std::sin(Math_TAU * phase);
    }
};
class SynthSquareOscillator: public SynthPhaseOscillator{
    GDCLASS(SynthSquareOscillator,SynthPhaseOscillator)
protected:
    static void _bind_methods();
public:
    float process() override{
        updatePhase();
        return phase < 0.5f ? 1.0f : -1.0f;
    }
};


class SynthGroupOscillator: public SynthOscillator{
    GDCLASS(SynthGroupOscillator,SynthOscillator)
protected:
    static void _bind_methods();

public:
    godot::TypedArray<SynthPhaseOscillator> oscillators;
    godot::Ref<SynthADSR> frequencyADSR;
    float minimumFrequencyRatio = 0.01f;

    float process() override;

    void note_on() override;
    void note_off() override;

    //setgets
    void set_oscillators(const godot::TypedArray<SynthPhaseOscillator> newOsc){oscillators = newOsc;};
    godot::TypedArray<SynthPhaseOscillator> get_oscillators(){return oscillators;};

    void set_freq_adsr(const godot::Ref<SynthADSR> newADSR){frequencyADSR = newADSR;};
    godot::Ref<SynthADSR> get_freq_adsr(){return frequencyADSR;};

    void set_minimum_frequency_ratio(const float newFreq){minimumFrequencyRatio = newFreq;}
    float get_minimum_frequency_ratio(){return minimumFrequencyRatio;}
};



class SynthFilter: public godot::Resource{
    GDCLASS(SynthFilter,godot::Resource)
protected:
    static void _bind_methods();
public:
    float minFrequencyHz = 1000.0f;
    float maxFrequencyHz = 5000.0f;
    float frequencyOffset = 0.0f;
    void setMinFreq(const float newFreq){
        minFrequencyHz = newFreq;
    }
    void setMaxFreq(const float newFreq){
        maxFrequencyHz = newFreq;
    }
    float getMinFreq(){
        return minFrequencyHz;
    }
    float getMaxFreq(){
        return maxFrequencyHz;
    }

    virtual float process(float input,float envelopeRatio) = 0;
};

class SynthSVF : public SynthFilter{ // CODE FROM https://gist.github.com/hollance/2891d89c57adc71d9560bcf0e1e55c4b - THANKS!!
    GDCLASS(SynthSVF,SynthFilter)
protected:
    static void _bind_methods();
public:
    float Q = 1.0;
    godot::Vector3 pass_mix = godot::Vector3(0.0f,1.0f,0.0f); //Low Pass, Band Pass, High Pass, respectively. Band Pass full by default.
    void setCoefficients(float freq, float newQ) noexcept{
        g = std::tan(Math_PI * freq / sampleRate);
        k = 1.0 / newQ;
        a1 = 1.0 / (1.0 + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    };

    void set_pass_mix(const godot::Vector3 newMix){
        pass_mix = newMix;
    }
    godot::Vector3 get_pass_mix(){
        return pass_mix;
    };
    void set_q(const float newQ){
        Q = newQ;
    };
    float get_q(){
        return Q;
    };

    float process(float v0, float envelopeRatio) override{
        float cutoff = minFrequencyHz*std::pow(maxFrequencyHz/minFrequencyHz,envelopeRatio); //EXPONENTIAL GAIN -- TODO: CHECK IF THIS SUCKS ASS
        cutoff *= 1.0+frequencyOffset;
        setCoefficients(cutoff, Q);
        float v3 = v0 - ic2eq;
        float v1 = a1 * ic1eq + a2 * v3;
        float v2 = ic2eq + a2 * ic1eq + a3 * v3;
        float high = v0 - k * v1 - v2;
        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;
        // if (fabs(ic1eq) < 1e-20f){ic1eq = 0.0f;}
        // if (fabs(ic2eq) < 1e-20f){ic2eq = 0.0f;} //bring to zero for floating poin weirdness.
        return pass_mix.x * v2 + pass_mix.y * v1 + pass_mix.z * high;
    };

private:
    float g, k, a1, a2, a3;  // filter coefficients
    float ic1eq, ic2eq;      // internal state
    float sampleRate = (float)godot::AudioServer::get_singleton()->get_mix_rate();

};


class SimpleSynthPatch: public godot::Resource{
    GDCLASS(SimpleSynthPatch, godot::Resource)
protected:
    static void _bind_methods();
public:

    godot::TypedArray<SynthOscillator> oscillators;
    godot::TypedArray<SynthFilter> filters;
    godot::Ref<SynthADSR> freqADSR;
    godot::Ref<SynthADSR> ampADSR;

    float frequencyOffset = 0.0f;
    float amplitudeOffset = 0.0f;

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

    void set_filters(const godot::TypedArray<SynthFilter> newFilters);
    godot::TypedArray<SynthFilter> get_filters() const;


    //Processing
    float process();

};

