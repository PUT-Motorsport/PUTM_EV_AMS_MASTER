import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# -----------------------------
# 1. Load data
# -----------------------------
df = pd.read_csv("battery_data.csv")

time = df["time"].values
V = df["voltage"].values
I = df["current"].values

# -----------------------------
# 2. Battery parameters
# -----------------------------
capacity_Ah = 5   # <-- set your battery capacity here
SOC0 = 100          # initial SOC (%), change if needed

# -----------------------------
# 3. Time step (important)
# -----------------------------
dt = np.diff(time, prepend=time[0])  # seconds

# -----------------------------
# 4. Coulomb counting SOC
# -----------------------------
# Convert current (A) over time into Ah consumed
Ah_used = np.cumsum(I * dt * -1) / 3600.0

SOC = SOC0 - (Ah_used / capacity_Ah) * 100

# Clip SOC
SOC = np.clip(SOC, 0, 100)

# -----------------------------
# 5. Plot results
# -----------------------------
plt.figure(figsize=(12, 6))

plt.subplot(2, 1, 1)
plt.plot(time, V, label="Voltage")
plt.ylabel("Voltage (V)")
plt.legend()
plt.grid()

plt.subplot(2, 1, 2)
plt.plot(time, SOC, color="green")
plt.ylabel("SOC (%)")
plt.xlabel("Time (s)")
plt.grid()

plt.tight_layout()
plt.show()