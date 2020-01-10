import QtQuick 2.4
import QtQuick.Controls 2.0

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