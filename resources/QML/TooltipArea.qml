import QtQuick 2.4

MouseArea {
	id: _root
	property alias text: toolTip.text

	anchors.fill: parent
	hoverEnabled: _root.enabled

	PGToolTip {
		visible: _root.containsMouse
		id: toolTip
	}
}