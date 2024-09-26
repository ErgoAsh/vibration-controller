from __future__ import annotations
import os
import numpy as np
import random
import yaml
import pandas as pd
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
        id=0,
    ):
        self.id = id
        if response_data is not None:
            self.response_data = response_data.copy()
        if regulation_data is not None:
            self.regulation_data = regulation_data.copy()
            return

        self.regulation_data = self.generate_new_regulation_data(x, v, a, u)

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

    def get_id(self):
        return self.id

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
        ts = self.calc_ts(self.regulation_data)
        x8 = self.calc_x8(self.regulation_data)
        return ts + 10 * x8

    def calc_peaks(self, regulation_data):
        peaks, _ = find_peaks(
            regulation_data["x"], prominence=100
        )  # TODO adjust prominence value
        min_peaks, _ = find_peaks(-regulation_data["x"], prominence=100)

        # Find all positive peaks
        interp_upper = interp1d(
            regulation_data["t"].iloc[peaks],
            regulation_data["x"].iloc[peaks],
            kind="linear",
            fill_value="extrapolate",
        )

        # Find all negitive peaks
        interp_lower = interp1d(
            regulation_data["t"].iloc[min_peaks],
            regulation_data["x"].iloc[min_peaks],
            kind="linear",
            fill_value="extrapolate",
        )

        regulation_data["x_up"] = interp_upper(regulation_data["t"])
        regulation_data["x_down"] = interp_lower(regulation_data["t"])
        return regulation_data

    def calc_t_reg(self, response_data):
        x_threshold = 0.15 * self.calc_x_max(response_data)
        t_calibration = response_data["t"].iloc[
            np.argmax(response_data["x_up"] < x_threshold)
        ]
        return t_calibration

    def calc_x_at_8(self, response_data):
        x_end = response_data["x_up"].iloc[-1] - response_data["x_down"].iloc[-1]
        return x_end

    def calc_x_max(self, response_data):
        return np.max(response_data["x"])

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


def initialize_population(pop_size):
    return [Individual() for _ in range(pop_size)]


def save_generation_results(generation, population):
    folder_name = f"generations/gen_{generation}"
    if not os.path.exists(folder_name):
        os.makedirs(folder_name)

    regulation_data = []
    for individual in population:
        regulation_data.append(
            {
                "number": individual.get_id(),
                "t_reg": individual.calc_t_reg(),
                "x_at_8": individual.calc_x_at_8(),
                "x_max": individual.calc_x_max(),
            }
        )

    # Zapis do pliku w formacie YAML
    with open(os.path.join(folder_name, "results.yaml"), "w") as f:
        yaml.dump(regulation_data, f)


def genetic_algorithm(pop_size=50, generations=2137, mutation_rate=0.001):
    population = initialize_population(pop_size)

    for generation in range(generations):
        # Wait here for all individuals

        population = sorted(population, key=lambda ind: ind.fitness())
        save_generation_results(generation, population)
        next_generation = population[: int(0.2 * pop_size)]

        while len(next_generation) < pop_size:
            parent1, parent2 = random.sample(population[: int(0.5 * pop_size)], 2)
            child1, child2 = parent1.crossover(parent2)
            next_generation.append(child1)
            if len(next_generation) < pop_size:
                next_generation.append(child2)

        for individual in next_generation:
            individual.mutate(mutation_rate)

        population = next_generation
