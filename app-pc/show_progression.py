import sys

import pandas as pd
import numpy as np

import matplotlib.pyplot as plt
import matplotlib as mpl
from genetics import Individual
from config_handler import config

if __name__ == "__main__":
    data = pd.DataFrame(columns=["index", "t_reg", "x_at_8", "x_max"])
    counter = 0
    for generation in config["generations"]:
        for individual in config["generations"][generation]:
            counter = counter + 1
            data = pd.concat(
                [
                    data,
                    pd.DataFrame.from_records(
                        [
                            {
                                "index": counter,
                                "t_reg": config["generations"][generation][individual][
                                    "t_reg"
                                ],
                                "x_at_8": config["generations"][generation][individual][
                                    "x_at_8"
                                ],
                                "x_max": config["generations"][generation][individual][
                                    "x_max"
                                ],
                            }
                        ]
                    ),
                ]
            )

    data = data.set_index("index")
    axes = data.plot(subplots=True, xlabel="Numer osobnika [-]")
    axes[0].set_ylabel(r"Czas regulacji t_reg [s]")
    axes[1].set_ylabel(r"Różnica między obwiedniami na końcu wykresu")
    axes[2].set_ylabel(r"Maksymalne przemieszczenie")

    plt.show()
