#!/usr/bin/env python3

if __name__ == "__main__":
    print("This is a module.")
    print("If you are manually running this file you are doing something stupid.")


import time
import threading

import serial

class Sciduino():
    streaming_timer = None
    streaming_timer_interval = 0.2

    def __init__(self, safety_check=True):
        self.connection = serial.Serial (
            port     = '/dev/ttyACM0',
            baudrate = 115200,

            bytesize = serial.EIGHTBITS,
            parity   = serial.PARITY_NONE,
            stopbits = serial.STOPBITS_ONE,

            dsrdtr  = True,
            xonxoff = False,
            rtscts  = False,

            timeout       = 1,
            write_timeout = 1,
        )

        if not safety_check: return

        print("Establishing connection to the board", end="", flush=True)
        for _ in range(3):
            time.sleep(1 / 4)
            print(".", end="", flush=True)
        time.sleep(1 / 4)
        print(flush=True)

        self.connection.write(b'*idn?\n')
        response = self.connection.readline().decode('ascii').strip()
        if response != 'rbi-sciduino1k':
            print("Wrong board, you’re connected to: ", response)
            time.sleep(2)

    def read_u16_value(self, max_value=3.3, resolution=10, precision=3):
        binary_val = int.from_bytes(self.connection.read(2), "little")
        return round(binary_val * max_value / (2 ** resolution), precision)

    def measure(self) -> float:
        """ Get current voltage read by the ADC """
        self.connection.write(bytes(':MEAS\n', 'ascii'))
        return self.read_u16_value()

    def burst(self, measurements, frequency) -> list[tuple[float,float]]:
        """ Get a lot of mesurements in a small amount of time """

        self.connection.write(bytes(f':BURST {measurements},{frequency}\n', 'ascii'))

        # HACK: Wait for the data to come in, in order to not trigger the
        # timeout when immediataly trying to read values from the board when
        # it’s measuring the input signal.
        time.sleep(measurements / frequency)

        return [(i / frequency, self.read_u16_value()) for i in range(measurements)]

    # HACK: This should be the same value as the buffer on the board.
    # FIXME: Remove this once we have a nice protocol to chare waveforms.
    STREAMING_BUFFER_SIZE = 100
    def streaming_timer_handler(self, callback, frequency):
        if self.connection.in_waiting > 0:
            new_values = [(i / frequency, self.read_u16_value()) for i in range(self.STREAMING_BUFFER_SIZE)]
            callback(new_values)

        self.streaming_timer = threading.Timer(
            self.streaming_timer_interval,
            self.streaming_timer_handler,
            args=[callback, frequency]
        )
        self.streaming_timer.start()

    def start_streaming(self, frequency: float, callback):
        self.connection.write(bytes(f':stream {frequency}\n', 'ascii'))
        self.streaming_timer_interval = self.STREAMING_BUFFER_SIZE / frequency
        self.streaming_timer = threading.Timer(
            self.streaming_timer_interval,
            self.streaming_timer_handler,
            args=[callback, frequency]
        )
        self.streaming_timer.start()

    def stop_streaming(self):
        self.streaming_timer.cancel()
        self.connection.write(b':stream:stop\n')
        self.connection.reset_input_buffer()
