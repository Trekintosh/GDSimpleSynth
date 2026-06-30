#include "simpleSynth.hpp"
#include "godot_cpp/classes/audio_stream_generator_playback.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include <cstdlib>

using namespace godot;

void SimpleSynth::generateNoise() 
{
    PackedVector2Array bufferArray;
    int availableFrames = myPlayback->get_frames_available();
    for (int i=0; i<availableFrames;i++){
        double randomDude = ((float)rand()/RAND_MAX)*2.0-1.0;
        bufferArray.append(Vector2(randomDude,randomDude));
    }
    myPlayback->push_buffer(bufferArray);
};

void SimpleSynth::_process(double delta){
    
    
    
};

void SimpleSynth::set_playback(const godot::Ref<godot::AudioStreamGeneratorPlayback> p){
    myPlayback = p;
};

void SimpleSynth::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("generateNoise"), &SimpleSynth::generateNoise);
    ClassDB::bind_method(D_METHOD("set_playback","playback"), &SimpleSynth::set_playback);
};
