import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.1
import QtQuick.Window 2.1

ApplicationWindow {
    id: page
    width: 800
    height: 400
    visible: true

    Bridge { id: scpino }

    ColumnLayout {
        Text {
            text: "Scpino1k Control Panel"
            font: ({
                bold: true,
                pointSize: 24,
            })
        }

        GridLayout {
            ColumnLayout {
                Label { text: "Port:" }
                TextField {
                    text: scpino.port
                    onEditingFinished: scpino.port = displayText
                    Layout.fillWidth: true
                }

                Label { text: "Baudrate:" }
                TextField {
                    text: scpino.baudrate
                    onEditingFinished: scpino.baudrate = Number(displayText)
                    Layout.fillWidth: true
                }
            }

            Button {
                text: "Start stream"
                enabled: false
                Layout.fillWidth: true
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
                    id: frequency_field
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
                        let frequency = Number(frequency_field.text)
                        scope.renderGraph(JSON.parse(scpino.burst(measurements, frequency)))
                    }
                }
            }

            ColumnLayout {
                Button {
                    id: oneShotButton
                    text: "Read one shot value"
                    onClicked: label.text = scpino.measure()
                }

                Text {
                    id: label
                    text: ""
                    padding: 1
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        ScopeView {
            id: scope
            width: 600
            height: 400
        }
    }
}
