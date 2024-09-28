import sys

import pandas as pd
import numpy as np

import matplotlib.pyplot as plt
import matplotlib as mpl
from genetics import Individual
from config_handler import config

if __name__ == "__main__":
    data = pd.DataFrame(columns=["index", "t_reg", "x_at_8", "f", "x_max"])
    for gen_id, generation in enumerate(config["generations"]):
        for individual in config["generations"][generation]:
            data.loc[len(data)] = {
                "index": 50 * gen_id + int(individual[4:]),
                "t_reg": config["generations"][generation][individual]["t_reg"],
                "x_at_8": config["generations"][generation][individual]["x_at_8"],
                "x_max": config["generations"][generation][individual]["x_max"],
            }

    data["f"] = data["t_reg"] + 10 * data["x_at_8"]
    data = data.set_index("index")
    data.sort_index(inplace=True)
    print(data)

    axes = data.plot(
        kind="line",
        subplots=True,
        sharex=False,
        xlabel="Numer osobnika [-]",
    )
    axes[0].set_ylabel(r"Czas regulacji t_reg [s]")
    axes[1].set_ylabel(r"Różnica między obwiedniami na końcu wykresu")
    axes[2].set_ylabel(r"F przetrwania")
    axes[3].set_ylabel(r"Maksymalne przemieszczenie")

    plt.show()
