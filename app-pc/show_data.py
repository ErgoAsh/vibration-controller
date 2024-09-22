import sys

import pandas as pd
import numpy as np

import matplotlib.pyplot as plt
import matplotlib as mpl

if __name__ == "__main__":
    number = sys.argv[1]
    data = pd.read_csv(f"./data/data{number}.csv")

    data.plot(x="t", y=["x", "v", "a", "u"], subplots=True)
    plt.xlabel(r"Czas t [ms]")
    plt.ylabel(r"Przemieszczenie x [um]")
    plt.grid(linestyle=":", which="major", color="darkgrey")
    plt.grid(linestyle=":", which="minor", color="whitesmoke")
    plt.show()
