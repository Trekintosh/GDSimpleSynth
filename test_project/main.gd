extends Node2D

func _ready() -> void:
	test_plugin_functionality()

func test_plugin_functionality()->void:
	var SSG = SimpleSynthGenerator.new()
	SSG.generateTone()
	
	
	
	
	
	
