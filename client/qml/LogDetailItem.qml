import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2

Rectangle {
    id: logDetailItem
    Layout.fillWidth: true

    height: message.contentHeight
    color: "#f5f5f5"
    visible: true

    Label {
        id: message
        anchors.left: parent.left
        anchors.leftMargin: 80
        color: "#555"
        //font.family: "Courier New"
        //font.bold: true
        font: Qt.font({ family: "monospace" })
    }
    Label {
        id: content
        anchors.left: parent.left
        anchors.leftMargin: 270
        //Layout.fillWidth: true
        color: "#222"
        font: Qt.font({ family: "monospace" })
    }
    function setText(label, data) {
        message.text = label;
        content.text = data;
        if (label === "Warning") {
            message.color = "red";
        }
    }
}
