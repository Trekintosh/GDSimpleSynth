extends Node2D
@onready var audio_stream_player: AudioStreamPlayer = $AudioStreamPlayer
@onready var SSG: SimpleSynth = $SimpleSynth
func _ready() -> void:
	audio_stream_player.play()
	await get_tree().process_frame
	SSG.set_playback(audio_stream_player.get_stream_playback())

func _process(_delta: float) -> void:
	if Input.is_action_just_pressed("ui_accept"):
		SSG.patch.note_on()
	elif Input.is_action_just_released("ui_accept"):
		SSG.patch.note_off()
