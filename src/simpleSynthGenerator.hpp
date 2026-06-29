#pragma once

#include "godot_cpp/classes/audio_stream_generator.hpp"
#include "godot_cpp/classes/wrapped.hpp"
class SimpleSynthGenerator: public godot::AudioStreamGenerator{
    GDCLASS(SimpleSynthGenerator, AudioStreamGenerator)
protected:
    static void _bind_methods();
private:
    void generateTone();

};
