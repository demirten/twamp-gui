import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.1
import "jbQuick"

ApplicationWindow {
    id: window
    visible: true
    property int margin: 10
    width: mainLayout.width + 2 * margin
    height: mainLayout.height + 2 * margin
    title: qsTr("Twamp Client")

    Connections {
        target: client

        onTestStarted: {
            progress.value = 0;
            progress.visible = true;
            sent.visible = true;
            sentProgressLabel.text = "0 of " + totalPackets;
            sentProgressLabel.visible = true
            testResults.visible = false
            btnStart.text = "STOP"
        }
        onTestFinished: {
            btnStart.text = "START"
        }

        onPacketSent: {
            sentProgressLabel.text = index + " of " + totalPackets
            progress.value = (index / totalPackets) * 0.95;
        }

        onDisplayError: {
            messageBox.text = message;
            messageBox.open();
        }

        onDatasetLatenciesChanged: {
            lineChart.requestPaint();
        }

        onCalculatedResults: {
            progress.value = 1
            testResults.visible = true

            var color = "green";
            if (packetLoss == 0) color = "green";
            else if (packetLoss < 1) color = "orange";
            else color = "red";
            packetLossLabel.text = "<b>Packet Loss:</b> <font color='" + color + "'>%" + parseFloat(packetLoss).toFixed(2) + "</font>";
            averageLatencyLabel.text = "<b>AvgLat:</b> " + parseFloat(averageLatency).toFixed(2) + " ms";
            averageJitterLabel.text = "<b>StdDev:</b> " + parseFloat(averageJitter).toFixed(2) + " ms";
            minLatencyLabel.text = "<b>MinLat:</b> " + parseFloat(minLatency).toFixed(2) + " ms";
            maxLatencyLabel.text = "<b>MaxLat:</b> " + parseFloat(maxLatency).toFixed(2) + " ms";
            btnStart.text = "START"
        }

    }

    statusBar: StatusBar {
        Label { width: parent.width; text: "About Twamp-Gui"; horizontalAlignment: Text.AlignRight; font.pixelSize: 12}
        MouseArea {
            anchors.fill: parent
            onClicked: {
                aboutBox.open()
            }
        }
    }

    ColumnLayout {
        id: mainLayout
        width: 700
        anchors.margins: 20
        spacing: 15

        //width: Math.max(window.viewport.width, column.implicitWidth + 2 * column.spacing)
        //height: Math.max(window.viewport.height, column.implicitHeight + 2 * column.spacing)

        GridLayout {
            id: optionsGrid
            columns: 6

            anchors.fill: parent
            anchors.margins: window.margin
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            //Layout.fillWidth: true
            //Layout.fillHeight: true


            Text { text: "Destination"; horizontalAlignment: Text.AlignRight }
            TextField { id: destination; text: "127.0.0.1"; Layout.fillWidth: true; width: 200}
            Text { text: "Port"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
            TextField { id: port; text: "862"; Layout.fillWidth: true }
            Text { text: "Total Packets"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
            SpinBox { id: total_packets; minimumValue: 5; maximumValue: 100; value: 20; Layout.fillWidth: true; }

            Text { text: "Interval (ms)"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
            SpinBox { id: interval; minimumValue: 10; maximumValue: 1000; value: 50; Layout.fillWidth: true; z: 1 }
            Text { text: "Payload (byte)"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
            SpinBox { id: payload; minimumValue: 0; maximumValue: 1472; value: 64; Layout.fillWidth: true; z: 1 }
            Text { text: "Light Mode"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
            CheckBox { id: light; checked: false; }

        }
        GroupBox {
            Layout.fillWidth: true;
            anchors.margins: window.margin
            anchors.left: parent.left
            RowLayout {
                anchors.fill: parent
                Button {
                    id: btnStart
                    text: "START"
                    onClicked: {
                        if (text == "START") {
                            client.startTest(light.checked, destination.text, port.text, total_packets.value, interval.value, payload.value)
                            text = "STOP"
                        } else {
                            client.stopTest()
                            text = "START"
                        }
                    }
                }
                Text {id: sent; visible: false; text: "Sent: "; width: 50; }
                Label {id: sentProgressLabel; visible: false; width: 150 }
                ProgressBar { id: progress; visible: false;}
            }

        }

        ColumnLayout {
            Rectangle {
                height: 180
                border.color: "#bbb"
                border.width: 0
                anchors.margins: window.margin
                anchors.left: parent.left
                Layout.fillWidth: true

                Chart {
                    id: lineChart
                    visible: true
                    Layout.fillWidth: true
                    anchors.fill: parent
                    anchors.topMargin: 5
                    chartAnimated: false
                    chartType: Charts.ChartType.LINE
                    chartData: {
                        "labels": client.xValues,
                                "datasets": [
                                    {
                                        fillColor: "rgba(0,152,18,0.2)",
                                        strokeColor: "rgba(151,187,205,1)",
                                        pointColor: "rgba(151,187,205,1)",
                                        pointStrokeColor: "#eee",
                                        pointDotStrokeWidth: 1,
                                        data: client.datasetLatencies
                                    }
                                ]
                    }
                }
            }

            Rectangle {
                height: 32
                anchors.margins: window.margin
                anchors.left: parent.left
                Layout.fillWidth: true
                Rectangle {
                    id: testResults
                    Layout.fillWidth: true
                    anchors.margins: window.margin
                    anchors.left: parent.left
                    anchors.fill: parent
                    visible: false
                    RowLayout {
                        anchors.fill: parent
                        Text { id: packetLossLabel; font.pixelSize: 12}
                        Text { id: averageLatencyLabel; font.pixelSize: 12}
                        Text { id: minLatencyLabel; font.pixelSize: 12 }
                        Text { id: maxLatencyLabel; font.pixelSize: 12 }
                        Text { id: averageJitterLabel; font.pixelSize: 12 }
                    }
                }
            }

            Rectangle {
                height: 200
                anchors.margins: window.margin
                anchors.left: parent.left
                Layout.fillWidth: true
                ScrollView {
                    anchors.fill: parent

                    ListView {
                        id: logListView
                        anchors.centerIn: parent
                        //spacing: parent.height * 0.01
                        model: client.logModel
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
                                    width: parent.width
                                    height: 18
                                    color: (index % 2 == 1) ? "#e7e7fe" : "#faf0d7"

                                    Text {
                                        id: timing
                                        text: modelData.timing
                                        color: "black"
                                        font.pixelSize: 13
                                    }

                                    Text {
                                        anchors.left: logSummary.left
                                        anchors.leftMargin: 80

                                        color: "black"
                                        font.pixelSize: 13
                                        text: modelData.summary
                                    }

                                }
                                ColumnLayout {
                                    id: logDetail
                                    width: parent.width
                                    spacing: 0

                                    anchors {
                                        top: logSummary.bottom;
                                        left: parent.left;
                                        right: parent.right;
                                    }

                                    property int savedHeight: 0

                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        if (logDetail.children.length === 0) {
                                            var detail = modelData.detail
                                            var i;
                                            var heightAdd = 0;
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
        id: aboutBox;
        objectName: "msgBox";
        icon: StandardIcon.Information;

        title: "About Twamp Gui v1.0.1";
        text: "Project home page:\nhttps://github.com/demirten/twamp-gui\n\nCopyright © Murat Demirten <mdemirten@yh.com.tr>"

        onAccepted: {
            close();
        }
    }

}

