import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np
import pandas as pd


def plot_interactive(data: pd.DataFrame):
    mpl.use("qtagg")  # Interactive plot mode

    axes = data.plot(
        x="t",
        y=["x", "v", "a", "u"],
        subplots=True,
        sharex=True,
        grid=True,
        legend=False,
        xlabel=r"Czas t [s]",
    )

    axes[0].set_ylabel(r"Przemieszczenie x [m]")
    axes[1].set_ylabel(r"Przyśpieszenie v [m/s]")
    axes[2].set_ylabel(r"Predkość a [m/s^2]")
    axes[3].set_ylabel(r"Nastawa elektromagnesów u [-]")
    for i in np.arange(0, 4):
        axes[i].grid(linestyle=":", which="major", color="darkgrey")
        axes[i].grid(linestyle=":", which="minor", color="whitesmoke")
        axes[i].autoscale(enable=True, axis="x", tight=True)
    plt.show()


def plot_to_file(data: pd.DataFrame):
    mpl.use("pgf")  # Save plots to file
    plt.rcParams.update(
        {
            "font.family": "serif",
            "text.usetex": True,
            "pgf.rcfonts": False,
            "pgf.texsystem": "lualatex",
            "pgf.preamble": "\n".join(
                [
                    r"\usepackage{amsmath}",
                    r"\usepackage[utf8]{inputenc}",
                    r"\usepackage[T1]{fontenc}",
                    r"\usepackage{polski}",
                    r"\usepackage{siunitx}",
                ]
            ),
        }
    )
    plt.style.use(["science", "ieee"])

    data.plot(x="t", y=["x"])
    plt.xlabel(r"Czas $t$ [ms]")
    plt.ylabel(r"Przemieszczenie $x$ [\unit{\micro\meter}]")
    plt.grid(linestyle=":", which="major", color="darkgrey")
    plt.grid(linestyle=":", which="minor", color="whitesmoke")
    plt.savefig("results/plot.png", dpi=300)
    plt.close()
