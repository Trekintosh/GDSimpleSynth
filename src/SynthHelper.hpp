#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <vector>

class SynthHelper : public godot::Object{
    GDCLASS(SynthHelper,godot::Object)

protected:
    static void _bind_methods();
public:
    enum Note {
        C,
        C_SHARP,
        D,
        D_SHARP,
        E,
        F,
        F_SHARP,
        G,
        G_SHARP,
        A,
        A_SHARP,
        B
    };

    static constexpr float A4_FREQUENCY = 440.0f;

    //Midi - note stuff
    static int note_to_midi(int note, int octave);
    static float midi_to_frequency(float midi);
    static float note_to_frequency(int note, int octave);
    static int frequency_to_midi(float frequency);

    //Semitone stuff
    static float semitones_to_ratio(float semitones);
    static float ratio_to_semitones(float ratio);
    static float transpose_frequency(float frequency, float semitones);
    static float apply_semitone_offset(float frequency, float semitones);
};

class SynthDelayLine{
public:
    SynthDelayLine(int size = 4096);
    SynthDelayLine(const SynthDelayLine&) = delete;
    SynthDelayLine& operator=(const SynthDelayLine&) = delete; //Stops it from being copied. 4kb buffer is a lot to accidentally clone or remake in a loop.
    
    void resize(int size);
    void clear();

    void set_delay(float samples);
    void set_smoothing(float newSmooth){smoothing = newSmooth;}

    float read();
    void write(float input);

private:
    std::vector<float> buffer;

    int writeIndex = 0;
    float delayCurrent = 1.0f;
    float delayTarget = 1.0f;

    float smoothing = 0.001f;
};
