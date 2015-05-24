import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

Item {
    id: slider

    Layout.fillWidth: true
    Layout.fillHeight: true

    property alias minimumValue: mSlider.minimumValue
    property alias maximumValue: mSlider.maximumValue
    property alias value: mSlider.value
    property alias stepSize: mSlider.stepSize

    RowLayout {
        anchors.fill: parent

        Slider {
            id: mSlider
            anchors.left: parent.left
            anchors.right: valueContainer.left
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            Layout.fillWidth: true
            Layout.minimumWidth: 150
        }

        Rectangle {
            id: valueContainer
            anchors.right: parent.right
            radius: 10
            color: "#eee"
            border.color: Qt.darker(color, 1.4)
            height: valueText.contentHeight + 20
            width: valueText.contentWidth + 20
            anchors.verticalCenter: parent.verticalCenter
            Text {
                id: valueText
                text: mSlider.value
                font.pointSize: 14
                font.bold: true
                color: "#555"
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
