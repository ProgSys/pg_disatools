import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
	property int stepSize: 10
	property int index: 0
	property real timeScale: 1.0

	id: ruler
	SystemPalette { id: activePalette }
	color: activePalette.base

	enabled: false
	height: 24
	
	Repeater {
		model: parent.width / (stepSize*4) //parent.width / stepSize
		Rectangle {
			anchors.top: ruler.top
			height: ruler.height //(index % 4)? ((index % 2) ? 3 : 7) : 25
			width: 1
			color: activePalette.windowText
			x: index * stepSize * 4
		}
	}
	Repeater {
		model: parent.width / stepSize / 4
		Label {
			anchors.top: ruler.top
			anchors.topMargin: -2
			color: activePalette.windowText
			x: index * stepSize * 4 + 2
			text: (index * 10 / timeScale)
			font.pointSize: 7.5
		}
	}
}
