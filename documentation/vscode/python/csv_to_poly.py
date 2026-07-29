import numpy as np
import matplotlib.pyplot as plt
import csv
import pandas as pd

OCV_COL = 2
SOC_COL = 0
OCV_FLIP_DATA = False
POLY_DEGREE = 16  # change to 2, 3, 4, etc., for different fits
VOLTAGE_MULTIPLIER = 144

# Load only specific columns by name
df = pd.read_csv('./data/molicelP50B/battery_curves_processed.csv')

# Show the loaded DataFrame
# print(df.head())
values = np.array(df.values)
values[:, 1] = values[:, 1]
print(values[:, 0])
# Fit a polynomial in R to approximate inv_T

soc = values[:, SOC_COL]
ocv = values[:, OCV_COL] * VOLTAGE_MULTIPLIER
if OCV_FLIP_DATA:
    ocv = ocv[::-1]

poly_coeffs = np.polyfit(soc, ocv, POLY_DEGREE) 
poly_fit = np.poly1d(poly_coeffs)

# Evaluate the polynomial approximation
ocv_approx = poly_fit(soc)

# --- Plotting for visualization ---
plt.figure(figsize=(10, 6))
plt.plot(soc, ocv, label='OCV(SOC)', color='blue')
plt.plot(soc, ocv_approx, label=f'Polynomial Approx (deg {POLY_DEGREE})', linestyle='--', color='red')
# plt.xscale('log')
plt.xlabel('soc')
plt.ylabel('ocv')
plt.title('OCV vs SOC')
plt.legend()
plt.grid(True, which='both', linestyle=':')
plt.tight_layout()
plt.show()

# --- Output the polynomial ---
print(f"Polynomial coefficients (highest degree first):")
for i, coef in enumerate(poly_coeffs):
    print(f"a{i} = {coef:.6e}")