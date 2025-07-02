#!/usr/bin/env python3

import math
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


def round_significant_figures(value, figures):
    return round(value, figures - 1 - int(math.log10(figures)))


def auto_scale(volt_per_channels, padding_ratio, significant_figures):
    y_min = min(map(lambda x: min(x), volt_per_channels))
    y_max = max(map(lambda x: max(x), volt_per_channels))

    amplitude = y_max - y_min
    y_min -= round_significant_figures(amplitude * padding_ratio, significant_figures)
    y_max += round_significant_figures(amplitude * padding_ratio, significant_figures)

    return (y_min, y_max)


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

    @Property(list)
    def analog_inputs(self):
        return list(map(lambda x: x.as_dict(), sciduino.analog_inputs))

    @Slot(str, QLineSeries)
    def register_series(self, name, series):
        self.chart_series.append((name, series))

    def find_series_by_name(self, wanted_name: str) -> QLineSeries | None:
        for name, series in self.chart_series:
            if name == wanted_name:
                return series
        return None


    @Slot(result=list)
    def get_input_names(self):
        return list(map(lambda x: x.name, sciduino.analog_inputs))

    @Slot(result=list)
    def get_active_channels(self):
        rv = []
        for i, x in enumerate(sciduino.analog_inputs):
            if x.enabled:
                rv.append(chr(ord('a') + i))
        return rv

    @Slot(list)
    def set_active_inputs(self, inputs):
        sciduino.set_active_inputs(inputs)

    @Slot(result=list)
    def measure(self):
        return list(map(lambda x: [ord(x[0]) - ord('A'), sciduino.analog_to_float(x[1])], sciduino.measure()))

    @Slot(result='QVariant')
    def test(self):
        return { "a": 12, "b": 42 }

    @Slot(int, float, QValueAxis, QValueAxis)
    def burst(self, measurements, frequency, x_axis, y_axis):
        waveforms = sciduino.burst(measurements, frequency)

        time = np.linspace(
            waveforms[0].meta.time,
            waveforms[0].meta.interval * waveforms[0].meta.length,
            waveforms[0].meta.length
        )

        # series.replaceNp(time, raw_burst.data)
        # x_axis.applyNiceNumbers()

        volts_per_channel = []

        for raw_burst in waveforms:
            analog_input = sciduino.find_input_by_pin(raw_burst.meta.pin)

            # formated_burst = raw_burst.data * analog_input.gain + analog_input.offset
            formated_burst = sciduino.analog_to_float(raw_burst.data)
            volts_per_channel.append(formated_burst)
            fuck_qt = [QPointF(time[i], formated_burst[i]) for i in range(measurements)]

            self.find_series_by_name(analog_input.name).replace(fuck_qt)

        x_axis.setMin(0)
        x_axis.setMax(waveforms[0].meta.interval * waveforms[0].meta.length)
        # x_axis.applyNiceNumbers()

        y_min, y_max = auto_scale(volts_per_channel, 0.05, 2)
        y_axis.setMin(y_min)
        y_axis.setMax(y_max)


    @Slot(float, float, QValueAxis, QValueAxis)
    def start_streaming(self, time_span, frequency, x_axis, y_axis):
        self.graph_start_x_position = 0
        x_axis.setMin(0)
        x_axis.setMax(time_span)

        self.points_in_series = None
        self.volts_in_series = None

        def stream_callback(waveform_list):
            max_index = int(time_span / waveform_list[0].meta.interval)
            header = waveform_list[0].meta
            time = np.linspace(
                header.time,
                header.time + header.interval * header.length,
                header.length
            )

            if self.points_in_series is None:
                self.points_in_series = [[] for _ in range(len(waveform_list))]
                self.volts_in_series = [np.array([]) for _ in range(len(waveform_list))]

            for i, waveform in enumerate(waveform_list):
                analog_input = sciduino.find_input_by_pin(waveform.meta.pin)

                formated_stream = sciduino.analog_to_float(waveform.data)

                self.volts_in_series[i] = np.append(self.volts_in_series[i], formated_stream)
                self.points_in_series[i] += [QPointF(x, y) for x, y in zip(time, formated_stream)]
                self.find_series_by_name(analog_input.name).replace(self.points_in_series[i])

                overflow = len(self.points_in_series[i]) - max_index
                if overflow > 0:
                    self.points_in_series[i] = self.points_in_series[i][overflow:]
                    self.volts_in_series[i] = self.volts_in_series[i][overflow:]
                    x_axis.setMin(self.points_in_series[i][0].x())
                    x_axis.setMax(self.points_in_series[i][-1].x())

            y_min, y_max = auto_scale(self.volts_in_series, 0.05, 2)
            y_axis.setMin(y_min)
            y_axis.setMax(y_max)

        sciduino.start_streaming(frequency, stream_callback)

    @Slot()
    def stop_streaming(self):
        sciduino.stop_streaming()


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
