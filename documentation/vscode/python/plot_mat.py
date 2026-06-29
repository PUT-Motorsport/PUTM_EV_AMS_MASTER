import scipy.io
import matplotlib.pyplot as plt
import numpy as np
import re
import csv

# ---------- Helper function ----------
def get_nested_attr(obj, path):
    """
    Supports paths like:
        sensor.time
        channels[0].voltage
        experiment.trials[5].force
    """
    for part in path.split("."):
        m = re.match(r"(\w+)\[(\d+)\]", part)

        if m:
            name, idx = m.groups()
            obj = getattr(obj, name)[int(idx)]
        else:
            obj = getattr(obj, part)

    return np.asarray(obj).squeeze()


# ---------- Load MAT file ----------
data = scipy.io.loadmat(
    "Molicel_INR21700P50B_measurement.mat",
    struct_as_record=False,
    squeeze_me=True
)

measurement = data["measurement"]


# ---------- Choose variables ----------
x_variable = "fu.DCC[1].t"

y_variables = [
    "fu.DCC[1].V",
    "fu.DCC[1].I",
]

# ---------- Extract x ----------
x = get_nested_attr(measurement, x_variable)

# ---------- Extract y data ----------
y_data = []
y_labels = []

for y_var in y_variables:
    y = get_nested_attr(measurement, y_var)
    y_data.append(y)
    y_labels.append(y_var)

# ---------- Align lengths ----------
n = len(x)
for y in y_data:
    n = min(n, len(y))

x = x[:n]
y_data = [y[:n] for y in y_data]

# ---------- WRITE TO CSV ----------
csv_filename = "battery_data.csv"

with open(csv_filename, mode="w", newline="") as f:
    writer = csv.writer(f)

    # header
    writer.writerow([x_variable] + y_labels)

    # rows
    for i in range(n):
        row = [x[i]] + [y_data[j][i] for j in range(len(y_data))]
        writer.writerow(row)

print(f"Saved CSV to {csv_filename}")

# ---------- Plot ----------
plt.figure(figsize=(10, 6))

for y_var, y in zip(y_labels, y_data):
    plt.plot(x, y, label=y_var)

plt.xlabel(x_variable)
plt.ylabel("Value")
plt.title("Selected variables")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()