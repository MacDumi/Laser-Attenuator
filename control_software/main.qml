import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.15

Window {
    id: window
    width: 640
    height: 480
    visible: true
    property alias label: label
    property alias button: button
    property alias slider: slider
    title: qsTr("Hello WorlAAttenuatord")


    Slider {
        id: slider
        x: 147
        y: 283
        width: 410
        height: 22
        value: 0.5
    }

    Button {
        id: button
        text: qsTr("Click")
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Label {
        id: label
        y: 189
        width: 158
        height: 18
        color: "#852121"
        text: qsTr("Label text")
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
