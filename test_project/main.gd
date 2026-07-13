extends Node2D
@onready var audio_stream_player: AudioStreamPlayer = $AudioStreamPlayer
@onready var SSG: SimpleSynthStream = audio_stream_player.stream
@onready var oscillators = SSG.patch.Oscillators

func _ready() -> void:
	audio_stream_player.play()
	await get_tree().process_frame
var time = 0.0
func _process(_delta: float) -> void:
	time+=_delta
	if Input.is_action_just_pressed("ui_accept"):
		SSG.patch.note_on()
	elif Input.is_action_just_released("ui_accept"):
		SSG.patch.note_off()
		
	if Input.is_action_pressed("ui_down"):
		SSG.patch.set_modulation_value_by_name("testes",sin(time))
	
	#if Input.is_mouse_button_pressed(MOUSE_BUTTON_LEFT):
		#SSG.patch.set_pitch_bend(lerpf(SSG.patch.get_pitch_bend(),get_global_mouse_position().y/get_viewport().size.y,_delta*5))
		#print(get_global_mouse_position().y/get_viewport().size.y)
		#
		##quill testing
		#var x = clamp(get_global_mouse_position().x/get_viewport().size.x,0,2)
		#for osc in oscillators:
			#if osc.has_method("set_cutoff"):
				#osc.set_cutoff(x)
		#
	#else:
		#SSG.patch.set_pitch_bend(lerpf(SSG.patch.get_pitch_bend(),0,_delta*5));
		#for osc in oscillators:
			#if osc.has_method("set_cutoff"):
				#osc.set_cutoff(lerpf(osc.get_cutoff(),0,_delta*15))



func _on_check_box_toggled(toggled_on: bool) -> void:
	if toggled_on:
		SSG.patch.note_on()
	else:
		SSG.patch.note_off()
		
		
