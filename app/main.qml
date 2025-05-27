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

        GridLayout {
            id: inputs
            columns: 3
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: page.Layout.maximumWidth
            Layout.fillWidth: true

            Component.onCompleted: {
                let input_json = bridge.get_input_names();
                let input_names = JSON.parse(input_json);
                for (const name of input_names) {
                    Qt.createQmlObject(`Text {
                        text: "${name}:"
                        horizontalAlignment: Text.AlignRight
                        Layout.fillWidth: true
                    }`, inputs)
                    Qt.createQmlObject(`Text { text: "0.000" }`, inputs)
                    Qt.createQmlObject(`Text { text: "V" }`, inputs)
                }
            }

            Timer {
                id: read_inputs_timer
                interval: 250
                repeat: true
                running: true
                onTriggered: {
                    const children = inputs.children;
                    for (let i = 1; i < children.length; i += 3) {
                        children[i].text = bridge.measure();
                        // console.log(bridge.measure());
                    }
                }
            }
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
                min: -1
                max: 4
            }

            Component.onCompleted: {
                let input_json = bridge.get_input_names();
                let input_names = JSON.parse(input_json);
                for (const name of input_names) {
                    let series = chart.createSeries(ChartView.SeriesTypeLine, name, axisX, axisY);
                    bridge.register_series(name, series);
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
                        text: "0.5"
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
                        text: "1000"
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
                        let time = Number(stream_time_span_field.text);
                        let frequency = Number(stream_frequency_field.text);

                        if (is_streaming) {
                            text = "Stream"
                            is_streaming = false
                            burst_column.enabled = true
                            read_inputs_timer.running = true
                            bridge.stop_streaming();
                        }
                        else {
                            text = "Stop";
                            is_streaming = true
                            burst_column.enabled = false
                            read_inputs_timer.running = false
                            bridge.start_streaming(time, frequency, axisX);
                        }
                    }
                }
            }
        }
    }
}
