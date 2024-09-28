import sys

import pandas as pd
import numpy as np

import matplotlib.pyplot as plt
import matplotlib as mpl
from genetics import Individual
from plot_handler import plot_interactive

if __name__ == "__main__":
    if len(sys.argv) > 1:
        generation_id = sys.argv[1]
        individual_id = sys.argv[2]
    else:
        generation_id = 0
        individual_id = 2

    individual = Individual(generation_id=generation_id, individual_id=individual_id)
    plot_interactive(individual, force_show=True)
