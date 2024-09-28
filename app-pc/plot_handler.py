import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np
import pandas as pd
from genetics import Individual
from config_handler import config
from scipy.signal import hilbert
from scipy.signal import find_peaks
from scipy.interpolate import interp1d

SENSOR_SENSITIVITY = 0.56 / 1000.0  # V/um


def map(x, in_min, in_max, out_min, out_max):
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min


def plot_interactive(individual: Individual = None, force_show: bool = False):
    if not force_show:
        if not config["always-show-plot"]:
            return

    mpl.use("qtagg")  # Interactive plot mode

    if individual is None:
        individual = Individual(
            generation_id=config["data"]["latest-generation"],
            individual_id=config["data"]["latest-individual"] - 1,  # FIXME
        )

    data = individual.response_data
    print(data)
    print(f"Showed data of gen_{individual.generation_id}/{individual.individual_id}")

    if config["scale-plots-to-real-units"]:
        sample_voltage = map(data["x"], 3200.0, 31655.0, -0.2112, -1.6369)
        sample_distance = sample_voltage / SENSOR_SENSITIVITY / 1000 / 1000
        data["x"] = sample_distance

        sample_velocity = np.gradient(data["x"], 0.001)
        data["v"] = sample_velocity  # np.insert(sample_velocity, 0, 0)

        sample_acceleration = np.gradient(data["v"], 0.001)
        data["a"] = sample_acceleration  # np.insert(sample_acceleration, 0, 0)

        sample_voltage = map(data["x_up"], 3200.0, 31655.0, -0.2112, -1.6369)
        sample_distance = sample_voltage / SENSOR_SENSITIVITY / 1000 / 1000
        data["x_up"] = sample_distance

        sample_voltage = map(data["x_down"], 3200.0, 31655.0, -0.2112, -1.6369)
        sample_distance = sample_voltage / SENSOR_SENSITIVITY / 1000 / 1000
        data["x_down"] = sample_distance

        data["x"] = data["x"] * 1000 * 10
        data["x_up"] = data["x_up"] * 1000 * 10
        data["x_down"] = data["x_down"] * 1000 * 10

    axes = data.plot(
        x="t",
        y=["x", "v", "a", "u"],
        subplots=True,
        sharex=True,
        grid=True,
        legend=False,
        xlabel=r"Czas t [s]",
    )

    individual.calc_peaks()
    print(
        f"x_max: {individual.calc_x_max()}, x_thresh: {individual.calc_x_thresh()}, x_at_8: {individual.calc_x_at_8()}, t_reg: {individual.calc_t_reg()}"
    )

    axes[0].vlines(
        individual.calc_t_reg(),
        0,
        1,
        transform=axes[0].get_xaxis_transform(),
        colors="r",
    )

    axes[0].plot(data["t"], data["x_up"], color="g")
    axes[0].plot(data["t"], data["x_down"], color="r")

    # print(f"DeltaX: max(data["x"][:128]) - min(data["x"][:128])

    axes[0].set_ylabel(r"Przemieszczenie x [m]")
    axes[1].set_ylabel(r"Predkość v [m/s]")
    axes[2].set_ylabel(r"Przyśpieszenie a [m/s^2]")
    axes[3].set_ylabel(r"Nastawa elektromagnesów u [-]")
    for i in np.arange(0, 4):
        axes[i].grid(linestyle=":", which="major", color="darkgrey")
        axes[i].grid(linestyle=":", which="minor", color="whitesmoke")
        axes[i].autoscale(enable=True, axis="x", tight=True)

    # if config["always-show-plot"]:
    plt.show()
    # plt.close()

    """
    Uncomment to view FFT of signal
    """
    # figure = plt.figure()
    # X = np.fft.fft(data["a"])
    # N = len(X)
    # n = np.arange(N)
    # T = N / 1000  # 1000 Hz
    # freq = n / T

    # plt.stem(freq, np.abs(X), "b", markerfmt=" ", basefmt="-b")
    # plt.xlabel("Freq (Hz)")
    # plt.ylabel("FFT Amplitude |X(freq)|")
    # plt.show()


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
