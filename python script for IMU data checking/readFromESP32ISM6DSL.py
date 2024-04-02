# Read data from our own recorder
# Author: Kai Zhao
# Date: 11/10/2022
import math
import struct

from TxtDataLoader import TxtDataLoader
import matplotlib.pyplot as plt
from enum import unique, Enum
from scipy import fftpack
import numpy as np



file_name = "ESP32_ISM6DSL/DATA0001.bin"

ideal_sampling_rate =100  # HZ

timestamp = []
timeDif =[]

acc_x = []
acc_y = []
acc_z = []
gyr_x = []
gyr_y = []
gyr_z = []
TxtDataLoader.open_new_file(file_name)
while True:
    raw = TxtDataLoader.read_bytes(28)
    if len(raw) < 24:
        break
    temp_time =  raw[0] + raw[1] * 256 + raw[2] * 256 * 256 + raw[3] * 256 * 256 * 256
    temp_acc_x = struct.unpack('<f', raw[4:8])[0]
    temp_acc_y = struct.unpack('<f', raw[8:12])[0]
    temp_acc_z = struct.unpack('<f', raw[12:16])[0]

    temp_gyr_x = struct.unpack('<f', raw[16:20])[0]
    temp_gyr_y = struct.unpack('<f', raw[20:24])[0]
    temp_gyr_z = struct.unpack('<f', raw[24:28])[0]
    a = temp_acc_x*temp_acc_x+temp_acc_y*temp_acc_y+temp_acc_z*temp_acc_z
    if(a>200 or a<25 ):
        print("aa"+str(len(acc_x)))

    timestamp.append(temp_time)
    acc_x.append(temp_acc_x)
    acc_y.append(temp_acc_y)
    acc_z.append(temp_acc_z)
    gyr_x.append(temp_gyr_x)
    gyr_y.append(temp_gyr_y)
    gyr_z.append(temp_gyr_z)
    if len(timestamp) ==1:
        timeDif.append(0)
    else:
        timeDif.append(temp_time-timestamp[-2])


sampling_rate = (len(timestamp) - 1) / (timestamp[-1] - timestamp[0]) * 1000

print("Sampling rate: " + str(sampling_rate))
print("Duration: " + str((timestamp[-1]-timestamp[0])/1000))


color = ["blue", "green", "red","black"]
plt.figure()
line_1, = plt.plot(list(range(len(gyr_x))), acc_x,color = color[0], marker='o')
line_1.set_label("ACC X")
line_2, = plt.plot(list(range(len(gyr_x))), acc_y,color = color[1], marker='o')
line_2.set_label("ACC Y")
line_3, = plt.plot(list(range(len(gyr_x))), acc_z,color = color[2], marker='o')
line_3.set_label("ACC Z")
line_4, = plt.plot(list(range(len(gyr_x))), timeDif,color = color[3], marker='o')
line_4.set_label("time diff")
plt.legend(loc='upper left')
plt.xlabel("time/ms")
plt.ylabel("g")

plt.figure()
line_1, = plt.plot(list(range(len(gyr_x))), gyr_x,color = color[0], marker='o')
line_1.set_label("Gyro X")
line_2, = plt.plot(list(range(len(gyr_x))), gyr_y,color = color[1], marker='o')
line_2.set_label("Gyro Y")
line_3, = plt.plot(list(range(len(gyr_x))), gyr_z,color = color[2], marker='o')
line_3.set_label("Gyro Z")
plt.legend(loc='upper left')
plt.xlabel("time/ms")
plt.ylabel(" ")

# line_1, = plt.plot(list(range(0, len(acc_x))), acc_x, color=color[0])
# line_1.set_label("ACC X")
# line_2, = plt.plot(list(range(0, len(acc_y))), acc_y, color=color[1])
# line_2.set_label("ACC Y")
# line_3, = plt.plot(list(range(0, len(acc_z))), acc_z, color=color[2])
# line_3.set_label("ACC Z")
# plt.legend(loc='upper left')
# plt.xlabel("sample")
# plt.ylabel("mg")

# plt.figure()
# line_1, = plt.plot(time, gyr_x,color = color[0])
# line_1.set_label("GYR X")
# line_2, = plt.plot(time, gyr_y,color = color[1])
# line_2.set_label("GYR Y")
# line_3, = plt.plot(time, gyr_z,color = color[2])
# line_3.set_label("GYR Z")
# plt.legend(loc='upper left')
# plt.xlabel("time/ms")
# plt.ylabel("mdps")
#
# plt.figure()
# delta_time= [time[i+1] - time[i] for i in range(0,len(time)-1)]
# line_1, = plt.plot(time[:-1], delta_time,color = color[0])
# line_1.set_label("sampling interval ")
# plt.legend(loc='upper left')
# plt.xlabel("time/ms")
# plt.ylabel("time/ms")

# segment_start = 56000  ## file_name = "Cadia2R00003.txt"
# segment_end = 100000
# # segment_start = 190000  ## file_name = "Cadia2R00003.txt"
# # segment_end = 250000
# # segment_start = 520000  ## file_name = "Cadia2R00003.txt"
# # segment_end = 570000
# # segment_start = 11000  ## file_name = "Cadia1R00003.txt"
# # segment_end = 45000
# acc_x = acc_x[segment_start:segment_end]
# acc_y = acc_y[segment_start:segment_end]
# acc_z = acc_z[segment_start:segment_end]

# freq = ideal_sampling_rate
#
# plt.figure()
# Fx = fftpack.fft(acc_x)
# fx = fftpack.fftfreq(len(acc_x), 1.0 / freq)
# Fy = fftpack.fft(acc_y)
# fy = fftpack.fftfreq(len(acc_y), 1.0 / freq)
# Fz = fftpack.fft(acc_z)
# fz = fftpack.fftfreq(len(acc_z), 1.0 / freq)
# mask = np.where(fx >= 0)
#
# line_1, = plt.plot(fx[mask], abs(Fx[mask]) / len(acc_x), label="real", color=color[0])
# line_1.set_label("FFT ACC_X")
# line_2, = plt.plot(fy[mask], abs(Fy[mask]) / len(acc_y), label="real", color=color[1])
# line_2.set_label("FFT ACC_Y")
# line_3, = plt.plot(fz[mask], abs(Fz[mask]) / len(acc_z), label="real", color=color[2])
# line_3.set_label("FFT ACC_Z")
#
# plt.legend(loc='upper left')
# plt.xlabel("freq/Hz")
# plt.ylabel("Power density")
#
plt.show()
