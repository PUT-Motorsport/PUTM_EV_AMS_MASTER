import numpy as np
import matplotlib.pyplot as plt

# L1 = 9.811
# L2 = 9.02
# L3 = 8.318
L1 = np.log(18.23)
L2 = np.log(8.27)
L3 = np.log(4.0951)
Y1 = 1 / 283
Y2 = 1 / 303
Y3 = 1 / 323

y2 = (Y2 - Y1)/(L2 - L1)
y3 = (Y3 - Y1)/(L3 - L1)

C = (y3 - y2) / (L1 + L2 + L3)
B = y2 - C * (L1**2 + L1 * L2 * L2**2)
A = Y1 - (B + L1**2 * C) * L1

# --- Input your constants here ---
# A = 0.001628  # example value
# B = 0.3482292631  # example value
# C = 4.853903323  # example value

# --- Define resistance range ---
R_min = 1     # minimum resistance
R_max = 20 # maximum resistance
num_points = 500  # number of data points

# Generate R values (log scale for better sampling)
R_values = np.logspace(np.log10(R_min), np.log10(R_max), num_points)
lnR = np.log(R_values)

# Original equation: 1/T = A + B*lnR + C*(lnR)^3
T = 1 / (A + B * lnR + C * (lnR ** 3))

# Fit a polynomial in R to approximate inv_T
poly_degree = 8  # change to 2, 3, 4, etc., for different fits
poly_coeffs = np.polyfit(R_values, T, poly_degree)
poly_fit = np.poly1d(poly_coeffs)

# Evaluate the polynomial approximation
T_approx = poly_fit(R_values)

# --- Plotting for visualization ---
plt.figure(figsize=(10, 6))
plt.plot(R_values, T, label='T', color='blue')
plt.plot(R_values, T_approx, label=f'Polynomial Approx (deg {poly_degree})', linestyle='--', color='red')
plt.xscale('log')
plt.xlabel('Resistance R (Ohms)')
plt.ylabel('T K')
plt.title('Stain-Hart Equation and Polynomial Approximation')
plt.legend()
plt.grid(True, which='both', linestyle=':')
plt.tight_layout()
plt.show()

# --- Output the polynomial ---
print(f"Polynomial coefficients (highest degree first):")
for i, coef in enumerate(poly_coeffs):
    print(f"a{i} = {coef:.6e}")