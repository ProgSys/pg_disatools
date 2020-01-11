import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
	property int index: 0
	property string layername: "noname"
	property int headerWidth: 100
	
	SystemPalette { id: activePalette }
	color: activePalette.midlight
	width: parent.width
	height: 20
	
	Rectangle {
		width: headerWidth
		height: 20
		color: activePalette.dark
		
		Text { 
			//font.pointSize: 7.5
			text: layername
			anchors.topMargin: 2
			//color: activePalette.light
		}
	}
	
	
}