import time
import numpy as np
import serial
import matplotlib.pyplot as plt

PORT = "COM16"    # Change to your Arduino port
BAUD = 115200
MAX_CM = 200         # furthest distance shown can do 2cm to 400cm 
REDRAW_EVERY = 5     # redraw after this many readings (higher = faster)

ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)        # the Uno resets when the port opens
ser.reset_input_buffer()

plt.ion()
fig = plt.figure(figsize=(7, 5))
ax = fig.add_subplot(111, polar=True)
ax.set_thetamin(0)
ax.set_thetamax(180)
ax.set_ylim(0, MAX_CM)
ax.set_title("Ultrasonic radar")
dots, = ax.plot([], [], "g.", markersize=8)

latest = {}          # angle -> latest distance, so old points are replaced as the sweep repeats
count = 0

try:
    while True:
        line = ser.readline().decode(errors="ignore").strip()
        try:
            angle_s, dist_s = line.split(",")
            angle, dist = int(angle_s), float(dist_s)
        except ValueError:
            continue                          # skip partial or garbled lines

        if 0 < dist <= MAX_CM:
            latest[angle] = dist
        else:
            latest.pop(angle, None)           # no echo or out of range: clear that angle

        count += 1
        if count % REDRAW_EVERY == 0:
            angles = np.radians(list(latest.keys()))
            dists = list(latest.values())
            dots.set_data(angles, dists)
            plt.pause(0.001)
except KeyboardInterrupt:
    pass
finally:
    ser.close()