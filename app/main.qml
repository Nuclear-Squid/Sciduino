import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.1
import QtQuick.Window 2.1
import QtCharts
import Qt.labs.qmlmodels


ApplicationWindow {
    id: window
    title: "Sciduino"
    visible: true

    Bridge { id: bridge }

    ColumnLayout {
        id: page
        anchors.topMargin: 20
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.bottomMargin: 20
        anchors.centerIn: parent
        anchors.fill: parent

        Layout.maximumWidth: 1200

        Text {
            text: "Sciduino"
            font: ({  bold: true, pointSize: 25  })
            Layout.alignment: Qt.AlignHCenter
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: page.Layout.maximumWidth
            Layout.fillWidth: true

            Label { text: "Active inputs:" }

            TextField {
                text: bridge.get_active_channels()
                Layout.maximumWidth: 75
                validator: RegularExpressionValidator { regularExpression: /[A-Za-z]+/ }
                onAcceptableInputChanged: color = acceptableInput ? "black" : "red";
                onTextEdited: {
                    bridge.set_active_inputs(text.toUpperCase().split(''));
                    chart.set_visible_series(text.split(''));
                }
            }

            Text { id: inputs }

            Timer {
                interval: 250
                repeat: true
                running: true
                triggeredOnStart: true
                onTriggered: inputs.text = bridge.measure()
            }

            // XXX: This useless item is needed to **actually** align the other
            // elements to the left, otherwise there’s a shit ton of padding
            // added for no fucking reason. Wtf QT, why ?
            Item { Layout.fillWidth: true }
        }

        ChartView {
            id: chart
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: page.Layout.maximumWidth
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            antialiasing: true
            animationOptions: ChartView.NoAnimation

            ValueAxis {
                id: axisX
                titleText: "Time [s]"
                min: 0
                max: 1
            }

            ValueAxis {
                id: axisY
                titleText: "Voltage [V]"
                min: 0
                max: 5
            }

            Component.onCompleted: {
                for (const name of bridge.get_input_names()) {
                    let series = chart.createSeries(ChartView.SeriesTypeLine, name, axisX, axisY);
                    bridge.register_series(name, series);
                }

                this.set_visible_series(bridge.get_active_channels());
            }

            function set_visible_series(visible_series) {
                const visible_series_indexes = visible_series.map(c => c.charCodeAt(0) - 'a'.charCodeAt(0))
                for (let i = 0; i < this.count; i++) {
                    this.series(i).visible = visible_series_indexes.includes(i);
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            uniformCellSizes: true
            spacing: 100

            ColumnLayout {
                id: burst_column

                GridLayout {
                    columns: 2
                    uniformCellWidths: true

                    Label {
                        text: "number of points:"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }

                    TextField {
                        id: burst_points_field
                        text: "500"
                        validator: IntValidator { bottom: 1 }
                        onAcceptableInputChanged: color = acceptableInput ? "black" : "red";
                        horizontalAlignment: Text.AlignRight
                        Layout.maximumWidth: 75
                    }

                    Label {
                        text: "frequency (Hz):"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }

                    TextField {
                        id: burst_frequency_field
                        text: "5000"
                        validator: DoubleValidator { bottom: 1 }
                        onAcceptableInputChanged: color = acceptableInput ? "black" : "red";
                        horizontalAlignment: Text.AlignRight
                        Layout.maximumWidth: 75
                    }
                }

                Button {
                    text: "Burst"
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: {
                        let points = Number(burst_points_field.text);
                        let frequency = Number(burst_frequency_field.text);
                        stream_column.enabled = false;
                        read_inputs_timer.running = false
                        bridge.burst(points, frequency, axisX);
                        stream_column.enabled = true;
                        read_inputs_timer.running = true
                    }
                }
            }

            ColumnLayout {
                id: stream_column

                GridLayout {
                    columns: 2
                    uniformCellWidths: true

                    Label {
                        text: "time span (s):"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }

                    TextField {
                        id: stream_time_span_field
                        text: "0.05"
                        validator: DoubleValidator { bottom: 0; locale: "en_EN" }
                        onAcceptableInputChanged: color = acceptableInput ? "black" : "red";
                        horizontalAlignment: Text.AlignRight
                        Layout.maximumWidth: 75
                    }

                    Label {
                        text: "frequency (Hz):"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }

                    TextField {
                        id: stream_frequency_field
                        text: "5000"
                        validator: DoubleValidator { bottom: 1 }
                        onAcceptableInputChanged: color = acceptableInput ? "black" : "red";
                        horizontalAlignment: Text.AlignRight
                        Layout.maximumWidth: 75
                    }
                }

                Button {
                    text: "Stream"
                    Layout.alignment: Qt.AlignHCenter

                    property bool is_streaming: false

                    onClicked: {
                        if (is_streaming) {
                            text = "Stream";
                            is_streaming = false;
                            burst_column.enabled = true;
                            read_inputs_timer.running = true;
                            bridge.stop_streaming();
                        }
                        else {
                            let time = Number(stream_time_span_field.text);
                            let frequency = Number(stream_frequency_field.text);

                            text = "Stop";
                            is_streaming = true;
                            burst_column.enabled = false;
                            read_inputs_timer.running = false;
                            bridge.start_streaming(time, frequency, axisX);
                        }
                    }
                }
            }
        }
    }
}
