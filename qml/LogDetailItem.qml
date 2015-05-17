import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2

Rectangle {
    id: logDetailItem
    width: parent.width

    height: 20
    color: "#f5f5f5"
    visible: true

    Label {
        id: message
        anchors.left: parent.left
        anchors.leftMargin: 80
        color: "#555"
        font.family: "Courier New"
        font.bold: true
    }
    Label {
        id: content
        anchors.left: parent.left
        anchors.leftMargin: 250
        color: "#222"
        font.family: "Courier New"
    }
    function setText(label, data) {
        message.text = label;
        content.text = data;
    }
}
