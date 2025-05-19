#!/usr/bin/env python3

import sys
import threading

from PySide6.QtCore import QObject, Signal, Slot, Property, QUrl, QPoint, QPointF
from PySide6.QtQml import QQmlApplicationEngine, QmlElement
from PySide6.QtWidgets import QApplication
from PySide6.QtQuickControls2 import QQuickStyle
from PySide6.QtCharts import QChartView, QLineSeries, QValueAxis

# To be used on the @QmlElement decorator
# (QML_IMPORT_MINOR_VERSION is optional)
QML_IMPORT_NAME = "io.qt.textproperties"
QML_IMPORT_MAJOR_VERSION = 1

from sciduino import Sciduino

sciduino = Sciduino(safety_check=False)


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
        return sciduino.connection.port

    @port.setter
    def port(self, new_port):
        sciduino.connection.port = new_port

    @Property(int, notify=baudrate_changed)
    def baudrate(self):
        return sciduino.connection.baudrate

    @baudrate.setter
    def baudrate(self, new_baudrate):
        sciduino.conection.baudrate = new_baudrate

    @Slot(result=str)
    def measure(self):
        return str(sciduino.measure()) + 'V'

    @Slot(QLineSeries, QValueAxis, int, float)
    def burst(self, series, axis_x, measurements, frequency):
        raw_burst = sciduino.burst(measurements, frequency)
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

        cancel_timer = threading.Timer(5.1, sciduino.stop_streaming)
        cancel_timer.start()
        sciduino.start_streaming(1000, stream_callback)


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
