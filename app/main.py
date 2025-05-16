#!/usr/bin/env python3

from enum import StrEnum
import random
import sys
import time

import serial
from PySide6.QtCore import QObject, Signal, Slot, Property, QUrl
from PySide6.QtQml import QQmlApplicationEngine, QmlElement
from PySide6.QtWidgets import QApplication
from PySide6.QtQuickControls2 import QQuickStyle

# To be used on the @QmlElement decorator
# (QML_IMPORT_MINOR_VERSION is optional)
QML_IMPORT_NAME = "io.qt.textproperties"
QML_IMPORT_MAJOR_VERSION = 1


class Scpino():
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

scpino = Scpino(safety_check=False)


@QmlElement
class Bridge(QObject):
    """ Empty element used to call python functions from QML """

    port_changed = Signal()
    baudrate_changed = Signal()

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

    @Slot(int, float, result=str)
    def burst(self, measurements, frequency):
        shit_json = '['
        for (i, x) in scpino.burst(measurements, frequency):
            shit_json += f'[{i}, {x}],'

        shit_json = shit_json[:-1] + ']'
        return shit_json


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
