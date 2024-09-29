import sys

import pandas as pd
import numpy as np

import matplotlib.pyplot as plt
import matplotlib as mpl
from genetics import Individual
from config_handler import config

if __name__ == "__main__":
    data = pd.DataFrame(
        columns=["index", "gen_id", "ind_id", "t_reg", "x_at_8", "f", "x_max"]
    )
    for generation in enumerate(config["generations"]):
        gen_id = int(generation[1][4:])
        for individual in config["generations"][f"gen_{gen_id}"]:
            ind_id = int(individual[4:])
            data.loc[len(data)] = {
                "index": 50 * gen_id + ind_id,
                "gen_id": gen_id,
                "ind_id": ind_id,
                "t_reg": config["generations"][f"gen_{gen_id}"][f"ind_{ind_id}"][
                    "t_reg"
                ],
                "x_at_8": config["generations"][f"gen_{gen_id}"][f"ind_{ind_id}"][
                    "x_at_8"
                ],
                "x_max": config["generations"][f"gen_{gen_id}"][f"ind_{ind_id}"][
                    "x_max"
                ],
            }

    data["f"] = data["t_reg"] + 10 * data["x_at_8"]
    data = data.set_index("index")
    data.sort_index(inplace=True)
    print(data)

    axes = data.plot(
        y=["t_reg", "x_at_8", "x_max", "f"],
        kind="line",
        subplots=True,
        sharex=False,
        xlabel="Numer osobnika [-]",
        legend=False,
    )
    axes[0].set_ylabel(r"Czas regulacji t_reg [s]")
    axes[1].set_ylabel(r"Różnica między obwiedniami dla t=8,192 s")
    axes[2].set_ylabel(r"Maksymalne przemieszczenie")
    axes[3].set_ylabel(r"F przetrwania")

    t_data = data[data["t_reg"] < 8.192].groupby("gen_id")
    gen_data = pd.DataFrame(
        {
            "count": t_data["t_reg"].count(),
            "mean": t_data["t_reg"].mean(),
            "x8_mean": t_data["x_at_8"].mean(),
            "x_max_mean": t_data["x_max"].mean(),
        }
    )
    print(gen_data)

    axes = gen_data.plot(
        subplots=True,
        xlabel="Numer generacji [-]",
        marker=".",
        legend=False,
    )
    axes[0].set_ylabel(r"Liczba ustabilizowanych osobników [-]")
    axes[1].set_ylabel(r"Średni czas stabilizacji [s]")
    axes[2].set_ylabel(r"Średnia wartość x przy t=8,192 [-]")
    axes[3].set_ylabel(r"Średnia wartość maksymalna x [-]")
    plt.show()
