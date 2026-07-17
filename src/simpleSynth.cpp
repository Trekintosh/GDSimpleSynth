#include "simpleSynth.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/core/object.hpp"
#include "simpleSynthPatch.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"

using namespace godot;

void SimpleSynthStream::set_patch(const Ref<SimpleSynthPatch> newPatch){
    myPatch = newPatch;
}

Ref<SimpleSynthPatch> SimpleSynthStream::get_patch() const{
    return myPatch;
}

Ref<AudioStreamPlayback> SimpleSynthStream::_instantiate_playback() const{
    Ref<SimpleSynthPlayback> pb;
    pb.instantiate();
    pb->set_stream(this);
    pb->patch = myPatch;
    return pb;
}

void SimpleSynthStream::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_sequencer_enabled", "enabled"), &SimpleSynthStream::set_sequencer_enabled);
    ClassDB::bind_method(D_METHOD("get_sequencer_enabled"), &SimpleSynthStream::get_sequencer_enabled);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "Sequencer Enabled"), "set_sequencer_enabled", "get_sequencer_enabled");
    
    ClassDB::bind_method(D_METHOD("set_patch","New Patch"),&SimpleSynthStream::set_patch);
    ClassDB::bind_method(D_METHOD("get_patch"),&SimpleSynthStream::get_patch);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"patch",PROPERTY_HINT_RESOURCE_TYPE,"SimpleSynthPatch"), "set_patch", "get_patch");
};



///PLAYBACK///

int SimpleSynthPlayback::_mix(godot::AudioFrame *buffer, float rate_scale, int frames) 
{
    if(patch.is_null()){
        return 0;
    }
    for (int i=0; i<frames;i++){
        float output = patch->process();
        buffer[i] = {output,output};
    }
    return frames;
};



void SimpleSynthPlayback::_bind_methods(){
    //None.
}

