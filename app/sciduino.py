#!/usr/bin/env python3

if __name__ == "__main__":
    print("This is a module.")
    print("If you are manually running this file you are doing something stupid.")


import ctypes
import time
import threading

import numpy as np
import serial


# NOTE: This is a Python representation of the AnalogInput type defined in the
# sciduino firmware. Make sure those two definitions match.
class AnalogInput(ctypes.Structure):
    # The arduino returns a packed struct to minimize memory footprint.
    # Setting this field to 1 ensures this structure is packed in the same way.
    _pack_ = 1
    _fields_ = [
        ("name",      ctypes.ARRAY(ctypes.c_char, 16)),
        ("unit",      ctypes.ARRAY(ctypes.c_char, 8)),
        ("gain",      ctypes.c_float),
        ("offset",    ctypes.c_float),
        ("precision", ctypes.c_uint8),
        ("pin",       ctypes.c_uint8),
    ]

    def from_reader(reader):
        return AnalogInput.from_buffer_copy(reader.read(ctypes.sizeof(AnalogInput)))


class Waveform:
    class Header(ctypes.Structure):
        _pack_ = 1
        _fields_ = [
            ("initial_time",  ctypes.c_float),
            ("time_interval", ctypes.c_float),
            ("values_count",  ctypes.c_uint32),
            ("pin",           ctypes.c_uint8),
        ]

        def from_reader(reader):
            return Waveform.Header.from_buffer_copy(reader.read(ctypes.sizeof(Waveform.Header)))


    @staticmethod
    def from_reader(reader):
        rv = Waveform()
        rv.meta = Waveform.Header.from_reader(reader)
        raw_data = reader.read(2 * rv.meta.values_count)
        rv.data = np.frombuffer(
            raw_data,
            dtype=np.uint16,
            count=rv.meta.values_count
        )
        return rv


class Sciduino():
    streaming_timer = None
    streaming_timer_interval = 0.2
    analog_inputs = []

    def __init__(self, board_name):
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

        print("Establishing connection to the board", end="", flush=True)
        for _ in range(3):
            time.sleep(1 / 4)
            print(".", end="", flush=True)
        time.sleep(1 / 4)
        print(flush=True)

        self.connection.write(b'*idn?\n')
        response = self.connection.readline().decode('ascii').strip()
        if response != board_name:
            print("Wrong board, you’re connected to: ", response)
            raise ValueError()

        self.connection.write(b':sources:get\n')
        inputs_count = int.from_bytes(self.connection.read())
        for i in range(inputs_count):
            self.analog_inputs.append(AnalogInput.from_reader(self.connection))

    def find_input_by_pin(self, pin: int) -> AnalogInput | None:
        for input in self.analog_inputs:
            if input.pin == pin:
                return input
        return None

    def read_u16_value(self, max_value=3.3, resolution=10, precision=3):
        binary_val = int.from_bytes(self.connection.read(2), "little")
        return round(binary_val * max_value / (2 ** resolution), precision)

    def measure(self) -> float:
        """ Get current voltage read by the ADC """
        self.connection.write(bytes(':MEAS\n', 'ascii'))
        return self.read_u16_value()

    def burst(self, measurements, frequency) -> list[Waveform]:
        """ Get a lot of mesurements in a small amount of time """

        self.connection.write(bytes(f':BURST {measurements},{frequency}\n', 'ascii'))

        # HACK: Wait for the data to come in, in order to not trigger the
        # timeout when immediataly trying to read values from the board when
        # it’s measuring the input signal.
        time.sleep(measurements / frequency)

        return [Waveform.from_reader(self.connection) for _ in range(2)]

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
