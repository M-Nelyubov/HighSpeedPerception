import matplotlib.pyplot as plt
import numpy as np
import serial
import time

BUFFER_SIZE = 100
t0 = time.time()

print("Starting COM")
com = serial.Serial("COM9",baudrate=115200)
print("Started COM")

buffers = {}

t = np.zeros(BUFFER_SIZE)
i=0

while True:
    print("Awaiting line...", end='')
    line = str(com.readline()).split("\\r")[0].split("b'")[-1]   # Remove newline at end
    
    components = line.split(" ")

    print(f"Read components: {components}")

    t[i] = time.time() - t0

    for c in components:
        if ":" not in c:
            continue
        pair = c.split(":")
        label = pair[0]
        val = float(pair[1])

        if label not in buffers:
            buffers[label] = np.zeros(BUFFER_SIZE)
        
        buffers[label][i] = val

        plt.scatter(t, buffers[label], label=label)#, s=2)

    plt.legend()
    plt.grid()
    plt.pause(.005)
    plt.clf()

    i = (i+1) % BUFFER_SIZE
