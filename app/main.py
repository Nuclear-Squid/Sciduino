#!/usr/bin/env python3

import numpy as np
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

sciduino = Sciduino('rbi-sciduino1k')


@QmlElement
class Bridge(QObject):
    """ Empty element used to call python functions from QML """

    port_changed = Signal()
    baudrate_changed = Signal()

    chart_series: list[tuple[str, QLineSeries]] = []

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

    @Slot(str, QLineSeries)
    def register_series(self, name, series):
        self.chart_series.append((name, series))

    def find_series_by_name(self, wanted_name: str) -> QLineSeries | None:
        for name, series in self.chart_series:
            if name == wanted_name:
                return series
        return None

    @Slot(result=str)
    def get_input_names(self):
        shit_json = '['
        for input in sciduino.analog_inputs:
            shit_json += f'"{input.name}",'
        shit_json = shit_json[:-1] + ']'
        return shit_json

    @Slot(result=str)
    def measure(self):
        return str(sciduino.measure()) + 'V'

    @Slot(int, float, QValueAxis)
    def burst(self, measurements, frequency, x_axis):
        raw_bursts = sciduino.burst(measurements, frequency)

        time = np.linspace(
            raw_bursts[0].meta.time,
            raw_bursts[0].meta.interval * raw_bursts[0].meta.length,
            raw_bursts[0].meta.length
        )

        # series.replaceNp(time, raw_burst.data)
        # x_axis.applyNiceNumbers()

        for raw_burst in raw_bursts:
            analog_input = sciduino.find_input_by_pin(raw_burst.meta.pin)

            formated_burst = raw_burst.data * analog_input.gain + analog_input.offset
            fuck_qt = [QPointF(time[i], formated_burst[i]) for i in range(measurements)]

            self.find_series_by_name(analog_input.name).replace(fuck_qt)

            x_axis.setMax(raw_burst.meta.interval * raw_burst.meta.length)
            x_axis.applyNiceNumbers()


    @Slot(float, QValueAxis)
    def start_streaming(self, frequency, x_axis):
        self.graph_start_x_position = 0
        x_axis.setMin(0)
        x_axis.setMax(1)

        self.points_in_series = None

        def stream_callback(waveform_list):
            max_index = int(1 / waveform_list[0].meta.interval)
            header = waveform_list[0].meta
            time = np.linspace(
                header.time,
                header.time + header.interval * header.length,
                header.length
            )

            if self.points_in_series is None:
                self.points_in_series = [[] for _ in range(len(waveform_list))]

            for i, waveform in enumerate(waveform_list):
                analog_input = sciduino.find_input_by_pin(waveform.meta.pin)
                formated_stream = waveform.data * analog_input.gain + analog_input.offset
                self.points_in_series[i] += [QPointF(x, y) for x, y in zip(time, formated_stream)]
                self.find_series_by_name(analog_input.name).replace(self.points_in_series[i])

            overflow = len(self.points_in_series[0]) - max_index
            if overflow > 0:
                self.points_in_series[0] = self.points_in_series[0][overflow:]
                x_axis.setMin(self.points_in_series[0][0].x())
                x_axis.setMax(self.points_in_series[0][-1].x())

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
