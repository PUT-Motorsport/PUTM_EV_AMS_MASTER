# this is a tps54202h calculations for optimal 

import numpy as np

Kind = 0.3
Vout = 5
Iout = 0.5#np.array([0.1, 0.5, 1])
Vinmax = 25.4
fs = 500000
Lo = 68e-6
Lmin = Vout * (Vinmax - Vout) / (Vinmax * Kind * Iout * fs)

print(Lmin)
#267.71uH (0.1A) ~53.54uH (0.5A) or ~26.77uH (1A)
deltaI = 0.4
deltaV = np.array([0.25, 0.05])
Co = 2 * deltaI / fs / deltaV
print(Co)
#~6.4uF or ~32uF
#Resr < 54.8mOhm

#final 1
#L => 68uH 
#Cout 22uF * 2? => 68uF

Ilmax = np.sqrt(Iout**2 + 1/12 * (Vout * (Vinmax - Vout) / (Vinmax * Lo * fs * 0.8))**2)

print(Ilmax)
