import numpy as np
from matplotlib import pyplot as plt
import csv

with open('sdc_data/sdc_profile0.csv', 'r') as file:
    csvReader = csv.reader(file)
    next(csvReader)
    data = np.array(list(csvReader))

time = data[0:200000, 0]
ch1 = data[0:200000, 1]
ch2 = data[0:200000, 2]

plt.plot(ch2, '.')
plt.show()