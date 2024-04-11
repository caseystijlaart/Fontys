from abc import ABC, abstractmethod
import json

class IAlgorithm(ABC):
    @abstractmethod
    def init(self):
        pass

    @abstractmethod
    def generate_composition(self, data: json):
        pass