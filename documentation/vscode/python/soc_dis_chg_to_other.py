from pathlib import Path
from typing import Union

import numpy as np
import pandas as pd


# ============================================================
# USER CONFIGURATION
# ============================================================

INPUT_CSV = Path("./data/molicelP50B/P50B_plotA_charge_discharge_0p5pct.csv")
OUTPUT_CSV = Path("./data/molicelP50B/battery_curves_processed.csv")

# True when the CSV has column names in its first row.
CSV_HAS_HEADER = True

# Columns can be selected by position:
SOC_COL = 0
DISCHARGE_OCV_COL = 1
CHARGE_OCV_COL = 2

# Or, when CSV_HAS_HEADER = True, by name:
# SOC_COL = "SOC"
# DISCHARGE_OCV_COL = "Discharge Voltage"
# CHARGE_OCV_COL = "Charge Voltage"

# Reverse the values in individual columns when necessary.
#
# This is useful when, for example, the discharge curve is stored
# from 100% SOC to 0% SOC while the SOC column is stored from
# 0% SOC to 100% SOC.
SOC_FLIP_DATA = False
DISCHARGE_OCV_FLIP_DATA = True
CHARGE_OCV_FLIP_DATA = False

# Difference convention:
# False: charge OCV - discharge OCV
# True:  absolute hysteresis magnitude
ABSOLUTE_HYSTERESIS = False

# Remove rows containing non-numeric or missing selected values.
DROP_INVALID_ROWS = False

# Sort the final result by increasing SOC.
SORT_BY_SOC = False

# Output column names
SOC_OUTPUT_NAME = "soc"
DISCHARGE_OUTPUT_NAME = "discharge_curve"
CHARGE_OUTPUT_NAME = "charge_curve"
AVERAGE_OUTPUT_NAME = "average_curve"
DIFFERENCE_OUTPUT_NAME = "difference_curve_hysteresis"


# ============================================================
# PROCESSING FUNCTIONS
# ============================================================

ColumnSelector = Union[int, str]


def read_csv_file(file_path: Path) -> pd.DataFrame:
    """Read the configured CSV file."""
    if not file_path.exists():
        raise FileNotFoundError(f"Input CSV not found: {file_path.resolve()}")

    header = 0 if CSV_HAS_HEADER else None
    dataframe = pd.read_csv(file_path, header=header)

    if dataframe.empty:
        raise ValueError("The input CSV is empty.")

    return dataframe


def select_numeric_column(
    dataframe: pd.DataFrame,
    column: ColumnSelector,
    output_name: str,
    flip_data: bool = False,
) -> pd.Series:
    """
    Select a column by index or name, convert it to numeric,
    and optionally reverse its values.
    """
    if isinstance(column, int):
        if column < 0 or column >= dataframe.shape[1]:
            raise IndexError(
                f"Column index {column} is invalid. "
                f"The CSV contains {dataframe.shape[1]} columns."
            )
        series = dataframe.iloc[:, column].copy()

    elif isinstance(column, str):
        if column not in dataframe.columns:
            raise KeyError(
                f"Column '{column}' was not found. "
                f"Available columns: {list(dataframe.columns)}"
            )
        series = dataframe[column].copy()

    else:
        raise TypeError(
            f"Column selector must be an integer or string, not {type(column).__name__}."
        )

    # Invalid entries become NaN and can be removed later.
    series = pd.to_numeric(series, errors="coerce")

    if flip_data:
        # Reverse only the values while retaining the dataframe row index.
        series = pd.Series(
            series.iloc[::-1].to_numpy(),
            index=series.index,
        )

    series.name = output_name
    return series


def process_battery_curves(dataframe: pd.DataFrame) -> pd.DataFrame:
    """Extract the curves and calculate average and hysteresis."""

    soc = select_numeric_column(
        dataframe=dataframe,
        column=SOC_COL,
        output_name=SOC_OUTPUT_NAME,
        flip_data=SOC_FLIP_DATA,
    )

    discharge_ocv = select_numeric_column(
        dataframe=dataframe,
        column=DISCHARGE_OCV_COL,
        output_name=DISCHARGE_OUTPUT_NAME,
        flip_data=DISCHARGE_OCV_FLIP_DATA,
    )

    charge_ocv = select_numeric_column(
        dataframe=dataframe,
        column=CHARGE_OCV_COL,
        output_name=CHARGE_OUTPUT_NAME,
        flip_data=CHARGE_OCV_FLIP_DATA,
    )

    result = pd.concat(
        [soc, discharge_ocv, charge_ocv],
        axis=1,
    )

    if DROP_INVALID_ROWS:
        original_row_count = len(result)
        result = result.dropna().reset_index(drop=True)
        removed_row_count = original_row_count - len(result)

        if removed_row_count:
            print(
                f"Removed {removed_row_count} row(s) containing "
                "missing or non-numeric data."
            )

    if result.empty:
        raise ValueError(
            "No valid data remains after selecting and cleaning the columns."
        )

    # Mean OCV at each SOC.
    result[AVERAGE_OUTPUT_NAME] = (
        result[CHARGE_OUTPUT_NAME] + result[DISCHARGE_OUTPUT_NAME]
    ) / 2.0

    # Hysteresis voltage at each SOC.
    difference = (
        result[CHARGE_OUTPUT_NAME] - result[DISCHARGE_OUTPUT_NAME]
    )

    if ABSOLUTE_HYSTERESIS:
        difference = difference.abs()

    result[DIFFERENCE_OUTPUT_NAME] = difference

    if SORT_BY_SOC:
        result = (
            result.sort_values(SOC_OUTPUT_NAME)
            .reset_index(drop=True)
        )

    return result


def main() -> None:
    """Run the CSV processing workflow."""
    input_data = read_csv_file(INPUT_CSV)
    output_data = process_battery_curves(input_data)

    OUTPUT_CSV.parent.mkdir(parents=True, exist_ok=True)
    output_data.to_csv(OUTPUT_CSV, index=False)

    print(f"Input file:  {INPUT_CSV.resolve()}")
    print(f"Output file: {OUTPUT_CSV.resolve()}")
    print(f"Output rows: {len(output_data)}")
    print("\nFirst processed rows:")
    print(output_data.head())


if __name__ == "__main__":
    main()