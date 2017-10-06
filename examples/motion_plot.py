#!/usr/bin/env python

import sys
import numpy as np
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
    print('Usage: {} FILE.csv'.format(sys.argv[0]))
    sys.exit(1)

t, p, v = np.loadtxt(sys.argv[1], delimiter=',', unpack=True)

td = np.diff(t)
td_mean = np.mean(td)
td_std = np.std(td)

plt.subplot(211)
plt.plot(t, p)

plt.title(r'Actual Position (tdiff: $\mu=%f, \sigma=%f$)' % (td_mean, td_std))
plt.xlabel('Time (s)')
plt.ylabel('Position (deg)')

plt.subplot(212)
plt.plot(t, v)

plt.title('Actual Velocity')
plt.xlabel('Time (s)')
plt.ylabel('Velocity (deg/s)')

plt.tight_layout()
plt.show()
