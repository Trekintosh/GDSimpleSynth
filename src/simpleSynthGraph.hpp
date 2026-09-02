#pragma once

#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/editor_plugin.hpp"
#include "godot_cpp/classes/popup_menu.hpp"
#include "godot_cpp/templates/hash_map.hpp"
#include "godot_cpp/variant/variant.hpp"
#include <cstdint>
#include <godot_cpp/classes/graph_node.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/classes/graph_edit.hpp>

using namespace godot;

enum class GraphPortType{
    PARAMETER,
    AUDIO
};

struct SynthGraphPort{
    GraphPortType type;
    
};

class SynthGraphNode : public GraphNode{
    GDCLASS(SynthGraphNode,GraphNode);

    protected:
        static void _bind_methods();
    
    private:
        uint32_t node_id = -1;

    public:
        SynthGraphNode();
        void setup_test_node();

        void set_node_id(uint32_t p_id){node_id = p_id;}
        uint32_t get_node_id() const {return node_id;}
        // private:
        //     std::vector<GraphPort> inputs;
        //     std::vector<GraphPort> outputs;

        //     void rebuild_ports();
        //     void clear_port_rows();
        
        // protected:
        //     void _notification(int32_t what);
        
        // public:
        //     SynthGraphNode();
        //     ~SynthGraphNode();

        //     void set_inputs(const Array &p_inputs);
        //     Array get_inputs() const;

        //     void set_outputs(const Array &p_inputs);
        //     Array get_outputs() const;

        //     void add_input(const StringName &p_name, int32_t p_type = 0, const Color &p_color = Color(1,1,1,1));
        //     void add_output(const StringName &p_name, int32_t p_type = 0, const Color &p_color = Color(1,1,1,1));

        //     void remove_input(int32_t p_index);
        //     void remove_output(int32_t p_index);

        //     void clear_inputs();
        //     void clear_outputs();
            
};


class SynthGraphEditor: public Control{
    GDCLASS(SynthGraphEditor,Control);

    protected:
        static void _bind_methods();

    private:
        GraphEdit *graph_edit = nullptr;
        PopupMenu *add_node_menu = nullptr;
        Vector2 add_node_position;

        uint32_t next_node_id = 1;
        HashMap<uint32_t, SynthGraphNode *> graph_nodes;
        HashMap<StringName, uint32_t> graph_node_names;

        void show_add_node_menu(const Vector2 &position);
        void add_node_menu_selected(int32_t id);

        void delete_nodes(const Array &nodes);

        void connection_requested(const StringName &from_node, int32_t from_port, const StringName &to_node, int32_t to_port);
        void disconnection_requested(const StringName &from_node, int32_t from_port, const StringName &to_node, int32_t to_port);
    
    public:
        SynthGraphEditor();
        ~SynthGraphEditor(){print_line("SynthGraphEditor DESTRUCTOR: " + String::num_uint64((uint64_t)this));}
        
        void start();

};


class SynthEditorPlugin : public EditorPlugin {
    GDCLASS(SynthEditorPlugin, EditorPlugin)

protected:
    static void _bind_methods();

private:
    SynthGraphEditor *graph_editor = nullptr;
    Button *graph_button = nullptr;

public:
    SynthEditorPlugin() {print_line("SynthEditorPlugin CONSTRUCTOR: " + String::num_uint64((uint64_t)this));};//= default;

    void start();
    void stop();
};
