import sys

import pandas as pd
import numpy as np

import matplotlib.pyplot as plt
import matplotlib as mpl

from plot_handler import plot_interactive

if __name__ == "__main__":
    number = sys.argv[1]
    data = pd.read_csv(f"./data/data{number}.csv")
    plot_interactive(data)
