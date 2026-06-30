extends Node2D
@onready var audio_stream_player: AudioStreamPlayer = $AudioStreamPlayer
@onready var SSG: SimpleSynth = SimpleSynth.new()
func _ready() -> void:
	get_tree().root.add_child.call_deferred(SSG)
	audio_stream_player.play()
	await get_tree().process_frame
	SSG.set_playback(audio_stream_player.get_stream_playback())

func _process(_delta: float) -> void:
	pass
