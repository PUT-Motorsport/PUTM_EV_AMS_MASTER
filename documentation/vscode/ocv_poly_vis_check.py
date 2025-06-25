import numpy as np
import matplotlib.pyplot as plt

poly = np.array([2034.785202, -9878.314180, 20304.286795, -22998.124140, 15652.018744, -6548.995145, 1657.814181, -240.646928, 18.231580, 3.143621])
# d_pol = np.polyder(poly)


x = np.linspace(0, 1, 100)
y = np.polyval(poly, x)

# Mark the first, mid, and last points
first_point = (x[0], y[0])
mid_point = (x[len(x)//2], y[len(x)//2])
last_point = (x[-1], y[-1])

plt.scatter(*first_point, color='red', label=f'First Point ({first_point[0]:.2f}, {first_point[1]:.2f})')
plt.scatter(*mid_point, color='green', label=f'Mid Point ({mid_point[0]:.2f}, {mid_point[1]:.2f})')
plt.scatter(*last_point, color='blue', label=f'Last Point ({last_point[0]:.2f}, {last_point[1]:.2f})')

# dy = np.polyval(d_pol, x)
plt.plot(x, y, label='Polynomial')
# plt.plot(x, dy, label='Derivative')
plt.axhline(0, color='black', lw=0.5)
plt.axvline(0, color='black', lw=0.5)
plt.title('Polynomial and its Derivative')
plt.xlabel('x')
plt.ylabel('y')
plt.legend()
plt.grid()
plt.show()
