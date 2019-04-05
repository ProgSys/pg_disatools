import QtQuick 2.4
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0

Rectangle {
	property var elementModel
	property int elementHeight: 20
	
	Rectangle {
		height: elementHeight
		width: elementModel.duration*((timeline.steps*4)/10)
		y: 0
		anchors.topMargin: 1
		//x: timeline.getOffset(index)*((timeline.steps*4)/10)
		
		border.width: 1
		border.color: "black"

		//color: (model.index %2 === 0 ? "darkgray" : "lightgray")
		LinearGradient {
			anchors.fill: parent
			start: Qt.point(0, 0)
			end: Qt.point(parent.width, 0)
			gradient: Gradient {
				GradientStop { position: 0.0; color: "#d0d0d0" }
				GradientStop { position: 1.0; color: "#d8d8d8" }
			}
		}
		
		Text { 
			visible: ((elementModel.duration < 4) ? false : true)
			font.pointSize: 7.5
			text: elementModel.duration
			anchors.top: parent.top
			anchors.right: parent.right
			anchors.rightMargin: 4
			anchors.topMargin: 1
		}
		Rectangle {
			anchors.top: parent.top
			anchors.right: parent.right
			height: parent.height
			width: 2
			color: mouseArea.containsMouse ? "white": "#444444" 
			
			MouseArea {
				id: mouseArea
				hoverEnabled: true 
				anchors.fill: parent
				//cursorShape: containsMouse ? Qt.SizeHorCursor : Qt.ArrowCursor
			}
		}
	}//delegate 2
}