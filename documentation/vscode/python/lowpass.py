import numpy as np
import matplotlib.pyplot as plt

i_lp_1 = 0.0
i_lp_2 = 0.0
rc = 0.005 # time constant
dt = 0.05 # time step
alpha = dt / (rc + dt)

def lowpass_filter(current, i_lp_1, i_lp_2):
    i_lp_1 = i_lp_1 + alpha * (current - i_lp_1)
    i_lp_2 = i_lp_2 + alpha * (i_lp_1 - i_lp_2)
    
    current_lp = i_lp_2

    return current_lp, i_lp_1, i_lp_2

# generate samples for 50 ms refreshrate
t = np.arange(0, 2, 0.05)
# generate step response
current = np.zeros_like(t)
current[20:] = 1.0
# generate sine wave response
# current = 0.5 * (1 + np.sin(2 * np.pi * 0.1 * t))

current_lp = np.zeros_like(t)

for i in range(len(t)):
    [current_lp[i], i_lp_1, i_lp_2] = lowpass_filter(current[i], i_lp_1, i_lp_2)

plt.figure(figsize=(10, 6))
plt.plot(t, current, label='Input Current (Step)', color='blue')
plt.plot(t, current_lp, label='Low-pass Filtered Current', color='red')
plt.xlabel('Time (s)')
plt.ylabel('Current (A)')
plt.title('Low-pass Filter Response')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()