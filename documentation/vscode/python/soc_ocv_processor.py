#!/usr/bin/env python3
"""
SOC–OCV curve processing tool configured directly in this file.

Edit only the USER CONFIGURATION section below, then run:

    python soc_ocv_processor_config.py

Required packages:

    pip install numpy pandas scipy scikit-learn matplotlib
"""

from __future__ import annotations

import sys
from pathlib import Path
from typing import Optional, Tuple

import numpy as np
import pandas as pd
from scipy.interpolate import PchipInterpolator
from scipy.signal import savgol_filter
from sklearn.isotonic import IsotonicRegression


# =============================================================================
# USER CONFIGURATION
# =============================================================================

# Input and output files.
INPUT_CSV = Path("./data/molicelP50B/P50B_plotA_charge_discharge_0p5pct.csv")
OUTPUT_CSV = Path("./data/molicelP50B/processed_soc_ocv.csv")

# CSV delimiter:
#   None  = automatically detect comma, semicolon, tab, etc.
#   ","   = comma-separated
#   ";"   = semicolon-separated
#   "\t"  = tab-separated
CSV_DELIMITER: Optional[str] = None

# CSV column layout.
#
# Set SHARED_SOC_COLUMN to True when charge and discharge OCV values use the
# same SOC column. Each OCV curve is paired independently with the valid SOC
# values from that shared column, so NaNs in one OCV column do not remove valid
# samples from the other curve.
SHARED_SOC_COLUMN = True

# Used when SHARED_SOC_COLUMN = True.
SOC_COLUMN = "SOC_plot_percent"

# Used when SHARED_SOC_COLUMN = False.
CHARGE_SOC_COLUMN = "SOC_plot_percent"
DISCHARGE_SOC_COLUMN = "SOC_plot_percent"

# OCV columns are always configured separately.
CHARGE_OCV_COLUMN = "P50B_charge_voltage_V"
DISCHARGE_OCV_COLUMN = "P50B_discharge_voltage_V"

# SOC units:
#   "auto"     = detect 0–1 versus 0–100 automatically
#   "fraction" = SOC is already between 0 and 1
#   "percent"  = SOC is between 0 and 100
#
# SHARED_SOC_UNITS is used when SHARED_SOC_COLUMN = True.
SHARED_SOC_UNITS = "auto"
CHARGE_SOC_UNITS = "auto"
DISCHARGE_SOC_UNITS = "auto"

# Reverse the original row order before processing.
# Usually this is not necessary because the script sorts by SOC.
CHARGE_REVERSE_ROWS = False
DISCHARGE_REVERSE_ROWS = False

# Reverse the SOC axis:
#   False: keep SOC values as supplied
#   True:  convert descending-style SOC values using min + max - SOC
#
# Example:
#   100, 90, ..., 0 becomes 0, 10, ..., 100 before normalization.
CHARGE_REVERSE_SOC_AXIS = False
DISCHARGE_REVERSE_SOC_AXIS = True

# Reverse the OCV values vertically using min + max - OCV.
# This is rarely needed and should normally remain False.
CHARGE_REVERSE_OCV_VALUES = False
DISCHARGE_REVERSE_OCV_VALUES = False

# Enforce a physically monotonic OCV-versus-SOC relationship before PCHIP.
# This removes small local measurement reversals.
ENFORCE_MONOTONIC_OCV = True

# Extra smoothing after interpolation.
ENABLE_EXTRA_SMOOTHING = True

# General smoothing over the complete SOC range.
# Window values must be odd. Larger values are smoother.
BASE_SMOOTHING_WINDOW = 8
BASE_SMOOTHING_POLYORDER = 8

# Stronger smoothing around 50% SOC.
ENABLE_STRONG_MID_SOC_SMOOTHING = True
MID_SOC_SMOOTHING_CENTER = 0.50
MID_SOC_SMOOTHING_WIDTH = 0.05
MID_SOC_STRONG_WINDOW = 101
MID_SOC_STRONG_POLYORDER = 3
MID_SOC_BLEND_STRENGTH = 1.0

# Keep the smoothed OCV curves physically monotonic.
ENFORCE_MONOTONIC_AFTER_SMOOTHING = True

# Number of samples in the generated common SOC grid.
NUMBER_OF_OUTPUT_POINTS = 101

# Plot options.
SHOW_PLOT = True
SAVE_PLOT = True
PLOT_FILE = Path("./data/molicelP50B/data/molicelP50B/soc_ocv_curves.png")
PLOT_DPI = 200

# Numerical CSV format.
OUTPUT_FLOAT_FORMAT = "%.10g"


# =============================================================================
# PROCESSING FUNCTIONS
# =============================================================================

VALID_SOC_UNITS = {"auto", "fraction", "percent"}


def read_csv(path: Path, delimiter: Optional[str]) -> pd.DataFrame:
    if not path.is_file():
        raise FileNotFoundError(f"Input CSV does not exist: {path}")

    try:
        if delimiter is None:
            return pd.read_csv(path, sep=None, engine="python")
        return pd.read_csv(path, sep=delimiter)
    except Exception as exc:
        raise RuntimeError(f"Could not read CSV '{path}': {exc}") from exc


def validate_configuration(df: pd.DataFrame) -> None:
    selected_columns = {
        "CHARGE_OCV_COLUMN": CHARGE_OCV_COLUMN,
        "DISCHARGE_OCV_COLUMN": DISCHARGE_OCV_COLUMN,
    }

    if SHARED_SOC_COLUMN:
        selected_columns["SOC_COLUMN"] = SOC_COLUMN
    else:
        selected_columns["CHARGE_SOC_COLUMN"] = CHARGE_SOC_COLUMN
        selected_columns["DISCHARGE_SOC_COLUMN"] = DISCHARGE_SOC_COLUMN

    missing = [
        f"{setting}={column!r}"
        for setting, column in selected_columns.items()
        if column not in df.columns
    ]
    if missing:
        available = ", ".join(repr(str(column)) for column in df.columns)
        raise KeyError(
            "Configured column names were not found:\n  "
            + "\n  ".join(missing)
            + f"\nAvailable columns: {available}"
        )

    soc_unit_settings = (
        {"SHARED_SOC_UNITS": SHARED_SOC_UNITS}
        if SHARED_SOC_COLUMN
        else {
            "CHARGE_SOC_UNITS": CHARGE_SOC_UNITS,
            "DISCHARGE_SOC_UNITS": DISCHARGE_SOC_UNITS,
        }
    )

    for setting, value in soc_unit_settings.items():
        if value not in VALID_SOC_UNITS:
            raise ValueError(
                f"{setting} must be one of {sorted(VALID_SOC_UNITS)}, got {value!r}."
            )

    if NUMBER_OF_OUTPUT_POINTS < 2:
        raise ValueError("NUMBER_OF_OUTPUT_POINTS must be at least 2.")


def numeric_series(df: pd.DataFrame, column: str) -> np.ndarray:
    return pd.to_numeric(df[column], errors="coerce").to_numpy(dtype=float)


def soc_scale(values: np.ndarray, units: str) -> float:
    if units == "fraction":
        return 1.0
    if units == "percent":
        return 0.01

    finite = values[np.isfinite(values)]
    if finite.size == 0:
        raise ValueError("SOC column has no numeric values.")

    return 0.01 if np.nanmax(np.abs(finite)) > 1.5 else 1.0


def prepare_curve(
    soc_raw: np.ndarray,
    ocv_raw: np.ndarray,
    *,
    units: str,
    reverse_rows: bool,
    reverse_soc_axis: bool,
    reverse_ocv_values: bool,
    enforce_monotonic: bool,
    curve_name: str,
) -> Tuple[np.ndarray, np.ndarray]:
    if reverse_rows:
        soc_raw = soc_raw[::-1]
        ocv_raw = ocv_raw[::-1]

    valid = np.isfinite(soc_raw) & np.isfinite(ocv_raw)
    soc = soc_raw[valid].astype(float)
    ocv = ocv_raw[valid].astype(float)

    if soc.size < 2:
        raise ValueError(f"{curve_name}: fewer than two valid SOC/OCV pairs.")

    if reverse_soc_axis:
        soc = np.nanmin(soc) + np.nanmax(soc) - soc

    soc *= soc_scale(soc, units)

    if reverse_ocv_values:
        ocv = np.nanmin(ocv) + np.nanmax(ocv) - ocv

    # Sort by SOC and average duplicate SOC measurements.
    curve = pd.DataFrame({"soc": soc, "ocv": ocv})
    curve = curve.groupby("soc", as_index=False, sort=True)["ocv"].mean()
    curve = curve.sort_values("soc")

    soc = curve["soc"].to_numpy(dtype=float)
    ocv = curve["ocv"].to_numpy(dtype=float)

    if soc.size < 2:
        raise ValueError(f"{curve_name}: fewer than two unique SOC values.")

    keep = np.r_[True, np.diff(soc) > 0]
    soc = soc[keep]
    ocv = ocv[keep]

    if enforce_monotonic:
        model = IsotonicRegression(increasing=True, out_of_bounds="clip")
        ocv = model.fit_transform(soc, ocv)

    return soc, ocv


def common_soc_grid(
    charge_soc: np.ndarray,
    discharge_soc: np.ndarray,
    points: int,
) -> np.ndarray:
    lower = max(float(np.min(charge_soc)), float(np.min(discharge_soc)))
    upper = min(float(np.max(charge_soc)), float(np.max(discharge_soc)))

    if upper <= lower:
        raise ValueError(
            "Charge and discharge curves do not share an overlapping SOC range."
        )

    return np.linspace(lower, upper, points)


def inverse_soc_at_ocv(
    soc: np.ndarray,
    ocv: np.ndarray,
    voltage_grid: np.ndarray,
) -> np.ndarray:
    inverse = pd.DataFrame({"ocv": ocv, "soc": soc})
    inverse = inverse.groupby("ocv", as_index=False, sort=True)["soc"].mean()

    voltage = inverse["ocv"].to_numpy(dtype=float)
    inverse_soc = inverse["soc"].to_numpy(dtype=float)

    if voltage.size < 2:
        return np.full_like(voltage_grid, np.nan, dtype=float)

    return PchipInterpolator(
        voltage,
        inverse_soc,
        extrapolate=False,
    )(voltage_grid)



def valid_savgol_window(requested: int, length: int, polyorder: int) -> int:
    """Return a valid odd Savitzky-Golay window."""
    window = int(requested)
    if window % 2 == 0:
        window += 1

    maximum = length if length % 2 == 1 else length - 1
    minimum = polyorder + 1
    if minimum % 2 == 0:
        minimum += 1

    window = min(max(window, minimum), maximum)
    if window <= polyorder:
        raise ValueError("Smoothing window must be larger than polynomial order.")
    return window


def smooth_ocv_curve(soc_grid: np.ndarray, voltage: np.ndarray) -> np.ndarray:
    """Apply global smoothing plus stronger local smoothing near 50% SOC."""
    if not ENABLE_EXTRA_SMOOTHING:
        return voltage.copy()

    base_window = valid_savgol_window(
        BASE_SMOOTHING_WINDOW, len(voltage), BASE_SMOOTHING_POLYORDER
    )
    base = savgol_filter(
        voltage,
        window_length=base_window,
        polyorder=BASE_SMOOTHING_POLYORDER,
        mode="interp",
    )

    smoothed = base

    if ENABLE_STRONG_MID_SOC_SMOOTHING:
        strong_window = valid_savgol_window(
            MID_SOC_STRONG_WINDOW, len(voltage), MID_SOC_STRONG_POLYORDER
        )
        strong = savgol_filter(
            voltage,
            window_length=strong_window,
            polyorder=MID_SOC_STRONG_POLYORDER,
            mode="interp",
        )

        if MID_SOC_SMOOTHING_WIDTH <= 0:
            raise ValueError("MID_SOC_SMOOTHING_WIDTH must be greater than zero.")

        strength = float(np.clip(MID_SOC_BLEND_STRENGTH, 0.0, 1.0))
        weight = strength * np.exp(
            -0.5
            * (
                (soc_grid - MID_SOC_SMOOTHING_CENTER)
                / MID_SOC_SMOOTHING_WIDTH
            )
            ** 2
        )
        smoothed = (1.0 - weight) * base + weight * strong

    if ENFORCE_MONOTONIC_AFTER_SMOOTHING:
        model = IsotonicRegression(increasing=True, out_of_bounds="clip")
        smoothed = model.fit_transform(soc_grid, smoothed)

    return smoothed


def process_curves(
    charge_soc: np.ndarray,
    charge_ocv: np.ndarray,
    discharge_soc: np.ndarray,
    discharge_ocv: np.ndarray,
    points: int,
) -> pd.DataFrame:
    grid = common_soc_grid(charge_soc, discharge_soc, points)

    charge_v_raw = PchipInterpolator(
        charge_soc,
        charge_ocv,
        extrapolate=False,
    )(grid)

    discharge_v_raw = PchipInterpolator(
        discharge_soc,
        discharge_ocv,
        extrapolate=False,
    )(grid)

    charge_v = smooth_ocv_curve(grid, charge_v_raw)
    discharge_v = smooth_ocv_curve(grid, discharge_v_raw)

    average_v = 0.5 * (charge_v + discharge_v)
    voltage_hysteresis = charge_v - discharge_v

    result = pd.DataFrame(
        {
            "soc_fraction": grid,
            "soc_percent": 100.0 * grid,
            "ocv_charge_raw_v": charge_v_raw,
            "ocv_discharge_raw_v": discharge_v_raw,
            "ocv_charge_v": charge_v,
            "ocv_discharge_v": discharge_v,
            "ocv_average_v": average_v,
            "voltage_hysteresis_v": voltage_hysteresis,
            "half_voltage_hysteresis_v": 0.5 * voltage_hysteresis,
        }
    )

    # SOC hysteresis at a common OCV:
    # SOC_charge(V) - SOC_discharge(V)
    voltage_lower = max(float(np.nanmin(charge_v)), float(np.nanmin(discharge_v)))
    voltage_upper = min(float(np.nanmax(charge_v)), float(np.nanmax(discharge_v)))

    if voltage_upper > voltage_lower:
        voltage_grid = np.linspace(voltage_lower, voltage_upper, points)

        charge_soc_at_v = inverse_soc_at_ocv(grid, charge_v, voltage_grid)
        discharge_soc_at_v = inverse_soc_at_ocv(grid, discharge_v, voltage_grid)

        result["common_ocv_v"] = voltage_grid
        result["charge_soc_at_ocv_fraction"] = charge_soc_at_v
        result["discharge_soc_at_ocv_fraction"] = discharge_soc_at_v
        result["soc_hysteresis_fraction"] = (
            charge_soc_at_v - discharge_soc_at_v
        )
        result["soc_hysteresis_percent"] = (
            100.0 * result["soc_hysteresis_fraction"]
        )

    return result


def make_plot(result: pd.DataFrame) -> None:
    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(9, 6))
    ax.plot(result["soc_percent"], result["ocv_charge_v"], label="Charge OCV")
    ax.plot(
        result["soc_percent"],
        result["ocv_discharge_v"],
        label="Discharge OCV",
    )
    ax.plot(
        result["soc_percent"],
        result["ocv_average_v"],
        label="Average OCV",
        linewidth=2,
    )

    ax.set_xlabel("SOC (%)")
    ax.set_ylabel("OCV (V)")
    ax.set_title("SOC–OCV Curves")
    ax.grid(True, alpha=0.3)
    ax.legend()
    fig.tight_layout()

    if SAVE_PLOT:
        PLOT_FILE.parent.mkdir(parents=True, exist_ok=True)
        fig.savefig(PLOT_FILE, dpi=PLOT_DPI)
        print(f"Saved plot: {PLOT_FILE.resolve()}")

    if SHOW_PLOT:
        plt.show()
    else:
        plt.close(fig)


def main() -> int:
    try:
        df = read_csv(INPUT_CSV, CSV_DELIMITER)

        if df.empty:
            raise ValueError("The input CSV file is empty.")

        validate_configuration(df)

        if SHARED_SOC_COLUMN:
            shared_soc = numeric_series(df, SOC_COLUMN)
            charge_soc_raw = shared_soc.copy()
            discharge_soc_raw = shared_soc.copy()
            charge_units = SHARED_SOC_UNITS
            discharge_units = SHARED_SOC_UNITS
        else:
            charge_soc_raw = numeric_series(df, CHARGE_SOC_COLUMN)
            discharge_soc_raw = numeric_series(df, DISCHARGE_SOC_COLUMN)
            charge_units = CHARGE_SOC_UNITS
            discharge_units = DISCHARGE_SOC_UNITS

        # The two masks are applied independently inside prepare_curve().
        # Therefore, a missing charge OCV sample does not discard the
        # corresponding discharge sample, and vice versa.
        charge_soc, charge_ocv = prepare_curve(
            charge_soc_raw,
            numeric_series(df, CHARGE_OCV_COLUMN),
            units=charge_units,
            reverse_rows=CHARGE_REVERSE_ROWS,
            reverse_soc_axis=CHARGE_REVERSE_SOC_AXIS,
            reverse_ocv_values=CHARGE_REVERSE_OCV_VALUES,
            enforce_monotonic=ENFORCE_MONOTONIC_OCV,
            curve_name="Charge",
        )

        discharge_soc, discharge_ocv = prepare_curve(
            discharge_soc_raw,
            numeric_series(df, DISCHARGE_OCV_COLUMN),
            units=discharge_units,
            reverse_rows=DISCHARGE_REVERSE_ROWS,
            reverse_soc_axis=DISCHARGE_REVERSE_SOC_AXIS,
            reverse_ocv_values=DISCHARGE_REVERSE_OCV_VALUES,
            enforce_monotonic=ENFORCE_MONOTONIC_OCV,
            curve_name="Discharge",
        )

        result = process_curves(
            charge_soc,
            charge_ocv,
            discharge_soc,
            discharge_ocv,
            NUMBER_OF_OUTPUT_POINTS,
        )

        OUTPUT_CSV.parent.mkdir(parents=True, exist_ok=True)
        result.to_csv(
            OUTPUT_CSV,
            index=False,
            float_format=OUTPUT_FLOAT_FORMAT,
        )

        print(f"Input file: {INPUT_CSV.resolve()}")
        print(f"Output file: {OUTPUT_CSV.resolve()}")
        print(f"Rows written: {len(result)}")
        print(
            "SOC overlap: "
            f"{result['soc_percent'].min():.3f}% to "
            f"{result['soc_percent'].max():.3f}%"
        )

        if SHOW_PLOT or SAVE_PLOT:
            make_plot(result)

        return 0

    except Exception as exc:
        print(f"Processing failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())