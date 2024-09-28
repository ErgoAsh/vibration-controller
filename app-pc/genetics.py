from __future__ import annotations
import os
import numpy as np
import random
import threading
import pandas as pd
from time import sleep
from enums import ToDeviceCommand
from config_handler import config, save_config
from scipy.signal import find_peaks
from scipy.interpolate import interp1d


class Individual:
    _regulation_data: pd.DataFrame = pd.DataFrame()
    _response_data: pd.DataFrame = pd.DataFrame()
    _has_received_response: bool = False

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
            self.has_received_response = True
        else:
            self.load_response()

        if regulation_data is not None:
            self.regulation_data = regulation_data.copy()
        else:
            self.load_regulation()

    @property
    def regulation_data(self):
        return self._regulation_data

    @regulation_data.setter
    def regulation_data(self, value):
        self._regulation_data = value

    @property
    def response_data(self):
        return self._response_data

    @response_data.setter
    def response_data(self, value):
        self._response_data = value

    @property
    def has_received_response(self):
        return self._has_received_response

    @has_received_response.setter
    def has_received_response(self, value):
        self._has_received_response = value

    def load_response(self):
        try:
            self.response_data = pd.read_csv(
                f"./generations/gen_{self.generation_id}/ind_{self.individual_id}_response.csv"
            )
            self.has_received_response = True
        except FileNotFoundError:
            pass

    def load_regulation(self):
        try:
            self.regulation_data = pd.read_csv(
                f"./generations/gen_{self.generation_id}/ind_{self.individual_id}_regulation.csv"
            )
        except FileNotFoundError:
            if self.generation_id == 0:
                self.regulation_data = self.generate_new_regulation_data()
                return
            pass

    def save_regulation(self):
        if self.regulation_data is not None:
            if not os.path.exists(f"./generations/gen_{self.generation_id}/"):
                os.makedirs(f"./generations/gen_{self.generation_id}/")

            self.regulation_data.to_csv(
                f"./generations/gen_{self.generation_id}/ind_{self.individual_id}_regulation.csv"
            )

    def save_response(self):
        if self.response_data is not None:
            if not os.path.exists(f"./generations/gen_{self.generation_id}/"):
                os.makedirs(f"./generations/gen_{self.generation_id}/")

            if "generations" not in config:
                config["generations"] = dict()

            if f"gen_{self.generation_id}" not in config["generations"]:
                config["generations"][f"gen_{self.generation_id}"] = dict()

            if (
                f"ind_{self.individual_id}"
                not in config["generations"][f"gen_{self.generation_id}"]
            ):
                config["generations"][f"gen_{self.generation_id}"][
                    f"ind_{self.individual_id}"
                ] = dict()

            config["generations"][f"gen_{self.generation_id}"][
                f"ind_{self.individual_id}"
            ]["t_reg"] = self.calc_t_reg()

            config["generations"][f"gen_{self.generation_id}"][
                f"ind_{self.individual_id}"
            ]["x_at_8"] = self.calc_x_at_8()

            config["generations"][f"gen_{self.generation_id}"][
                f"ind_{self.individual_id}"
            ]["x_thresh"] = self.calc_x_thresh()

            config["generations"][f"gen_{self.generation_id}"][
                f"ind_{self.individual_id}"
            ]["x_max"] = self.calc_x_max()

            self.response_data.to_csv(
                f"./generations/gen_{self.generation_id}/ind_{self.individual_id}_response.csv"
            )

            save_config(new_config=config)
            self.has_received_response = True

    def generate_new_regulation_data(self, x=None, v=None, a=None, u=None):
        regulation_data = pd.DataFrame()
        regulation_data["x"] = (
            x if x is not None else np.random.uniform(-4000, 4000, 4096)
        )
        regulation_data["v"] = (
            v if v is not None else np.random.uniform(-4000, 4000, 4096)
        )
        regulation_data["a"] = (
            a if a is not None else np.random.uniform(-4000, 4000, 4096)
        )
        regulation_data["u"] = (
            u if u is not None else np.random.uniform(-1000, 1000, 4096)
        )
        return regulation_data

    def get_id(self):
        return self.individual_id

    def get_x(self):
        return self.regulation_data["x"]

    def get_v(self):
        return self.regulation_data["v"]

    def get_a(self):
        return self.regulation_data["a"]

    def get_u(self):
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
        peaks, _ = find_peaks(self.response_data["x"], prominence=100, distance=50)
        min_peaks, _ = find_peaks(-self.response_data["x"], prominence=100, distance=50)

        # Find all positive peaks
        interp_upper = interp1d(
            self.response_data["t"].iloc[peaks],
            self.response_data["x"].iloc[peaks],
            kind="linear",
            fill_value="extrapolate",
        )

        # Find all negitive peaks
        interp_lower = interp1d(
            self.response_data["t"].iloc[min_peaks],
            self.response_data["x"].iloc[min_peaks],
            kind="linear",
            fill_value="extrapolate",
        )

        self.response_data["x_up"] = interp_upper(self.response_data["t"])
        self.response_data["x_down"] = interp_lower(self.response_data["t"])

    def calc_t_reg(self):
        if "x_up" not in self.response_data.columns:
            self.calc_peaks()

        x_threshold = self.calc_x_thresh()
        t_calibration: np.float32 = self.response_data["t"].iloc[
            np.argmax(
                (self.response_data["x_up"] - self.response_data["x_down"])
                < x_threshold
            )
        ]

        result = t_calibration.item()
        if result <= 0:
            result = 8192 * 0.001

        return result

    def calc_x_thresh(self):
        return 0.15 * self.calc_x_max()

    def calc_x_at_8(self):
        if "x_up" not in self.response_data.columns:
            self.calc_peaks()

        # Could be -1 but using the very last point might be risky
        x_end: np.float32 = (
            self.response_data["x_up"].iloc[-25]
            - self.response_data["x_down"].iloc[-25]
        )
        return x_end.item()

    def calc_x_max(self):
        return np.max(np.abs(self.response_data["x"])).item()

    def crossover(self, other: Individual):
        alpha = random.random()
        child1 = Individual(regulation_data=self.regulation_data)
        child1.set_x(alpha * self.get_x() + (1 - alpha) * other.get_x())
        child1.set_v(alpha * self.get_v() + (1 - alpha) * other.get_v())
        child1.set_a(alpha * self.get_a() + (1 - alpha) * other.get_a())
        child1.set_u(alpha * self.get_u() + (1 - alpha) * other.get_u())

        child2 = Individual(regulation_data=other.regulation_data)
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
    # save regulation when generated
    for individual in population:
        individual.save_regulation()

    while True:
        for i, individual in enumerate(population):
            current_individual = individual
            config["data"]["latest-individual"] = i

            if individual.has_received_response:
                continue

            print(
                f"Genetic algorithm is now using gen_{generation_id} indiv_{current_individual.individual_id}"
            )

            genetics_event.clear()
            # send regulation data
            execute_to_device_command(
                ToDeviceCommand.COMMAND_SET_REGULATION_DATA, individual
            )

            sleep(1.5)

            # get respons data
            execute_to_device_command(ToDeviceCommand.COMMAND_MOVE_TO_START)
            genetics_event.wait()

        print(f"Responses from gen_{generation_id} has been received")

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

        generation_id = generation_id + 1
        config["data"]["latest-individual"] = 0
        config["data"]["latest-generation"] = generation_id
        save_config()

        # update indices
        for i in np.arange(0, pop_size):
            next_generation[i].generation_id = generation_id
            next_generation[i].individual_id = i
            next_generation[i].has_received_response = False
            next_generation[i].save_regulation()

        population = next_generation


current_individual: Individual = Individual()
genetics_event = threading.Event()
genetics_thread_obj = threading.Thread(target=genetic_thread)
