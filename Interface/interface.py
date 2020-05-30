import serial
import time 
import struct 
import array
import sys 
import serial.tools.list_ports
import matplotlib
import matplotlib.pyplot as plt
import numpy as np

class CONTROL:
    def __init__(self, setpoint, pressure, output, timestamp):
        self.setpoint = setpoint
        self.pressure = pressure
        self.output = output
        self.timestamp = timestamp


def get_packet(ser, start_time, timeout):
    step = 0
    count = 0
    chk = 0
    payload = []

    while time.time() < start_time + timeout:
        try:
            data = ser.read(1)
        except TypeError:
            return None
        if data:
            data = ord(data)

        if(step == 0 and data == 0x5E):
            count = 0
            chk = 0
            step += 1
        elif(step == 1):
            if(count < 12):
                payload.append(data)
                count += 1
                chk += data
                chk &= 0xFF
                if count == 12:
                    step += 1
        elif(step == 2):
            if(data == chk):
                return payload
            else:
                print("Checksum wrong")
                print(data)
                print(chk)
                step = 0
                count = 0


if __name__ == "__main__":

    # Figure out the correct port
    port = ""
    connected = [comport for comport in serial.tools.list_ports.comports()]

    for comport in connected:
        if "ASF" in comport[1]:
            port = comport[0]
            break

    if port != "":
        ser = serial.Serial(port, timeout=0.1)  # open serial port
        print("Connected to Low Cost Ventilator")
    else:
        print("Could not connect to Low Cost Ventilator")
        sys.exit()
    

    measurements = []

    start_time = time.time()

    if(len(sys.argv) > 1):
        runtime = float(sys.argv[1])
    else:
        runtime = 15.0

    while(time.time() < start_time + runtime):
        # get payload
        payload = get_packet(ser, time.time(), 0.1)
        if payload:
            # unpack it
            line_spec = "<iif"
            info = struct.unpack(line_spec, array.array('B',payload).tostring())

            measurements.append(CONTROL(pressure=info[0],setpoint=info[1],output=info[2],timestamp=time.time()))

    ser.close()             # close port
    print("Readings complete")
    print("Readings received: {}".format(len(measurements)))

    times_s = np.array([m.timestamp for m in measurements])
    times_s = times_s - times_s[0]
    pressure = np.array([m.pressure for m in measurements])
    setpoint = np.array([m.setpoint for m in measurements])
    output = np.array([m.output for m in measurements])

    fig, ax = plt.subplots()
    plt.subplot(2,1,1)
    plt.plot(times_s, pressure, '.-',label='Measured')
    plt.plot(times_s, setpoint, '.-',label='Desired')
    plt.legend()
    plt.grid()
    plt.ylabel('Pressure, cmH20')
    plt.subplot(2,1,2)
    plt.plot(times_s, output, '.-',label='Output')
    plt.ylabel('Output')
    plt.legend()
    plt.grid()

    fig, ax = plt.subplots()
    ax.plot(times_s, pressure, '.-',label='Measured')
    ax.plot(times_s, output, '.-',label='Output')
    ax.legend()
    ax.grid()

    plt.show()