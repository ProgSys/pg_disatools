import QtQuick 2.4
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import MySpriteData 0.1

Rectangle {
	id: root
	property var markerModel
	property var currentAnimation
	property int elementHeight: 10
	property bool isSelected: false
	property int elementType: 1
	
	function getTrackScale(){
		return ((timeline.steps*4)/10)
	}
	
	signal selected(var item)
	signal deselected(var item)

	x: markerModel.start*getTrackScale()
	width: markerModel.duration*getTrackScale()
	height: elementHeight
	border.width:1
	border.color:"black"
	
	function getmarkerModelColor(){
		if(mouseArea.containsMouse || isSelected)
			return "white"
		if(markerModel.type == 1){
			return (markerModel.a || markerModel.b)? "lightgreen":"green"
		}else if(markerModel.type == 2){
			return (markerModel.a || markerModel.b)? "orange":"red"
		}else if(markerModel.type == 3){
			return (markerModel.a || markerModel.b)? "lightblue":"blue"
		}else if(markerModel.type == 15){
			return (markerModel.a || markerModel.b)? "#f486db":"#ff00f0"
		}else if(markerModel.a || markerModel.b){
			return "yellow"
		}
		return "gray"
	}
	
	color: getmarkerModelColor()
	
	Menu { 
		id: contextMenu
		MenuItem {
			text: qsTr('Delete')
			onTriggered:{
				if(currentAnimation) {
					if(isSelected) root.deselected(root)
					currentAnimation.removeMarker(markerModel)
				}
			}
		}
	}
	
	MouseArea{
		id:mouseArea
		hoverEnabled: true 
		anchors.fill: parent
		
		cursorShape:Qt.SizeHorCursor
		
		drag.target: parent
		drag.axis: Drag.XAxis
		drag.minimumX: 0
		drag.maximumX: 5000
		
		acceptedButtons: Qt.LeftButton | Qt.RightButton
		
		
		onClicked:{
			if(mouse.button & Qt.LeftButton){
				if(isSelected)
					root.deselected(root)
				else
					root.selected(root)
			}else if(mouse.button & Qt.RightButton )
				contextMenu.popup();
		}
		
		
		onPressAndHold:{
			mouse.accepted = false  
		}
		
		onReleased:{
			if(currentAnimation && markerModel){
				currentAnimation.moveMarker(markerModel, parent.x/getTrackScale())
			}
		}
		
		//TooltipArea {text: "Type: "+markerModel.type+" a: "+markerModel.a+" b: "+markerModel.b}

	}
	}