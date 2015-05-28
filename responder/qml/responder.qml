import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.1

ApplicationWindow {
    id: window
    visible: true
    property int margin: 10
    title: qsTr("Twamp Responder")

    Connections {
        target: responder

        onResponderStarted: {
            btnStart.text = "STOP"
        }
        onResponderStopped: {
            btnStart.text = "START"
        }

        onDisplayError: {
            messageBox.text = message;
            messageBox.open();
        }


    }

    statusBar: StatusBar {
        Label { width: parent.width; text: "About Twamp-Gui"; horizontalAlignment: Text.AlignRight; font.pixelSize: 12}
        MouseArea {
            anchors.fill: parent
            onClicked: {
                aboutDialog.open()
            }
        }
    }

    ColumnLayout {
        id: mainLayout
        anchors.margins: window.margin
        anchors.fill: parent
        spacing: 0

        GridLayout {
            id: optionsGrid
            columns: 4

            anchors.top: parent.top
            anchors.left: parent.left

            Text { text: "Twamp Control TCP Listen Port: "; horizontalAlignment: Text.AlignRight }
            SpinBox { id: controlPort; minimumValue: 862; maximumValue: 65535; value: 862; }
            Text { text: "Twamp Light UDP Listen Port: "; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
            SpinBox { id: lightPort; minimumValue: 862; maximumValue: 65535; value: 862; }
        }
        GroupBox {
            Layout.fillWidth: true
            RowLayout {
                Button {
                    id: btnStart
                    text: "START"
                    onClicked: {
                        if (text == "START") {
                            responder.startServer(controlPort.value, lightPort.value)
                        } else {
                            responder.stopServer()
                        }
                    }
                }
                Button {
                    id: btnClear
                    text: "CLEAR LOGS"
                    onClicked: {
                        responder.clearLogs()
                    }
                }
            }
        }

        ColumnLayout {
            Rectangle {
                implicitHeight: 500
                anchors.left: parent.left
                Layout.fillWidth: true
                Layout.fillHeight: true
                ScrollView {
                    anchors.fill: parent

                    ListView {
                        id: logListView
                        anchors.centerIn: parent
                        model: responder.logModel
                        anchors.fill: parent

                        delegate: Rectangle {
                            id: delegateRect
                            width: ListView.view.width
                            height: logSummary.height

                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 0
                                Rectangle {
                                    id: logSummary
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 18
                                    Layout.maximumHeight: 50
                                    color: (index % 2 == 1) ? "#e7e7fe" : "#faf0d7"

                                    Text {
                                        id: timing
                                        text: modelData.timing
                                        color: "black"
                                        //font.pixelSize: 13
                                        font: Qt.font({ family: "monospace" })
                                    }

                                    Text {
                                        anchors.left: logSummary.left
                                        anchors.leftMargin: 80

                                        color: "black"
                                        //font.pixelSize: 13
                                        font: Qt.font({ family: "monospace" })
                                        text: modelData.summary
                                    }

                                }
                                ColumnLayout {
                                    id: logDetail
                                    Layout.fillWidth: true
                                    spacing: 0

                                    anchors {
                                        top: logSummary.bottom;
                                        left: parent.left;
                                    }

                                    property int savedHeight: 0

                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        if (logDetail.children.length === 0) {
                                            var i;
                                            var heightAdd = 0;
                                            var detail = modelData.detail()

                                            for (i = 0; i < detail.length; i += 2) {
                                                var component = Qt.createComponent("qrc:/LogDetailItem.qml");
                                                var object = component.createObject(logDetail);
                                                object.setText(detail[i], detail[i+1])
                                                heightAdd += object.height
                                            }

                                            logDetail.height = heightAdd
                                            logDetail.savedHeight = heightAdd
                                            delegateRect.height += heightAdd
                                        } else {
                                            if (!logDetail.visible) {
                                                delegateRect.height += logDetail.savedHeight
                                                logDetail.visible = true
                                            } else {
                                                delegateRect.height -= logDetail.savedHeight
                                                logDetail.visible = false
                                            }
                                        }
                                    }
                                }

                            }
                        }
                    }
                }
            }
        }

    }

    MessageDialog {
        id: messageBox;
        objectName: "msgBox";
        icon: StandardIcon.Critical;

        title: "Error";

        onAccepted: {
            close();
        }
    }
    MessageDialog {
        id: aboutDialog;
        icon: StandardIcon.Information;

        title: "About Twamp Gui v1.0.5";
        text: "Project home page:\nhttps://github.com/demirten/twamp-gui"
        informativeText: "Copyright © Murat Demirten <mdemirten@yh.com.tr>"
        onAccepted: {
            close();
        }
    }

}

