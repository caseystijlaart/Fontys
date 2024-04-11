from midi2audio import FluidSynth
from pydub import AudioSegment

def convert_midi_to_mp3(midi_file, soundfont_path, output_mp3):
    # Convert MIDI to WAV using FluidSynth
    fs = FluidSynth(soundfont_path)
    wav_file = "output.wav"
    fs.midi_to_audio(midi_file, wav_file)

    # Convert WAV to MP3 using Pydub
    audio = AudioSegment.from_wav(wav_file)
    audio.export(output_mp3, format="mp3")

if __name__ == "__main__":
    # Replace 'generated_music.mid' with the name of your generated MIDI file
    midi_file_path = "generated_music.mid"
    soundfont_path = "soundfonts/GeneralUser GS 1.471/GeneralUser GS v1.471.sf2"
    output_mp3_path = "generated_music.mp3"

    convert_midi_to_mp3(midi_file_path, soundfont_path, output_mp3_path)
