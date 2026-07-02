extends Node2D
@onready var audio_stream_player: AudioStreamPlayer = $AudioStreamPlayer
@onready var SSG: SimpleSynthStream = audio_stream_player.stream
func _ready() -> void:
	audio_stream_player.play()
	await get_tree().process_frame

func _process(_delta: float) -> void:
	if Input.is_action_just_pressed("ui_accept"):
		SSG.patch.note_on()
	elif Input.is_action_just_released("ui_accept"):
		SSG.patch.note_off()
