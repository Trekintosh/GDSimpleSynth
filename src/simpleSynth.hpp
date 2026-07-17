#pragma once

#include "godot_cpp/classes/audio_frame.hpp"
#include "godot_cpp/classes/audio_server.hpp"
#include "godot_cpp/classes/audio_stream.hpp"
#include "godot_cpp/classes/audio_stream_playback.hpp"
#include "godot_cpp/variant/string.hpp"
#include "simpleSynthPatch.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include <cstdint>

class SimpleSynthStream: public godot::AudioStream{
    GDCLASS(SimpleSynthStream, godot::AudioStream)
protected:
    static void _bind_methods();
public:
    bool sequencer_enabled = true;
    godot::Ref<SimpleSynthPatch> myPatch;

    //Setgets
    void set_sequencer_enabled(const bool set);
    bool get_sequencer_enabled() const;


    void set_patch(const godot::Ref<SimpleSynthPatch> newPatch);
    godot::Ref<SimpleSynthPatch> get_patch() const;

    //BIG PILE OF OVERRIDES WOOO
    godot::Ref<godot::AudioStreamPlayback> _instantiate_playback() const override; // The only important one here.
    
    bool _is_monophonic() const override {return true;};
    double _get_length() const override {return 0.0;};
    godot::String _get_stream_name() const override {return "Simple Synth";};
    double _get_bpm() const override {return 0;};
    int32_t _get_beat_count() const override {return 0;};
    void set_position(uint64_t pos){return;};
    void reset()const{return;};


};

class SimpleSynthPlayback: public godot::AudioStreamPlayback{
    GDCLASS(SimpleSynthPlayback,godot::AudioStreamPlayback)
protected:
    static void _bind_methods();
public:
    const SimpleSynthStream *myStream = nullptr;
    void set_stream(const SimpleSynthStream *p_stream) {
        // godot::print_line(p_stream);
        myStream = p_stream;
    }

    godot::Ref<SimpleSynthPatch> patch;
    int _mix(godot::AudioFrame *buffer, float rate_scale, int frames) override;

    //Mandatory overrides
    int get_loop_count() const{return 0;};
    double _get_playback_position() const override{return 0.0;};
    void _start(double p_from_pos = 0.0) override{
        patch->initialize();
        return;};
    void _stop() override{return;}


private:
    int sampleRate = godot::AudioServer::get_singleton()->get_mix_rate();

};
