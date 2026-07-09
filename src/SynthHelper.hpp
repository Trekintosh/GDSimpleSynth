#pragma once

#include <godot_cpp/classes/object.hpp>

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
