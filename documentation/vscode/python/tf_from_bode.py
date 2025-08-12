import numpy as np
from matplotlib import pyplot as plt
import csv
# import scipy.signal as sig
import control as ct
from slycot import sb10yd

def fitfrd(gfrd, n, *, tol=0, minphase=False):
    discfl = 0
    flag = int(minphase) 

    rfrdat = gfrd.fresp[0,0].real
    ifrdat = gfrd.fresp[0,0].imag

    omega = gfrd.omega

    lendat = len(gfrd.omega)
    
    print(discfl, flag, lendat, rfrdat, ifrdat, omega, n, tol)
    nout, a, b, c, d = sb10yd(discfl, flag, lendat, rfrdat, ifrdat, omega, n, tol)

    if nout != n:
        # really raise a warning; can't trigger this
        raise ValueError(f"asked for order {n}, got {nout}")
    return ct.ss(a, b, c, d)

frequency = np.array([])
gain = np.array([])
phase = np.array([])

with open('sdc_data/bode_wave.csv', 'r') as file:
    csvReader = csv.reader(file)

    csvData = np.array(list(csvReader))

    for f, g, p in csvData:
        frequency = np.append(frequency, np.float64(f))
        gain = np.append(gain, np.float64(g))
        phase = np.append(phase, np.float64(p))


response = np.multiply(gain, np.exp(1j * phase * np.pi * 180))
frdsys = ct.frd(response, frequency)
tfsys = ct.fitfrd()

print(tfsys)
