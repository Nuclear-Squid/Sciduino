#!/usr/bin/env python3

import ctypes
import textwrap
import time
import threading

import numpy as np
import serial


# NOTE: This is a Python representation of the AnalogInput type defined in the
# sciduino firmware. Make sure those two definitions match.
class AnalogInput(ctypes.Structure):
    # The board returrns a packed struct to ensure a consistant memory layout
    # across different architectures
    _pack_ = 1
    _fields_ = [
        ("_name",          ctypes.c_char * 16),
        ("_unit",          ctypes.c_char * 8),
        ("input_range_id", ctypes.c_uint8),
        ("precision",      ctypes.c_uint8),
        ("pin",            ctypes.c_uint8),
        ("enabled",        ctypes.c_bool),
    ]

    def __str__(self):
        return textwrap.dedent(f"""\
            name:           {self.name}
            unit:           {self.unit}
            input_range_id: {self.input_range_id}
            precision:      {self.precision}
            pin:            {self.pin}
            enabled:        {self.enabled}
        """)

    @property
    def name(self): return self._name.decode('ascii')
    @name.setter
    def name(self, new_name): self._name = bytes(new_name, 'utf8')

    @property
    def unit(self): return self._unit.decode('utf8')
    @unit.setter
    def unit(self, new_unit): self._unit = bytes(new_unit, 'utf8')

    def as_dict(self):
        return {
            "name":           self.name,
            "unit":           self.unit,
            "input_range_id": self.input_range_id,
            "precision":      self.precision,
            "pin":            self.pin,
            "enabled":        self.enabled,
        }

    @staticmethod
    def from_reader(reader) -> list[any]:
        input_count = int.from_bytes(reader.read())
        size = ctypes.sizeof(AnalogInput)
        return [AnalogInput.from_buffer_copy(reader.read(size)) for _ in range(input_count)]

    @staticmethod
    def from_scpi_str(input: str) -> list[any]:
        command_body = input.removeprefix(':input:')
        if input == command_body: raise ValueError(f'Wrong command, idiot:\n{input}')

        rv = []
        current_input = None
        raw_tokens = map(lambda s: s.split(maxsplit=1) + [None], command_body.split(';'))

        for arg, value, *_ in raw_tokens:
            # print(arg, value)
            match arg:
                case "begin":     current_input = AnalogInput()
                case "end":       rv.append(current_input)
                case "name":      current_input.name = value
                case "unit":      current_input.unit = value
                case "range":     current_input.input_range_id = int(value)
                # case "gain":      current_input.gain = float(value)
                # case "offset":    current_input.offset = float(value)
                case "precision": current_input.precision = int(value)
                case "pin":       current_input.pin = int(value)
                case "enabled":   current_input.enabled = value.lower() == "true"
                case _: raise ValueError(f'Invalid field: {arg}')
        return rv


class Waveform:
    class Header(ctypes.Structure):
        _pack_ = 1
        _fields_ = [
            ("length",   ctypes.c_uint32),
            ("time",     ctypes.c_float),
            ("interval", ctypes.c_float),
            ("pin",      ctypes.c_uint8),
        ]

        def __str__(self):
            return textwrap.dedent(f"""\
                length:   {self.length}
                time:     {self.time}
                interval: {self.interval}
                pin:      {self.pin}
            """)

        def from_reader(reader):
            return Waveform.Header.from_buffer_copy(reader.read(ctypes.sizeof(Waveform.Header)))

    def __init__(self):
        self.meta = Waveform.Header()

    def __str__(self):
        return str(self.meta) + 'data:\n' + str(self.data) + '\n'

    @staticmethod
    def from_reader(reader) -> list[any]:
        waveform_count = int.from_bytes(reader.read())
        rv = []
        for _ in range(waveform_count):
            new_waveform = Waveform()
            new_waveform.meta = Waveform.Header.from_reader(reader)
            raw_data = reader.read(2 * new_waveform.meta.length)

            new_waveform.data = np.frombuffer(
                raw_data,
                dtype=np.uint16,
                count=new_waveform.meta.length
            )
            rv.append(new_waveform)
        return rv

    @staticmethod
    def from_scpi_str(input: str) -> list[any]:
        command_body = input.removeprefix(':waveform:')
        if input == command_body: raise ValueError(f'Wrong command, idiot:\n{input}')

        rv = []
        current_waveform = None
        raw_tokens = map(lambda s: s.split(maxsplit=1) + [None], command_body.split(';'))

        for arg, value, *_ in raw_tokens:
            match arg:
                case "begin":    current_waveform = Waveform()
                case "end":      rv.append(current_waveform)
                case "length":   current_waveform.meta.length = int(value)
                case "time":     current_waveform.meta.time = float(value)
                case "interval": current_waveform.meta.interval = float(value)
                case "pin":      current_waveform.meta.pin = int(value)
                case "data":     current_waveform.data = np.fromiter(map(int, value.split(',')), dtype=np.uint16)
                case _: raise ValueError(f'Invalid field: {arg}')
        return rv


class InputRange(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("gain",   ctypes.c_float),
        ("offset", ctypes.c_float),
    ]

    def __str__(self):
        return f"{{ gain: {self.gain}, offset: {self.offset} }}"

    @staticmethod
    def from_reader(reader):
        input_range_count = int.from_bytes(reader.read())
        return [InputRange.from_buffer_copy(reader.read(ctypes.sizeof(InputRange))) for _ in range(input_range_count)]


class Sciduino():
    streaming_timer = None
    streaming_timer_interval = 0.2
    analog_inputs = []

    def __init__(self, board_name):
        self.connection = serial.Serial (
            port     = '/dev/ttyACM1',
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

        self.connection.reset_input_buffer()
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

        self.connection.write(b':format:ascii\n')
        self.connection.write(b':inputs?\n')
        self.analog_inputs = AnalogInput.from_scpi_str(self.connection.readline().decode('ascii'))
        self.connection.write(b':format:binary;:inputs:ranges?\n')
        self.available_input_ranges = InputRange.from_reader(self.connection)
        self.input_range = self.available_input_ranges[self.analog_inputs[0].input_range_id]


    def find_input_by_pin(self, pin: int) -> AnalogInput | None:
        for input in self.analog_inputs:
            if input.pin == pin:
                return input
        return None

    def find_input_by_channel_name(self, channel: str) -> AnalogInput | None:
        index = ord(channel) - ord('A')
        return self.analog_inputs[index] if index < len(self.analog_inputs) else None

    def analog_to_float(self, channel: str, value: int):
        analog_input = self.find_input_by_channel_name(channel)
        input_range = self.available_input_ranges[analog_input.input_range_id]
        return value * input_range.gain + input_range.offset

    def set_active_inputs(self, inputs):
        self.connection.write(bytes(f':inputs:set {','.join(inputs)}\n', 'ascii'));

    def measure(self) -> list[tuple[str, float]]:
        """ Get current voltage read by the ADC """
        self.connection.write(b':format:binary;:measure\n')

        format = self.connection.read()
        if format == b'A':
            raise NotImplementedError

        if format == b'B':
            values_count = int.from_bytes(self.connection.read())
            rv = [(self.connection.read().decode('ascii'), int.from_bytes(self.connection.read(2))) for _ in range(values_count)]
            return rv

        if format == b'E':
            error_message = b'E' + self.connection.readline()
            print(error_message.decode('ascii'))
            raise ValueError(error_message.decode('ascii'))

        # self.connection.write(bytes(':MEASURE\n', 'ascii'))
        # binary_val = int(self.connection.readline())
        # ai = self.analog_inputs[0]
        # return round(binary_val * ai.gain + ai.offset, ai.precision)

    def burst(self, measurements, frequency) -> list[Waveform] | None:
        """ Get a lot of mesurements in a small amount of time """

        self.connection.write(bytes(':format:binary\n', 'ascii'))
        self.connection.write(bytes(f':BURST {measurements},{frequency}\n', 'ascii'))

        # HACK: Wait for the data to come in, in order to not trigger the
        # timeout when immediataly trying to read values from the board when
        # it’s measuring the input signal.
        time.sleep(measurements / frequency)

        format = self.connection.read()
        if format == b'A':
            return Waveform.from_scpi_str(self.connection.readline().decode('ascii'))
        if format == b'B':
            return Waveform.from_reader(self.connection)
        if format == b'E':
            error_message = format + self.connection.readline()
            print(error_message.decode('ascii'))
            return None
        raise ValueError(f'Invalid response format: {format}')

    def streaming_timer_handler(self, callback, frequency):
        if self.connection.in_waiting > 0:
            new_waveforms = None
            format = self.connection.read()
            if format == b'A':
                new_waveforms = Waveform.from_scpi_str(self.connection.readline().decode('ascii'))
            elif format == b'B':
                new_waveforms = Waveform.from_reader(self.connection)
            elif format == b'E':
                error_message = format + self.connection.readline()
                print(error_message.decode('ascii'))
            else:
                raise ValueError(f'Invalid response format: {format}')

            callback(new_waveforms)

        self.streaming_timer = threading.Timer(
            self.streaming_timer_interval,
            self.streaming_timer_handler,
            args=[callback, frequency]
        )
        self.streaming_timer.start()

    def start_streaming(self, frequency: float, callback):
        self.connection.reset_input_buffer()
        self.connection.write(bytes(':format:binary\n', 'ascii'))
        self.connection.write(bytes(f':stream {frequency}\n', 'ascii'))
        self.streaming_timer_interval = 0.01
        self.streaming_timer = threading.Timer(
            self.streaming_timer_interval,
            self.streaming_timer_handler,
            args=[callback, frequency]
        )
        self.streaming_timer.start()
        self.timer_repeat = True

    def stop_streaming(self):
        self.streaming_timer.cancel()
        self.connection.write(b':stream:stop\n')
