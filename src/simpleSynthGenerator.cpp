#include "simpleSynthGenerator.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/print_string.hpp"

using namespace godot;

void SimpleSynthGenerator::generateTone() 
{
    print_line("Hello World!");
}

void SimpleSynthGenerator::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("generateTone"), &SimpleSynthGenerator::generateTone);
}
