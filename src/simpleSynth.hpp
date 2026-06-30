#pragma once

#include "godot_cpp/classes/audio_stream_generator_playback.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/wrapped.hpp"
class SimpleSynth: public godot::Node{
    GDCLASS(SimpleSynth, godot::Node)
protected:
    static void _bind_methods();
public:
    godot::Ref<godot::AudioStreamGeneratorPlayback> myPlayback;
    void set_playback(const godot::Ref<godot::AudioStreamGeneratorPlayback> p);
    void generateNoise();
    void _process(double delta) override;
};
