#!/usr/bin/env python3

from enum import StrEnum
import random
import sys
import threading
import time

import serial
from PySide6.QtCore import QObject, Signal, Slot, Property, QUrl, QPoint, QPointF
from PySide6.QtQml import QQmlApplicationEngine, QmlElement
from PySide6.QtWidgets import QApplication
from PySide6.QtQuickControls2 import QQuickStyle
from PySide6.QtCharts import QChartView, QLineSeries, QValueAxis

# To be used on the @QmlElement decorator
# (QML_IMPORT_MINOR_VERSION is optional)
QML_IMPORT_NAME = "io.qt.textproperties"
QML_IMPORT_MAJOR_VERSION = 1


class Scpino():
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
        if response != 'rbi-scpino1k':
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

scpino = Scpino(safety_check=False)




@QmlElement
class Bridge(QObject):
    """ Empty element used to call python functions from QML """

    port_changed = Signal()
    baudrate_changed = Signal()

    def __init__(self, parent=None):
        super(Bridge, self).__init__(parent)
        self.my_data = []

    @Property(str, notify=port_changed)
    def port(self):
        return scpino.connection.port

    @port.setter
    def port(self, new_port):
        scpino.connection.port = new_port

    @Property(int, notify=baudrate_changed)
    def baudrate(self):
        return scpino.connection.baudrate

    @baudrate.setter
    def baudrate(self, new_baudrate):
        scpino.conection.baudrate = new_baudrate

    @Slot(result=str)
    def measure(self):
        return str(scpino.measure()) + 'V'

    @Slot(QLineSeries, QValueAxis, int, float)
    def burst(self, series, axis_x, measurements, frequency):
        raw_burst = scpino.burst(measurements, frequency)
        series.replace([QPointF(x, y) for (x, y) in raw_burst])
        axis_x.setMax(raw_burst[-1][0])
        axis_x.applyNiceNumbers()

    @Slot(float, QLineSeries, QValueAxis)
    def start_streaming(self, frequency, series, axis_x):
        self.graph_start_x_position = 0
        axis_x.setMin(0)
        axis_x.setMax(1)

        def stream_callback(new_values):
            fuck_qt = [QPointF(x + self.graph_start_x_position, y) for (x, y) in new_values]
            series.append(fuck_qt)

            overflow_x = new_values[-1][0] + self.graph_start_x_position - axis_x.max()
            if overflow_x > 0:
                axis_x.setMax(axis_x.max() + overflow_x)
                axis_x.setMin(axis_x.min() + overflow_x)

            waveform_size_x = (new_values[1][0] - new_values[0][0]) * (len(new_values) + 1)
            self.graph_start_x_position += waveform_size_x

        cancel_timer = threading.Timer(5.1, scpino.stop_streaming)
        cancel_timer.start()
        scpino.start_streaming(1000, stream_callback)


def main() -> None:
    app = QApplication(sys.argv)
    engine = QQmlApplicationEngine()

    # Add the current directory to the import paths and load the main module.
    dir_path = sys.path[0]
    engine.addImportPath(dir_path)
    engine.load(QUrl(dir_path + '/main.qml'))

    if not engine.rootObjects():
        sys.exit(-1)

    exit_code = app.exec()
    del engine
    sys.exit(exit_code)


if __name__ == "__main__":
    main()
