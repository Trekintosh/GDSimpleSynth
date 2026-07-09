// Include your classes, that you want to expose to Godot

#include "simpleSynth.hpp"
#include "simpleSynthPatch.hpp"
#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level)
{
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	// Register your classes here, so they are available in the Godot editor and engine
	GDREGISTER_CLASS(SynthHelper)
	GDREGISTER_CLASS(SimpleSynthStream)
	GDREGISTER_CLASS(SimpleSynthPlayback)
	GDREGISTER_CLASS(SimpleSynthPatch)
	GDREGISTER_CLASS(SynthADSR)
	GDREGISTER_CLASS(SynthLFO)
	GDREGISTER_ABSTRACT_CLASS(SynthOscillator)
	GDREGISTER_CLASS(SynthNoiseOscillator)
	GDREGISTER_CLASS(SynthPhaseOscillator)
	GDREGISTER_CLASS(SynthGroupOscillator)
	GDREGISTER_ABSTRACT_CLASS(SynthFilter)
	GDREGISTER_CLASS(SynthSVF)
	GDREGISTER_CLASS(SynthParallelFilter)
	GDREGISTER_CLASS(SynthHarmonicParallelFilter)
	GDREGISTER_CLASS(SynthResonator)
	GDREGISTER_CLASS(SimpleSynthSequencerSettings)
}

void uninitialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C"
{
	// Initialization
	GDExtensionBool GDE_EXPORT gdsimplesynth_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
	{
		GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		init_obj.register_initializer(initialize_gdextension_types);
		init_obj.register_terminator(uninitialize_gdextension_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}
