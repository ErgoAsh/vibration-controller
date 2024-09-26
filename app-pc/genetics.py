from __future__ import annotations
import os
import numpy as np
import random
import yaml
import threading
import pandas as pd
from collections import defaultdict
from enums import ToDeviceCommand
from config_handler import config, get_updated_dict, save_config
from scipy.signal import hilbert
from scipy.signal import find_peaks
from scipy.interpolate import interp1d


class Individual:
    def __init__(
        self,
        x=None,
        v=None,
        a=None,
        u=None,
        regulation_data: pd.DataFrame = None,
        response_data: pd.DataFrame = None,
        individual_id=0,
        generation_id=0,
    ):
        self.individual_id = individual_id
        self.generation_id = generation_id

        if response_data is not None:
            self.response_data = response_data.copy()
        else:
            self.load_response()

        if generation_id == 0:
            self.regulation_data = self.generate_new_regulation_data(x, v, a, u)
            return

        if regulation_data is not None:
            self.regulation_data = regulation_data.copy()
        else:
            self.load_regulation()

    @property
    def regulation_data(self):
        return self._name

    @regulation_data.setter
    def regulation_data(self, value):
        self._name = value

    @property
    def response_data(self):
        return self._name

    @response_data.setter
    def response_data(self, value):
        self._name = value

    def load_response(self):
        try:
            self.regulation_data = pd.read_csv(
                f"./generations/{self.generation_id}/{self.individual_id}_response.csv"
            )
        except FileNotFoundError:
            pass

    def load_regulation(self):
        try:
            self.regulation_data = pd.read_csv(
                f"./generations/{self.generation_id}/{self.individual_id}_regulation.csv"
            )
        except FileNotFoundError:
            pass

    def save_regulation(self):
        self.response_data.to_csv(
            f"./generations/{self.generation_id}/{self.individual_id}_regulation.csv"
        )

    def save_response(self):
        if self.response_data is not None:
            if not os.path.exists(f"./generations/{self.generation_id}/"):
                os.makedirs(f"./generations/{self.generation_id}/")

            self.response_data.to_csv(
                f"./generations/{self.generation_id}/{self.individual_id}_response.csv"
            )

            if "generations" not in config:
                config["generations"] = dict()

            if self.generation_id not in config["generations"]:
                config["generations"][self.generation_id] = dict()

            config["generations"][self.generation_id][self.individual_id] = {
                "t_reg": self.calc_t_reg(),
                "x_at_8": self.calc_x_at_8(),
                "x_max": self.calc_x_max(),
            }
            save_config()

    def generate_new_regulation_data(self, x=None, v=None, a=None, u=None):
        regulation_data = pd.DataFrame()
        regulation_data["x"] = (
            x if x is not None else np.random.uniform(-4000, 4000, 1000)
        )
        regulation_data["v"] = (
            v if v is not None else np.random.uniform(-4000, 4000, 1000)
        )
        regulation_data["a"] = (
            a if a is not None else np.random.uniform(-4000, 4000, 1000)
        )
        regulation_data["u"] = (
            u if u is not None else np.random.uniform(-1000, 1000, 1000)
        )
        return regulation_data

    def has_received_response(self):
        return self.received_response

    def get_id(self):
        return self.individual_id

    def get_regulation_data(self):
        return self.regulation_data

    def get_response_data(self):
        return self.regulation_data

    def get_x(self, x):
        return self.regulation_data["x"]

    def get_v(self, v):
        return self.regulation_data["v"]

    def get_a(self, a):
        return self.regulation_data["a"]

    def get_u(self, u):
        return self.regulation_data["u"]

    def set_x(self, x):
        self.regulation_data["x"] = x

    def set_v(self, v):
        self.regulation_data["v"] = v

    def set_a(self, a):
        self.regulation_data["a"] = a

    def set_u(self, u):
        self.regulation_data["u"] = u

    def fitness(self):
        ts = self.calc_t_reg()
        x8 = self.calc_x_at_8()
        return ts + 10 * x8

    def calc_peaks(self):
        peaks, _ = find_peaks(self.get_response_data()["x"], prominence=500)
        min_peaks, _ = find_peaks(-self.get_response_data()["x"], prominence=500)

        # Find all positive peaks
        interp_upper = interp1d(
            self.get_response_data()["t"].iloc[peaks],
            self.get_response_data()["x"].iloc[peaks],
            kind="linear",
            fill_value="extrapolate",
        )

        # Find all negitive peaks
        interp_lower = interp1d(
            self.get_response_data()["t"].iloc[min_peaks],
            self.get_response_data()["x"].iloc[min_peaks],
            kind="linear",
            fill_value="extrapolate",
        )

        self.get_response_data()["x_up"] = interp_upper(self.get_response_data()["t"])
        self.get_response_data()["x_down"] = interp_lower(self.get_response_data()["t"])

    def calc_t_reg(self):
        if "x_up" not in self.response_data.columns:
            self.calc_peaks()

        x_threshold = 0.15 * self.calc_x_max()
        t_calibration = self.get_response_data()["t"].iloc[
            np.argmax(self.get_response_data()["x_up"] < x_threshold)
        ]
        return t_calibration

    def calc_x_at_8(self):
        if "x_up" not in self.response_data.columns:
            self.calc_peaks()

        x_end = (
            self.get_response_data()["x_up"].iloc[-1]
            - self.get_response_data()["x_down"].iloc[-1]
        )
        return x_end

    def calc_x_max(self):
        return np.max(self.get_response_data()["x"])

    def crossover(self, other: Individual):
        alpha = random.random()
        child1 = Individual(regulation_data=self.get_regulation_data())
        child1.set_x(alpha * self.get_x() + (1 - alpha) * other.get_x())
        child1.set_v(alpha * self.get_v() + (1 - alpha) * other.get_v())
        child1.set_a(alpha * self.get_a() + (1 - alpha) * other.get_a())
        child1.set_u(alpha * self.get_u() + (1 - alpha) * other.get_u())

        child2 = Individual(regulation_data=other.get_regulation_data())
        child2.set_x((1 - alpha) * self.get_x() + alpha * other.get_x())
        child2.set_v((1 - alpha) * self.get_v() + alpha * other.get_v())
        child2.set_a((1 - alpha) * self.get_a() + alpha * other.get_a())
        child2.set_u((1 - alpha) * self.get_u() + alpha * other.get_u())

        return child1, child2

    def mutate(self, mutation_rate=0.001):
        if random.random() < mutation_rate:
            self.regulation_data = self.generate_new_regulation_data()


def initialize_population(pop_size, generation_id):
    return [
        Individual(individual_id=i, generation_id=generation_id)
        for i in range(pop_size)
    ]


def genetic_thread():
    from commands import execute_to_device_command

    global current_individual

    generation_id = config["data"]["latest-generation"]
    pop_size = config["data"]["population-size"]
    mutation_rate = config["data"]["mutation-rate"]

    population = initialize_population(pop_size, generation_id)

    while True:
        for individual in population:
            current_individual = individual
            if individual.has_received_response():
                continue

            genetics_event.clear()
            execute_to_device_command(
                ToDeviceCommand.COMMAND_SET_REGULATION_DATA, individual
            )
            genetics_event.wait()

        # sort population by fitness
        population = sorted(population, key=lambda ind: ind.fitness())

        # 1. add best individuals to population of new generation
        next_generation = population[: int(0.2 * pop_size)]

        # 2. create remaining individuals by crossing over old individuals
        while len(next_generation) < pop_size:
            parent1, parent2 = random.sample(
                population[int(0.2 * pop_size) : int(0.5 * pop_size)], 2
            )

            child1, child2 = parent1.crossover(parent2)
            next_generation.append(child1)
            if len(next_generation) < pop_size:
                next_generation.append(child2)

        # 3. mutate some individuals
        for individual in next_generation:
            individual.mutate(mutation_rate)

        population = next_generation


current_individual: Individual = Individual()
genetics_event = threading.Event()
genetics_thread_obj = threading.Thread(target=genetic_thread)
