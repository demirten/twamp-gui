import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1

Item {
    id: slider

    Layout.fillWidth: true
    Layout.fillHeight: true
    implicitWidth: 220

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

            style:SliderStyle{
                groove:Rectangle {
                    implicitHeight: mSlider.height * 0.3
                    implicitWidth: 100
                    radius: height/2
                    border.color: "#333"
                    color: "#eee"
                    Rectangle {
                        height: parent.height
                        width: styleData.handlePosition
                        implicitHeight: 6
                        implicitWidth: 100
                        radius: height/2
                        color: "#25b1e8"
                        opacity: 0.7
                    }
                }
                handle: Image {
                    id: imgHandle
                    source: "images/slider_handle.png"
                }
            }
        }

        Rectangle {
            id: valueContainer
            anchors.right: parent.right
            radius: 5
            color: "#eee"
            border.color: Qt.darker(color, 1.1)
            height: valueText.contentHeight + 5
            width: valueText.contentWidth + 10
            anchors.verticalCenter: parent.verticalCenter
            Text {
                id: valueText
                text: mSlider.value
                font.pointSize: 12
                color: "#555"
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
