#!/usr/bin/env python3

from enum import StrEnum
import random
import sys

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
    def measure_raw(self) -> int:
        """ Get the raw binary value of the ADC """
        self.write(bytes(':MEAS\n', 'ascii'))
        return int(self.readline().decode('ascii'))

    def measure(self) -> float:
        """ Get current voltage read by the ADC """
        self.write(bytes(':MEAS\n', 'ascii'))
        return round(int(self.readline().decode('ascii')) * 3.3 / 1024, 3)

    def burst(self) -> list[float]:
        """ Get a lot of mesurements in a small amount of time """
        self.write(bytes(':BRST\n', 'ascii'))
        return [ round(int(self.readline().decode('ascii')) * 3.3 / 1024, 3) for _ in range(50) ]

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

    @Slot(result=str)
    def getBurst(self):
        shit_json = '['
        for (i, x) in enumerate(scpino.burst()):
            shit_json += f'[{i * 50}, {x}],'

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
