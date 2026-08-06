import numpy as np
import matplotlib.pyplot as plt

# L1 = 9.811
# L2 = 9.02
# L3 = 8.318
#idk
# L1 = np.log(18.23) # R1 [kOhm] at Y1 
# L2 = np.log(8.27)  # R2 [kOhm] at Y2
# L3 = np.log(4.0951) # R3 [kOhm] at Y3
# Y1 = 1 / 283 #[1/K]
# Y2 = 1 / 303 #[1/K]
# Y3 = 1 / 323 #[1/K]

# B is normal
# R is in kOhm
def get_coefs(B, R25):
    return [np.log(R25 * np.exp(B * (1/283.15 - 1/298.15))), np.log(R25 * np.exp(B * (1/303.15 - 1/298.15))), np.log(R25 * np.exp(B * (1/323.15 - 1/298.15))), 1/283.15, 1/303.15, 1/323.15, R25 * np.exp(B * (1/363.15 - 1/298.15)), R25 * np.exp(B * (1/263.15 - 1/298.15))]

poly_degree = 13  # change to 2, 3, 4, etc., for different fits

#dont remember
idk = [np.log(18.23), np.log(8.27), np.log(4.0951), 1 / 283, 1 / 303, 1 / 323]

# GA2.2K3A1IA
_2_252k_3976K = get_coefs(3976, 2.252)     
 
# B57861S0202F040
_2k_3560K = get_coefs(3560, 2)  

# TG310J34GBNR
_10k_3434K = get_coefs(3434, 10)

# 103AT-11, NTCLE413E2103F520L, NTCLE413E2103F106A
_10k_3435K = get_coefs(3435, 10)

# B57861S0103J040, B57861S0103F045
_10k_3988K = get_coefs(3988, 10)

# NTCLE413E2103H400
_10k_3984K = get_coefs(3984, 10)

# NTCLE413E2103F520L, TTS-10KC3-BZ, NTCLE300E3103SB
_10k_3977K = get_coefs(3977, 10)

# PANE103395
_10k_3950K = get_coefs(3950, 10)
L1, L2, L3, Y1, Y2, Y3, R_min, R_max = _10k_3950K

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
# R_min = 1    # minimum resistance in kOhms
# R_max = 10 # maximum resistance in kOhms
num_points = 500  # number of data points

# Generate R values (log scale for better sampling)
R_values = np.logspace(np.log10(R_min), np.log10(R_max), num_points)
lnR = np.log(R_values)

# Original equation: 1/T = A + B*lnR + C*(lnR)^3
T = 1 / (A + B * lnR + C * (lnR ** 3))
T_original = np.array(T)
T = [t if t < 321 else ((t - 321) / 3 + 321) for t in T]

# Fit a polynomial in R to approximate inv_T

poly_coeffs = np.polyfit(R_values, T, poly_degree)
poly_fit = np.poly1d(poly_coeffs)

# Evaluate the polynomial approximation
T_approx = poly_fit(R_values)

T_approx = T_approx - 273
T_original = T_original - 273
T = np.array(T) - 273

# --- Plotting for visualization ---
plt.figure(figsize=(10, 6))
plt.plot(R_values, T, label='T', color='blue')
plt.plot(R_values, T_approx, label=f'Polynomial Approx (deg {poly_degree})', linestyle='--', color='red')
plt.plot(R_values, T_original, label='T original', color='green')
plt.xscale('log')
plt.xlabel('Resistance R (Ohms)')
plt.ylabel('T degC')
plt.title('Stain-Hart Equation and Polynomial Approximation')
plt.legend()
plt.grid(True, which='both', linestyle=':')
plt.tight_layout()
plt.show()

# --- Output the polynomial ---
print(f"Polynomial coefficients (highest degree first):")
for i, coef in enumerate(poly_coeffs):
    print(f"a{i} = {coef:.6e}")