import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np
import pandas as pd
from scipy.signal import hilbert
from scipy.signal import find_peaks
from scipy.interpolate import interp1d


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

    # analytic_signal = np.array(hilbert(data["x"]))
    # data["x_up"] = np.abs(analytic_signal)  # Górna obwiednia
    # data["x_down"] = -data["x_up"]  # Dolna obwiednia
    # axes[0].plot(data["t"], data["x_up"], color="m")
    # axes[0].plot(data["t"], data["x_down"], color="m")

    peaks, _ = find_peaks(data["x"], prominence=100)
    min_peaks, _ = find_peaks(-data["x"], prominence=100)

    interp_upper = interp1d(
        data["t"].iloc[peaks],
        data["x"].iloc[peaks],
        kind="linear",
        fill_value="extrapolate",
    )
    interp_lower = interp1d(
        data["t"].iloc[min_peaks],
        data["x"].iloc[min_peaks],
        kind="linear",
        fill_value="extrapolate",
    )

    data["x_up"] = interp_upper(data["t"])
    data["x_down"] = interp_lower(data["t"])

    x_threshold = 0.15 * np.max(data["x"])
    x_end = data["x_up"].iloc[-1] - data["x_down"].iloc[-1]
    t_calibration = data["t"].iloc[np.argmax(data["x_up"] < x_threshold)]
    print(f"x_end: {x_end}, x_threshold: {x_threshold}, t_calibration: {t_calibration}")
    axes[0].vlines(
        t_calibration, 0, 1, transform=axes[0].get_xaxis_transform(), colors="r"
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
    plt.show()

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
