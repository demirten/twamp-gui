import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.1
import "jbQuick"

ApplicationWindow {
    id: window
    visible: true
    property int margin: 10
    title: qsTr("Twamp Client")

    Component.onCompleted: {
        setX(Screen.width / 2 - width / 2);
        setY(Screen.height / 2 - height / 2);
    }

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
        //width: 700
        anchors.margins: window.margin
        anchors.fill: parent
        spacing: 0

        //width: Math.max(window.viewport.width, column.implicitWidth + 2 * column.spacing)
        //height: Math.max(window.viewport.height, column.implicitHeight + 2 * column.spacing)

        GridLayout {
            id: optionsGrid
            columns: getGridColumns()

            anchors.top: parent.top
            anchors.left: parent.left
            //Layout.fillWidth: true
            //Layout.fillHeight: true

            function getGridColumns() {
                if (Qt.platform.os == "android" || Qt.platform.os == "ios") {
                    return 2
                } else {
                    return (window.width > 1000) ? 6 : 4
                }
            }

            Text { text: "Destination"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
            TextField { id: destination; text: "127.0.0.1"; Layout.fillWidth: true; }
            Text { text: "Port"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
            TextField { id: port; text: "862"; Layout.fillWidth: true }
            Text { text: "Total Packets"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
            CustomSlider { id: total_packets; minimumValue: 5; maximumValue: 100; value: 20; stepSize: 1; Layout.fillWidth: true; }

            Text { text: "Interval (ms)"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
            CustomSlider {id: interval; minimumValue: 10; maximumValue: 1000; value: 50; stepSize: 10; Layout.fillWidth: true}
            Text { text: "Payload (byte)"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
            CustomSlider {id: payload; minimumValue: 0; maximumValue: 1472; value: 64; stepSize: 1}
            Text { text: "Light Mode"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
            CheckBox { id: light; checked: false; }

        }
        ColumnLayout {
            id: startGroup
            anchors.top: optionsGrid.bottom
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
                Text {id: sent; visible: false; text: "Sent Progress"; horizontalAlignment: Text.AlignRight;
                    anchors.right: sentProgressLabel.left; anchors.rightMargin: 10; Layout.fillWidth: true}
                Label {id: sentProgressLabel; visible: false; anchors.rightMargin: 20; anchors.right: progress.left;
                    horizontalAlignment: Text.AlignRight; }
                ProgressBar { id: progress; visible: false; height: 5; Layout.preferredWidth: 200; Layout.fillWidth: true}
            }

        }

        ColumnLayout {
            anchors.top: startGroup.bottom
            anchors.left: parent.left
            anchors.topMargin: 10
            spacing: 10

            Rectangle {
                anchors.left: parent.left
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 200
                Layout.minimumHeight: 200
                Layout.maximumHeight: 250

                Chart {
                    id: lineChart
                    visible: true
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
                anchors.left: parent.left
                Layout.fillWidth: true
                Layout.preferredHeight: packetLossLabel.contentHeight + 10
                Rectangle {
                    id: testResults
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: 5
                    Layout.fillWidth: true
                    anchors.left: parent.left
                    anchors.fill: parent
                    visible: false
                    RowLayout {
                        anchors.fill: parent
                        Text { id: packetLossLabel; }
                        Text { id: averageLatencyLabel; }
                        Text { id: minLatencyLabel;  }
                        Text { id: maxLatencyLabel;  }
                        Text { id: averageJitterLabel;  }
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 200
                Layout.minimumHeight: 200
                ScrollView {
                    anchors.fill: parent

                    ListView {
                        id: logListView
                        anchors.centerIn: parent
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
                                    Layout.fillWidth: true
                                    implicitHeight: timing.contentHeight + 2
                                    color: (index % 2 == 1) ? "#e7e7fe" : "#faf0d7"

                                    Text {
                                        id: timing
                                        text: modelData.timing
                                        color: "black"
                                        font: Qt.font({ family: "monospace" })
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Text {
                                        anchors.left: logSummary.left
                                        anchors.leftMargin: 80
                                        anchors.verticalCenter: parent.verticalCenter

                                        color: "black"
                                        font: Qt.font({ family: "monospace" })
                                        text: modelData.summary
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
                                                delegateRect.height += heightAdd
                                                logDetail.savedHeight = Qt.binding(function() { return heightAdd })
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
                                ColumnLayout {
                                    id: logDetail
                                    Layout.fillWidth: true
                                    spacing: 0

                                    property int savedHeight: 0

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

        title: "About Twamp Gui v1.0.5";
        text: "Project home page:\nhttps://github.com/demirten/twamp-gui\n\nCopyright © Murat Demirten <mdemirten@yh.com.tr>"

        onAccepted: {
            close();
        }
    }

}

