# this is a tps54202h calculations for optimal 

import numpy as np

Kind = 0.3
Vout = 12
Iout = 3.5#np.array([0.1, 0.5, 1])
Vinmax = 25.2
fsw = 1e6
Lo = 6.8e-6
Lmin = Vout * (Vinmax - Vout) / (Vinmax * Kind * Iout * fsw)

print('L: ', Lmin)
#267.71uH (0.1A) ~53.54uH (0.5A) or ~26.77uH (1A)
#~6.4uF or ~32uF
#Resr < 54.8mOhm

#final 1
#L => 68uH 
#Cout 22uF * 2? => 68uF

Ilmax = np.sqrt(Iout**2 + 1/12 * (Vout * (Vinmax - Vout) / (Vinmax * Lo * fsw * 0.8))**2)

print('Ilmax:', Ilmax)

dVout = 0.12

Cout = (Iout) / (fsw * dVout * Kind) * ((1 - Vout/Vinmax) * (1 + Kind) + Kind**2/12 * (2 - Vout/Vinmax))

ESR = ((2+Kind) * dVout) / (2 * Iout *(1 + Kind + Kind**2/12 * (1 + 1 / (1 - Vout/Vinmax))))

print('Cout: ', Cout)
print('ESR: ', ESR)