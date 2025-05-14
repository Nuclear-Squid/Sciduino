#!/usr/bin/env python3

from enum import StrEnum
import random
import sys
import time

import serial
from PySide6.QtCore import QObject, Slot, QUrl
from PySide6.QtQml import QQmlApplicationEngine, QmlElement
from PySide6.QtWidgets import QApplication
from PySide6.QtQuickControls2 import QQuickStyle

# To be used on the @QmlElement decorator
# (QML_IMPORT_MINOR_VERSION is optional)
QML_IMPORT_NAME = "io.qt.textproperties"
QML_IMPORT_MAJOR_VERSION = 1


class Board(serial.Serial):
    def read_u16_value(self, max_value=3.3, resolution=10, precision=3):
        binary_val = int.from_bytes(self.read() + self.read(), "little")
        return round(binary_val * max_value / (2 ** resolution), precision)

    def measure(self) -> float:
        """ Get current voltage read by the ADC """
        self.write(bytes(':MEAS\n', 'ascii'))
        return self.read_u16_value()

    def burst(self, measurements, frequency) -> list[tuple[float,float]]:
        """ Get a lot of mesurements in a small amount of time """

        self.write(bytes(f':BURST {measurements},{frequency}\n', 'ascii'))

        # HACK: Wait for the data to come in, in order to not trigger the
        # timeout when immediataly trying to read values from the board when
        # it’s measuring the input signal.
        time.sleep(measurements / frequency)

        return [(i / frequency, self.read_u16_value()) for i in range(measurements)]

scpino = Board(
    port     = "/dev/ttyACM0",
    baudrate = 115200,
    timeout  = 1
)


@QmlElement
class Bridge(QObject):
    """ Empty element used to call python functions from QML """

    @Slot(result=str)
    def getPort(self):
        return scpino.port

    @Slot(str)
    def setPort(self, new_port):
        scpino.port = new_port

    @Slot(result=int)
    def getBaudrate(self):
        return scpino.baudrate

    @Slot(str)
    def setBaudrate(self, new_baudrate):
        scpino.baudrate = new_baudrate

    @Slot(result=str)
    def readOneShotValue(self):
        return str(scpino.measure()) + 'V'

    @Slot(int, float, result=str)
    def getBurst(self, measurements, frequency):
        shit_json = '['
        for (i, x) in scpino.burst(measurements, frequency):
            shit_json += f'[{i}, {x}],'

        shit_json = shit_json[:-1] + ']'
        return shit_json


def main() -> None:
    app = QApplication(sys.argv)
    engine = QQmlApplicationEngine()
    # Add the current directory to the import paths and load the main module.
    engine.addImportPath(sys.path[0])
    engine.load(QUrl('./main.qml'))

    if not engine.rootObjects():
        sys.exit(-1)

    exit_code = app.exec()
    del engine
    sys.exit(exit_code)


if __name__ == "__main__":
    main()
