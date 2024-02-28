import matplotlib.pyplot as plt
import numpy as np
import serial
import serial.tools.list_ports
import time


BUFFER_SIZE = 100
t0 = time.time()

print("Starting COM")
# Find the available port
# https://pyserial.readthedocs.io/en/latest/tools.html#module-serial.tools.list_ports
# https://pyserial.readthedocs.io/en/latest/tools.html#serial.tools.list_ports.ListPortInfo
ports = [port for port, desc, info in serial.tools.list_ports.comports()]  
if len(ports) == 0:
    print("No devices available.")
    exit
    
if len(ports) > 1:
    print("Multiple devices detected:", ports)

print("Connecting to device:", ports[0])
com = serial.Serial(ports[0], baudrate=115200)
print("Started COM")

buffers = {}

t = np.zeros(BUFFER_SIZE)
i=0

while True:
    print("Awaiting line...", end='')
    line = str(com.readline()).split("\\r")[0].split("b'")[-1]   # Remove newline at end
    
    components = line.split(" ")

    print(f"Read components: {components}")

    # Junk data
    if "cam_hal" in line:
        continue

    if len(components) == 0:
        continue

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

        plt.scatter(buffers[label], t, label=label)#, s=2)
        plt.xlim([-1,1])

    plt.legend()
    plt.grid()
    plt.pause(.005)
    plt.clf()

    i = (i+1) % BUFFER_SIZE
