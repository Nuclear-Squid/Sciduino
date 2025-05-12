import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.1
import QtQuick.Window 2.1
import QtQuick.Controls.Material 2.1

ApplicationWindow {
    id: page
    width: 800
    height: 400
    visible: true
    Material.theme: Material.Dark
    Material.accent: Material.Red

    Bridge { id: bridge }

    ColumnLayout {
        Text {
            text: "Scpino1k control panel"
            color: "Ghost white"
            font: ({
                bold: true,
                pointSize: 24,
            })
        }

        GridLayout {
            ColumnLayout {
                TextField {
                    placeholderText: bridge.getPort()
                    onEditingFinished: bridge.setPort(displayText)
                    Layout.fillWidth: true
                }

                TextField {
                    placeholderText: bridge.getBaudrate()
                    onEditingFinished: bridge.setBaudrate(Number(displayText))
                    Layout.fillWidth: true
                }

                Button {
                    text: "Start stream"
                    Layout.fillWidth: true
                }

                Button {
                    text: "Get burst"
                    Layout.fillWidth: true
                    onClicked: scope.renderGraph(JSON.parse(bridge.getBurst()))
                }


                ColumnLayout {
                    Button {
                        id: oneShotButton
                        text: "Read one shot value"
                        onClicked: label.text = bridge.readOneShotValue()
                    }

                    Text {
                        id: label
                        text: ""
                        color: "Ghost white"
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
}
