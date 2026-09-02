@tool
extends EditorPlugin

var native_plugin

func _enter_tree(): native_plugin = SynthEditorPlugin.new(); native_plugin.start()

func _exit_tree(): native_plugin.stop(); native_plugin.free(); native_plugin = null
