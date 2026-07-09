#pragma once
#include "SynthHelper.hpp"
#include "godot_cpp/core/math_defs.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"
#define _USE_MATH_DEFINES
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include "godot_cpp/classes/audio_server.hpp"


//struct that holds any variables that need to be referenced by anything else in the patch regardless of depth and such.
struct SynthPatchLocals{
    float sampleRate = godot::AudioServer::get_singleton()->get_mix_rate();
    float pitchBend = 0;
};


//This is a class to hold ADSR stuff
class SynthADSR: public godot::Resource {
    GDCLASS(SynthADSR,Resource)
    //ADR ARE IN SAMPLES, S IS A FLOAT BECUASE IT'S A PERCENTAGE
private:
    SynthPatchLocals *synthLocals = nullptr;

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

    virtual void initialize(SynthPatchLocals *locals);    
protected:
    static void _bind_methods();
};

//Extremely simple sine-wave LFO.
class SynthLFO: public godot::Resource{
    GDCLASS(SynthLFO,godot::Resource)
protected:
    static void _bind_methods();
public:    
    SynthPatchLocals *synthLocals = nullptr;
    virtual void initialize(SynthPatchLocals *locals);

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
    SynthPatchLocals *synthLocals = nullptr;
    virtual void initialize(SynthPatchLocals *locals);

    virtual float process(){return 0.0f;}; //PROCESSES A SINGLE SAMPLE OF OSCILLATION
    bool active = false;
    virtual void note_on();
    virtual void note_off();
    float semitone_offset = 0.0f;
    float gain = 1.0f;
    void set_gain(const float x){gain = x;}
    float get_gain() const {return gain;}
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

class SynthFrequencyOscillator: public SynthOscillator{
    GDCLASS(SynthFrequencyOscillator, SynthOscillator)
protected:
    static void _bind_methods();
    float phase = 0.0f;
    godot::Ref<SynthLFO> lfo;
    float lfo_depth = 0.0f;
    float pitchbendRange = 0.0f;
    virtual float processPitch(); //For processing the LFO but can be overwritten if an oscillator needs to get WEIRD
    
    //setgets
    void set_lfo(const godot::Ref<SynthLFO> newlfo){lfo = newlfo;}
    godot::Ref<SynthLFO> get_lfo() const{return lfo;}

    void set_lfo_depth(const float newdepth){lfo_depth = newdepth;}
    float get_lfo_depth() const{return lfo_depth;}

public:
    float frequency = 440.0f; 
    float sampleRate = godot::AudioServer::get_singleton()->get_mix_rate();

    //For handling notes.
    void updateFrequency(){
        frequency = SynthHelper::note_to_frequency(note, octave);
    }
    void set_note(const int new_note){note = static_cast<SynthHelper::Note>(new_note);updateFrequency();}
    int get_note()const{return note;}
    void set_octave(const int new_octave){octave = new_octave;updateFrequency();}
    int get_octave()const{return octave;}

    void set_bend_range(const float x){pitchbendRange = x;}
    float get_bend_range() const {return pitchbendRange;}
    SynthHelper::Note note = SynthHelper::A; //TODO - Set up _validate_property to hide this if the oscillator IS set to pulse.
    int octave = 4; //TODO - Set up _validate_property to hide this if the oscillator IS set to pulse.

    void set_frequency(const float newFreq){frequency = newFreq;};
    float get_frequency() const{return frequency;};
};

class SynthResonantOscillator: public SynthFrequencyOscillator{
    GDCLASS(SynthResonantOscillator,SynthFrequencyOscillator)
protected:
    static void _bind_methods();
private:
    //State variables
    float x = 0.0f;
    float y = 0.0f;

    //actual decay
    float decay = 0.995f;

public:
    virtual void initialize(SynthPatchLocals *locals) override;
    float decay_time = 0.5f;
    float excitation_strength = 1.0;
    godot::Ref<SynthOscillator> excitor;

    float process() override
    {
        float in = 0.0f;

        if (excitor.is_valid()){
            excitor->active = active;
            in = excitor->process()*excitor->gain;
        }
        
        x += in * excitation_strength;

        float pitchRatio = processPitch();
        
        float theta = Math_TAU * (frequency*pitchRatio) / sampleRate;

        float c = std::cos(theta);
        float s = std::sin(theta);

        float nx = c*x - s*y;
        float ny = s*x + c*y;

        x = nx * decay;
        y = ny * decay;

        return x;
    }

    void set_decay_time(const float x){decay_time = x;
        decay = std::exp(std::log(0.001f) / (decay_time * sampleRate));}
    float get_decay_time() const {return decay_time;}

    void set_excitation_strength(const float x) {excitation_strength = x;}
    float get_excitation_strength() const {return excitation_strength;}
    
    void set_excitor(const godot::Ref<SynthOscillator> x){excitor = x;}
    godot::Ref<SynthOscillator> get_excitor() const {return excitor;}

};

class SynthPhaseOscillator: public SynthFrequencyOscillator{
    GDCLASS(SynthPhaseOscillator,SynthFrequencyOscillator)
protected:
    static void _bind_methods();
protected:
    int duty_cycle = 1;//The duty cycle of the pulse oscillator. TODO - Set up _validate_property to hide this if the oscillator isn't set to pulse.
    bool has_fired = false;//for pulse oscillator - maybe useful for other weird kinds?
    bool one_shot = false;// EXPOSE THIS ONE FOR PULSE RESONATOR
    int one_shot_cooldown = sampleRate; //Cooldown on one-shot.

private:
    int one_shot_counter = 0;
public:
    void updatePhase() {
        float pitchRatio = processPitch();
        phase += (frequency*pitchRatio)/sampleRate;
        while(phase>=1.0f){
            phase-=1.0f;
            if(!one_shot){has_fired = false;}
            else if(one_shot_counter>=one_shot_cooldown){ //Increments the counter for the one shot cooldown.
                has_fired = false;
                one_shot_counter = 0;
            }
            else{ one_shot_counter ++;}
        }
    };
    enum wf{
        SINE,
        SQUARE,
        TRIANGLE,
        SAWTOOTH,
        PULSE
    };
    wf waveform = SINE;

    float process() override{
        updatePhase();
        switch(waveform){
            case SINE:
                return std::sin(Math_TAU * phase);
            case SQUARE:
                return phase < 0.5f ? 1.0f : -1.0f;
            case TRIANGLE:
                return 4.0f * std::fabs(phase - 0.5f) - 1.0f;
            case SAWTOOTH:
                return phase * 2.0f-1.0f;
            case PULSE:
                if(!has_fired){
                    has_fired = true;
                    return 1.0f;
                }
                else{return 0.0f;}
        }
    }

    //Setgets
    void set_waveform(const int newForm){waveform = static_cast<wf>(newForm);;}
    int get_waveform(){return waveform;}
    
};



class SynthGroupOscillator: public SynthOscillator{
    GDCLASS(SynthGroupOscillator,SynthOscillator)
protected:
    static void _bind_methods();

public:
    godot::TypedArray<SynthPhaseOscillator> oscillators;
    godot::Ref<SynthADSR> frequencyADSR;
    float min_semitones = -4.0f; //How many semitones below the target's note can it go?
    float max_semitones = 4.0f; //The upper end of the ADSR compared to the target's target frequency.

    void initialize(SynthPatchLocals *locals) override;

    float process() override;

    void note_on() override;
    void note_off() override;

    //setgets
    void set_oscillators(const godot::TypedArray<SynthPhaseOscillator> newOsc){oscillators = newOsc;};
    godot::TypedArray<SynthPhaseOscillator> get_oscillators(){return oscillators;};

    void set_freq_adsr(const godot::Ref<SynthADSR> newADSR){frequencyADSR = newADSR;};
    godot::Ref<SynthADSR> get_freq_adsr(){return frequencyADSR;};

    void set_min_semitones(const float x){min_semitones = x;}
    float get_min_semitones(){return min_semitones;}

    void set_max_semitones(const float x){max_semitones = x;}
    float get_max_semitones(){return max_semitones;}

};



class SynthFilter: public godot::Resource{
    GDCLASS(SynthFilter,godot::Resource)
protected:
    static void _bind_methods();
public:    
    
    SynthPatchLocals *synthLocals = nullptr;
    virtual void initialize(SynthPatchLocals *locals);
    
    float cutoff = 1000.0f;
    float envelopeAmount = 12.0f; //Semitones
    float gain = 1.0f;

    virtual void set_cutoff(const float newFreq){cutoff = newFreq;}
    virtual void set_envelope_amount(const float newFreq){envelopeAmount = newFreq;}
    virtual float get_cutoff(){return cutoff;}
    virtual float get_envelope_amount(){return envelopeAmount;}
    void set_gain(const float x){gain = x;}
    float get_gain() const{return gain;}

    virtual float process(float input,float envelopeRatio){return input*envelopeAmount*envelopeRatio;};
};

class SynthSVF : public SynthFilter{ // CODE FROM https://gist.github.com/hollance/2891d89c57adc71d9560bcf0e1e55c4b - THANKS!!
    GDCLASS(SynthSVF,SynthFilter)
protected:
    static void _bind_methods();
public:
    float Q = 0.1;
    
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
        float semitones = envelopeAmount*envelopeRatio;//Ratio is 0-1.0, envelopeAmount is in semitones
        // float semitonesRatio = std::pow(2.0f,semitones/12.0f);
        // float cutoff = minFrequencyHz*std::pow(maxFrequencyHz/minFrequencyHz,envelopeRatio); //EXPONENTIAL GAIN -- TODO: CHECK IF THIS SUCKS ASS
        float finalCutoff = SynthHelper::apply_semitone_offset(cutoff, semitones);
        setCoefficients(finalCutoff, Q);
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
    float g= 0.0f, k= 0.0f, a1= 0.0f, a2= 0.0f, a3= 0.0f;  // filter coefficients
    float ic1eq = 0.0f, ic2eq = 0.0f;      // internal state
    float sampleRate = (float)godot::AudioServer::get_singleton()->get_mix_rate();

};

class SynthParallelFilter: public SynthFilter{
    GDCLASS(SynthParallelFilter,SynthFilter)
protected:
    static void _bind_methods();
public:
    void initialize(SynthPatchLocals *locals) override;
    float process(float input,float envelopeRatio) override;
     godot::TypedArray<SynthFilter> filters;
     void set_filters(const godot::TypedArray<SynthFilter> newFilters){
        filters = newFilters;
        set_cutoff(cutoff);}//Force recalculate of resonance ratios.
    
    godot::TypedArray<SynthFilter> get_filters() const{return filters;}
};

//Resonator container for SVFs
class SynthHarmonicParallelFilter: public SynthParallelFilter{
    GDCLASS(SynthHarmonicParallelFilter,SynthParallelFilter);
protected:
    static void _bind_methods();
public:
   
    godot::PackedFloat32Array filterResonanceRatio;

    float fallbackFrequencyRatio = 2.0;

    void set_filter_resonance_ratios(const godot::PackedFloat32Array x){
        filterResonanceRatio = x;
        set_cutoff(cutoff);}//Force recalculate of resonance ratios.

    godot::PackedFloat32Array get_filter_resonance_ratios()const{return filterResonanceRatio;}

    void set_fallback_frequency_ratio(const float x){
        fallbackFrequencyRatio = x;
    set_cutoff(cutoff);}//Force recalculate of resonance ratios.

    float get_fallback_frequency_ratio()const{return fallbackFrequencyRatio;}

    void set_cutoff(const float newFreq) override;

};

//Resonator as a filter
class SynthResonantFilter: public SynthFilter{
    GDCLASS(SynthResonantFilter,SynthFilter)
protected:
    // int counter = 0;
    static void _bind_methods();
public:
    float decay = 1.0f;
    float excitation_strength = 1.0f;
    float wet = 1.0f;//wet is full resonator
    float dry = 0.0f;//dry is full input.
    SynthHelper::Note note = SynthHelper::A;
    int octave = 4;

    void setCoefficients(){
        r = exp(-6.9078f / (decay * sampleRate));;
        theta = Math_TAU * cutoff/sampleRate;

        a1 = 2.0f * r * std::cos(theta);
        a2 = -r * r;
        godot::print_line(godot::vformat("cutoff=%f r=%f a1=%f a2=%f",cutoff, r, a1, a2));
    };

    float process(float input, float envelopeRatio) override{
        // if(input!=0.0f){godot::print_line(input);}
        float y = excitation_strength*input + a1 * y1 + a2 * y2;
        y2 = y1;
        y1 = y;
        // if(std::abs(y)>0.1&&counter%1000==1){godot::print_line(y);}
        // counter++;
        return dry*input + wet*y;
    }

    //setgets
    void set_cutoff(const float x) override{
        cutoff = x;
        setCoefficients();
    }
    void set_decay(const float x){
        decay = x;
        setCoefficients();
    }//TODO: Set this up to remap to human readable format.
    float get_decay() const {return decay;}
    void set_excitation_strength(const float x) {excitation_strength = x;}
    void set_wet(const float x) {wet = x;}
    void set_dry(const float x) {dry = x;}
    float get_excitation_strength() const {return excitation_strength;}
    float get_wet() const {return wet;}
    float get_dry() const {return dry;}

    //For handling notes.
    void updateFrequency(){
        set_cutoff(SynthHelper::note_to_frequency(note, octave));
    }
    
    void set_note(const int new_note){note = static_cast<SynthHelper::Note>(new_note);updateFrequency();}
    int get_note()const{return note;}
    void set_octave(const int new_octave){octave = new_octave;updateFrequency();}
    int get_octave()const{return octave;}

private:
    float sampleRate = (float)godot::AudioServer::get_singleton()->get_mix_rate();
    //State variables
    float y1 = 0.0f;
    float y2 = 0.0f;
    //coefficients (Yes I run the calculations here again, sue me.)
    float r = 0.0f;
    float theta = 0.0f;
    float a1 = 0.0f;
    float a2 = 0.0f;
};


class SimpleSynthPatch: public godot::Resource{
    GDCLASS(SimpleSynthPatch, godot::Resource)
protected:
    static void _bind_methods();
public:

    SynthPatchLocals synthLocals;

    godot::TypedArray<SynthOscillator> oscillators;
    godot::TypedArray<SynthFilter> filters;
    godot::Ref<SynthADSR> filterADSR;
    godot::Ref<SynthADSR> preFilterADSR;
    godot::Ref<SynthADSR> postFilterADSR;
    

    float frequencyOffset = 0.0f;
    float amplitudeOffset = 0.0f;

    // Setters and getters
    void updateFilterADSR(float a,float d, float s, float r);
    void updatePreFilterADSR(float a,float d, float s, float r);
    void updatePostFilterADSR(float a,float d, float s, float r);
    void set_filter_adsr(const godot::Ref<SynthADSR> newADSR);
    void set_pre_filter_adsr(const godot::Ref<SynthADSR> newADSR);
    void set_post_filter_adsr(const godot::Ref<SynthADSR> newADSR);
    godot::Ref<SynthADSR> get_filter_adsr() const;
    godot::Ref<SynthADSR> get_pre_filter_adsr() const;
    godot::Ref<SynthADSR> get_post_filter_adsr() const;

    void note_on();
    void note_off();

    void set_pitch_bend(const float x){synthLocals.pitchBend = x;}
    float get_pitch_bend() const {return synthLocals.pitchBend;}

    void set_oscillators(const godot::TypedArray<SynthOscillator> newOsc);
    godot::TypedArray<SynthOscillator> get_oscillators() const;

    void set_filters(const godot::TypedArray<SynthFilter> newFilters);
    godot::TypedArray<SynthFilter> get_filters() const;

    void initialize(); //Initializes all children, passes the context down.

    //Processing
    float process();

};

