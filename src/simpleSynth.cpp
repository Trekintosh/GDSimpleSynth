#include "simpleSynth.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/core/object.hpp"
#include "simpleSynthPatch.hpp"
#include "godot_cpp/classes/audio_stream_generator_playback.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"

using namespace godot;

void SimpleSynth::generateSound() 
{
    if(myPlayback.is_null()|myPatch.is_null()){
        return;
    }
    int availableFrames = myPlayback->get_frames_available();
    PackedVector2Array bufferArray;
    bufferArray.resize(availableFrames);
    for (int i=0; i<availableFrames;i++){
        float output = myPatch->process();
        bufferArray[i] = (Vector2(output,output));
    }
    myPlayback->push_buffer(bufferArray);
};

void SimpleSynth::_process(double delta){
    generateSound();
};

void SimpleSynth::set_patch(const Ref<SimpleSynthPatch> newPatch){
    myPatch = newPatch;
}

Ref<SimpleSynthPatch> SimpleSynth::get_patch() const{
    return myPatch;
}

void SimpleSynth::set_playback(const godot::Ref<godot::AudioStreamGeneratorPlayback> p){
    myPlayback = p;
};

void SimpleSynth::_bind_methods()
{
    // ClassDB::bind_method(D_METHOD("generateSound"), &SimpleSynth::generateSound);
    ClassDB::bind_method(D_METHOD("set_playback","playback"), &SimpleSynth::set_playback);
    ClassDB::bind_method(D_METHOD("set_patch","New Patch"),&SimpleSynth::set_patch);
    ClassDB::bind_method(D_METHOD("get_patch"),&SimpleSynth::get_patch);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"patch",PROPERTY_HINT_RESOURCE_TYPE,"SimpleSynthPatch"), "set_patch", "get_patch");
};
