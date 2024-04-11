import random
from midiutil import MIDIFile
import json

def map_value_to_intensity(value, min_value, max_value):
    # Map the value to an intensity level between 60 and 100
    return int(((value - min_value) / (max_value - min_value)) * 40 + 60)

def map_temperature_to_pitch(temperature):
    # Map temperature to MIDI pitch within the range of 0 to 127
    return int((temperature / 40) * 127)

def map_humidity_to_duration(humidity):
    # Map humidity to note duration
    return 1 / (humidity / 50)  # Adjust the scaling as needed

def map_intensity_to_instrument(intensity):
    # Map intensity to instrument
    if intensity < 70:
        return 0  # Example instrument number for low intensity
    elif 70 <= intensity < 74:
        return 52  # Example instrument number for medium intensity
    elif 74 <= intensity < 78:
        return 27  # Example instrument number for medium intensity
    elif 78 <= intensity < 82:
        return 55  # Example instrument number for medium intensity
    else:
        return 30  # Example instrument number for high intensity

def generate_music_from_environment(json_data):
    # Parse JSON data
    data = json.loads(json_data)

    # Create a MIDI file with two tracks (one for temperature, one for humidity)
    midi = MIDIFile(2)

    midi.addTempo(0, 0, 120)

    # Generate musical composition based on the environmental data
    for i, entry in enumerate(data):
        # Extract data from JSON
        temperature = entry.get("temperature", 0)
        humidity = entry.get("humidity", 0)

        # Map values to intensity for instrument selection
        temperature_intensity = map_value_to_intensity(temperature, 0, 35)
        humidity_intensity = map_value_to_intensity(humidity, 30, 75)

        # Map intensity to instrument
        temperature_instrument = map_intensity_to_instrument(temperature_intensity)
        humidity_instrument = map_intensity_to_instrument(humidity_intensity)

        # Assign instruments to tracks
        midi.addProgramChange(0, 0, i, temperature_instrument)  # Track 0 for temperature
        midi.addProgramChange(1, 0, i, humidity_instrument)  # Track 1 for humidity

        # Map temperature to pitch and humidity to duration
        pitch = map_temperature_to_pitch(temperature)
        duration = map_humidity_to_duration(humidity)

        # Add notes to tracks
        midi.addNote(0, i, pitch, i, 2, 100)  # Track 0 for temperature
        midi.addNote(1, i, pitch + 12, i, duration, 100)  # Track 1 for humidity

    # Save the generated MIDI file
    with open("generated_music.mid", "wb") as midi_file:
        midi.writeFile(midi_file)

if __name__ == "__main__":
    # Example: Replace this with actual JSON data
    environmental_data_json = '''
    [
        {"temperature": 20, "humidity": 40},
        {"temperature": 22, "humidity": 45},
        {"temperature": 25, "humidity": 50},
        {"temperature": 23, "humidity": 48},
        {"temperature": 18, "humidity": 42},
        {"temperature": 19, "humidity": 44},
        {"temperature": 21, "humidity": 46},
        {"temperature": 24, "humidity": 49}
    ]
    '''

    # Generate music based on environmental data in JSON format
    generate_music_from_environment(environmental_data_json)