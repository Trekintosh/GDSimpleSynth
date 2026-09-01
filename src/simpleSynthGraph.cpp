#include "simpleSynthGraph.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/popup_menu.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/variant/string_name.hpp"
#include "godot_cpp/variant/vector2.hpp"

using namespace godot;

SynthGraphNode::SynthGraphNode(){
    
}

void SynthGraphNode::setup_test_node(){
    set_title(String(get_name()));
    set_size(Vector2(256,256));

    Label *input_label = memnew(Label);
    input_label->set_text("Input");
    add_child(input_label);

    set_slot(0, true, 1, Color(1,1,1), true, 1, Color(1,1,1));
}

void SynthGraphNode::_bind_methods(){

}

SynthGraphEditor::SynthGraphEditor(){
    
}

void SynthGraphEditor::start(){
    //Instantiate and label graphEdit.
    graph_edit = memnew(GraphEdit);
    graph_edit->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
    add_child(graph_edit);

    Label *label = memnew(Label);
    label->set_text("SimpleSynth Editor");
    graph_edit->add_child(label);

    //Set up the add node menu
    add_node_menu = memnew(PopupMenu);
    add_child(add_node_menu);

    add_node_menu->add_item("LFO",1);
    
    print_line("SynthGraphEditor CONSTRUCTOR: " + String::num_uint64((uint64_t)this));
    graph_edit->connect("popup_request",Callable(this,"show_add_node_menu"));
    add_node_menu->connect("id_pressed",Callable(this,"add_node_menu_selected"));

    //Deletion and copy/paste
    graph_edit->connect("delete_nodes_request",Callable(this,"delete_nodes"));
}

void SynthGraphEditor::show_add_node_menu(const Vector2 &position){
    print_line("Node Menu Requested.");
    add_node_position = (position + graph_edit->get_scroll_offset()) / graph_edit->get_zoom();
    add_node_menu->set_position(graph_edit->get_screen_position()+position);
    add_node_menu->popup();
}

void SynthGraphEditor::add_node_menu_selected(int32_t id){
    //instantiate node
    SynthGraphNode *node = memnew(SynthGraphNode);
    node->set_title("Hello World!");
    node->set_node_id(next_node_id++);
    node->set_name("Node_"+String::num_uint64(node->get_node_id()));

    //Update IDs
    //TODO: Move graph_nodes to the serialization authority of truth.
    graph_nodes[node->get_node_id()] = node;
    graph_node_names[node->get_name()] = node->get_node_id();

    graph_edit->add_child(node);
    
    //Update position and run init script
    //TODO: Genericize init script.
    node->set_position_offset(add_node_position);
    node->setup_test_node();
}

void SynthGraphEditor::delete_nodes(const Array &nodes){
    print_line("Delete Nodes requested.");
    print_line(nodes);
    for(int i=0; i<nodes.size(); ++i){
        
        StringName node_name = nodes[i];
        uint32_t id = graph_node_names[node_name];

        graph_nodes[id]->queue_free();
        graph_nodes.erase(id);
        graph_node_names.erase(node_name);
        
    }
}

void SynthGraphEditor::_bind_methods(){
    ClassDB::bind_method(D_METHOD("show_add_node_menu", "position"), &SynthGraphEditor::show_add_node_menu);
    ClassDB::bind_method(D_METHOD("add_node_menu_selected", "id"), &SynthGraphEditor::add_node_menu_selected);
    ClassDB::bind_method(D_METHOD("delete_nodes", "nodes"), &SynthGraphEditor::delete_nodes);
}


using namespace godot;

void SynthEditorPlugin::_bind_methods() {
    ClassDB::bind_method(D_METHOD("start"), &SynthEditorPlugin::start);
    ClassDB::bind_method(D_METHOD("stop"), &SynthEditorPlugin::stop);
}

void SynthEditorPlugin::start() {
    print_line("SynthEditorPlugin::start: " + String::num_uint64((uint64_t)this));

    if (graph_editor) {
        print_line("Already have graph editor!");
        return;
    }

    graph_editor = memnew(SynthGraphEditor);
    graph_button = add_control_to_bottom_panel(graph_editor, "Synth Graph");
    graph_editor->start();
}

void SynthEditorPlugin::stop() {
    if (!graph_editor) {
        return;
    }

    remove_control_from_bottom_panel(graph_editor);

    graph_editor->queue_free();
    graph_editor = nullptr;
    graph_button = nullptr;
}
