# main.py

from AlgorithmicComposition import AlgorithmicComposition

if __name__ == "__main__":
    environmental_data_json = '''
    [
        {"temperature": 10, "humidity": 40},
        {"temperature": 12, "humidity": 45},
        {"temperature": 15, "humidity": 50},
        {"temperature": 13, "humidity": 48},
        {"temperature": 8, "humidity": 42},
        {"temperature": 9, "humidity": 44},
        {"temperature": 11, "humidity": 46},
        {"temperature": 14, "humidity": 49},
        {"temperature": 20, "humidity": 30},
        {"temperature": 22, "humidity": 35},
        {"temperature": 25, "humidity": 40},
        {"temperature": 23, "humidity": 38},
        {"temperature": 28, "humidity": 52},
        {"temperature": 29, "humidity": 54},
        {"temperature": 21, "humidity": 56},
        {"temperature": 24, "humidity": 59},
        {"temperature": 10, "humidity": 40},
        {"temperature": 12, "humidity": 45},
        {"temperature": 15, "humidity": 50},
        {"temperature": 13, "humidity": 58},
        {"temperature": 8, "humidity": 56},
        {"temperature": 9, "humidity": 64},
        {"temperature": 11, "humidity": 66},
        {"temperature": 14, "humidity": 69},
        {"temperature": 20, "humidity": 70}
    ]
    '''

    algorithm = AlgorithmicComposition()
    algorithm.init()
    algorithm.generate_composition(environmental_data_json)
