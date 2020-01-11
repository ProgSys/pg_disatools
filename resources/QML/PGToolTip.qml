import QtQuick 2.12
import QtQuick.Controls 2.12

ToolTip {
	id: control
	visible: parent.hovered
	text: ""
	font.pointSize:8
	delay: 600
	
	contentItem: Text {
        text: control.text
        font: control.font
        color: "black"
    }

    background: Rectangle {
        color: "#A0FFFFFF"
        border.color: "black"
		radius: 3
    }
}