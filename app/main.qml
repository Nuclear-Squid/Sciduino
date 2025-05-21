import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.1
import QtQuick.Window 2.1
import QtCharts


ApplicationWindow {
    id: window
    title: "Sciduino Control Panel"
    width: 640
    height: 480
    visible: true

    Bridge { id: bridge }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "Sciduino Control Panel"
            font: ({  bold: true, pointSize: 25  })
            Layout.alignment: Qt.AlignHCenter
        }


        GridLayout {
            ColumnLayout {
                Label { text: "Port:" }
                TextField {
                    text: bridge.port
                    onEditingFinished: bridge.port = displayText
                    Layout.fillWidth: true
                }

                Label { text: "Baudrate:" }
                TextField {
                    text: bridge.baudrate
                    onEditingFinished: bridge.baudrate = Number(displayText)
                    Layout.fillWidth: true
                }
            }

            ColumnLayout {
                Label { text: "Frequency [Hz]" }
                TextField {
                    id: frequency_field_stream
                    text: "5000"
                    validator: DoubleValidator { bottom: 1 }
                    onAcceptableInputChanged: color = acceptableInput ? "black" : "red";
                    Layout.fillWidth: true
                }

                Button {
                    text: "Start Streaming"
                    Layout.fillWidth: true
                    onClicked: {
                        let frequency = Number(frequency_field_stream.text)
                        bridge.start_streaming(frequency, chart.series(0), axisX)
                    }
                }
            }

            ColumnLayout {
                Label { text: "Measurements" }
                TextField {
                    id: measurements_field
                    text: "500"
                    validator: IntValidator { bottom: 1 }
                    onAcceptableInputChanged: color = acceptableInput ? "black" : "red";
                    Layout.fillWidth: true
                }

                Label { text: "Frequency [Hz]" }
                TextField {
                    id: frequency_field_burst
                    text: "5000"
                    validator: DoubleValidator { bottom: 1 }
                    onAcceptableInputChanged: color = acceptableInput ? "black" : "red";
                    Layout.fillWidth: true
                }

                Button {
                    text: "Get burst"
                    Layout.fillWidth: true
                    onClicked: {
                        let measurements = Number(measurements_field.text)
                        let frequency = Number(frequency_field_burst.text)
                        bridge.burst(chart.series(0), chart.series(1), axisX, measurements, frequency)
                    }
                }
            }

            ColumnLayout {
                Button {
                    text: "Read one shot value"
                    onClicked: oneShotValue.text = bridge.measure()
                }

                Text {
                    id: oneShotValue
                    text: ""
                    padding: 1
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        ChartView {
            id: chart
            width: 600
            height: 400

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
                for (const name of input_names)
                    chart.createSeries(ChartView.SeriesTypeLine, name, axisX, axisY);
            }
        }
    }
}
