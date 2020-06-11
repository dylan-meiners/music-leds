import serial
import time
import pyaudio
import numpy as np
import struct
from scipy.fftpack import fft
import pyqtgraph as pg
import math

COM_PORT = "COM4"

CHUNK = 2048
RATE = 44100

MAX = .4
MIN = 50

EMPTY_ARR_128 = []

EXPERIMENTAL = False
OUTPUT_GRAPH = False

fx = []
for i in range(128): fx.append(7.2167161275126 * pow(1.0235052413905, i))
for i in range(128):
    if fx[i] > 127: fx[i] = 127
    if int(fx[i]) not in fx: fx[i] = int(fx[i])
    else: fx[i] = None

ser = serial.Serial(port=COM_PORT, baudrate=2000000, timeout=None)
print("Connected over " + ser.name)
time.sleep(1)

p = pyaudio.PyAudio()
stream = p.open(format=pyaudio.paInt16,
                channels=1,
                rate=RATE,
                input=True,
                frames_per_buffer=CHUNK,
                input_device_index=3)
if OUTPUT_GRAPH:
    pw = pg.plot()
    pw.setRange(yRange=[0, 255]);

while True:
    raw_sound = stream.read(CHUNK)
    data_int = struct.unpack(str(2 * CHUNK) + 'B', raw_sound)
    data_np = np.array(data_int, dtype='b')[::2] + 128
    fft_data = (np.abs(np.fft.fft(data_int)[0:CHUNK]) / (128 * CHUNK))[1:129]

    if EXPERIMENTAL:
        #function indexing
        gx = []
        for i in range(128): gx.append(fft_data[fx[i]] if fx[i] != None else 0)
        gx = np.array(gx)
    else: gx = fft_data

    gx = gx * 255 / MAX

    for i in range(len(gx)):

        if gx[i] < MIN:
            gx[i] = 0

        if gx[i] > 255:
            gx[i] = 255
    
    gx = gx - MIN
    for i in range(len(gx)):
        if gx[i] < 0:
            gx[i] = 0
    gx = gx * 255 / (255 - MIN)

    if OUTPUT_GRAPH:
        pw.plot(np.arange(128), gx, clear=True)
        pg.QtGui.QApplication.processEvents()

    if ser.write(bytearray(list(gx.astype("int")))) == 128: print("Successfully sent all 128 bytes")
    else: print("Failed to write all 128 bytes")

    time.sleep(0.022)
