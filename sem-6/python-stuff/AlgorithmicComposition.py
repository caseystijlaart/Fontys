# AlgorithmicComposition.py

from midiutil import MIDIFile
from IAlgorithm import IAlgorithm
import json

class AlgorithmicComposition(IAlgorithm):
    def __init__(self):
        self.midi = MIDIFile(0)  # Initialize with 0 tracks

    def init(self):
        print("Initializing AlgorithmicComposition")

    def generate_composition(self, json_data):
        data = json.loads(json_data)
        self.midi = MIDIFile(len(data))  # Adjust the number of tracks based on the number of fields

        self.midi.addTempo(0, 0, 120)

        for i, entry in enumerate(data):
            for field_name, field_value in entry.items():
                intensity = self.map_value_to_intensity(field_value, 0, 35)
                instrument = self.map_intensity_to_instrument(intensity)
                pitch = self.map_field_to_pitch(field_value)  # Modify this function based on field type

                track_number = i  # Each field type has its own track
                self.midi.addProgramChange(track_number, 0, i, instrument)
                self.midi.addNote(track_number, i, pitch, i, 2, 100)

        with open("generated_music.mid", "wb") as midi_file:
            self.midi.writeFile(midi_file)

    def map_value_to_intensity(self, value, min_value, max_value):
        return int(((value - min_value) / (max_value - min_value)) * 40 + 60)

    def map_field_to_pitch(self, field_value):
        # Modify this function based on the type of field
        # For now, it assumes a simple mapping similar to temperature
        return int((field_value / 40) * 127)

    def map_intensity_to_instrument(self, intensity):
        if intensity < 70:
            return 0
        elif 70 <= intensity < 74:
            return 52
        elif 74 <= intensity < 78:
            return 27
        elif 78 <= intensity < 82:
            return 55
        else:
            return 30
