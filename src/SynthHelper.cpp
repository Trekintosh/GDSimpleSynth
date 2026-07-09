#include "SynthHelper.hpp"
#include "godot_cpp/core/class_db.hpp"

using namespace godot;

void SynthHelper::_bind_methods(){

    ClassDB::bind_static_method("SynthHelper",D_METHOD("note_to_frequency", "note", "octave"),&SynthHelper::note_to_frequency);
    ClassDB::bind_static_method("SynthHelper",D_METHOD("midi_to_frequency", "midi"),&SynthHelper::midi_to_frequency);
    ClassDB::bind_static_method("SynthHelper",D_METHOD("frequency_to_midi", "frequency"),&SynthHelper::frequency_to_midi);
    ClassDB::bind_static_method("SynthHelper",D_METHOD("semitones_to_ratio", "semitones"),&SynthHelper::semitones_to_ratio);
    ClassDB::bind_static_method("SynthHelper",D_METHOD("ratio_to_semitones", "ratio"),&SynthHelper::ratio_to_semitones);
    ClassDB::bind_static_method("SynthHelper",D_METHOD("transpose_frequency", "frequency", "semitones"),&SynthHelper::transpose_frequency);
}

int SynthHelper::note_to_midi(int note, int octave){return (octave+1)*12+static_cast<int>(note);}

float SynthHelper::midi_to_frequency(float midi){return A4_FREQUENCY*std::pow(2.0f,(midi-69.0f)/12.0f);}

float SynthHelper::note_to_frequency(int note, int octave){return midi_to_frequency(note_to_midi(note,octave));}

int SynthHelper::frequency_to_midi(float frequency){return static_cast<int>(std::round(69.0f+12.0f*std::log2(frequency/A4_FREQUENCY)));}

float SynthHelper::semitones_to_ratio(float semitones){return std::pow(2.0f,semitones/12.0f);}

float SynthHelper::ratio_to_semitones(float ratio){return 12.0f*std::log2(ratio);}

float SynthHelper::transpose_frequency(float frequency, float semitones){return frequency*semitones_to_ratio(semitones);}

float SynthHelper::apply_semitone_offset(float frequency, float semitones){return frequency*semitones_to_ratio(semitones);}
