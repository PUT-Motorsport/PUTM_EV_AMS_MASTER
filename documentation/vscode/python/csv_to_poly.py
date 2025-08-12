import numpy as np
import matplotlib.pyplot as plt
import csv
import pandas as pd

# Load only specific columns by name
df = pd.read_csv('cosmx_soc.csv')

# Show the loaded DataFrame
# print(df.head())
values = np.array(df.values)
values[:, 1] = values[:, 1] / 100
print(values[:, 0])
# Fit a polynomial in R to approximate inv_T
poly_degree = 8  # change to 2, 3, 4, etc., for different fits
poly_coeffs = np.polyfit(values[:, 1], values[:, 0], poly_degree) 
poly_fit = np.poly1d(poly_coeffs)

# Evaluate the polynomial approximation
ocv_approx = poly_fit(values[:, 1])

# --- Plotting for visualization ---
plt.figure(figsize=(10, 6))
plt.plot(values[:, 1], values[:, 0], label='OCV(SOC)', color='blue')
plt.plot(values[:, 1], ocv_approx, label=f'Polynomial Approx (deg {poly_degree})', linestyle='--', color='red')
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