#!/usr/bin/env python

import sys
import numpy as np
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
    print('Usage: {} FILE.csv'.format(sys.argv[0]))
    sys.exit(1)
elif len(sys.argv) == 3:
    samples = int(sys.argv[2])
else:
    samples = -1

t, d = np.loadtxt(sys.argv[1], delimiter=',', unpack=True)
t, d = t[:samples], d[:samples]

plt.stem(t, d)

plt.title('Velocity vs. time')
plt.xlabel('Time (s)')
plt.ylabel('Velocity (rps)')

d_min = np.min(d)
d_max = np.max(d)
offset = 0.05
plt.ylim([d_min * (1 - offset), d_max * (1 + offset)])

plt.show()
