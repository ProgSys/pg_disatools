import QtQuick 2.4
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import MySpriteData 0.1

Rectangle {
	id:root
	property var keyframeModel
	property var layerModel
	property int elementHeight: 40
	property int elementType: 0
	property bool isSelected: false
	
		
	function getTrackScale(){
		return ((timeline.steps*4)/10)
	}
	
	SystemPalette { id: activePalette }
	
	signal selected(var item)
	signal deselected(var item)
	
	height: elementHeight
	width: keyframeModel.duration*getTrackScale()
	x: keyframeModel.start*getTrackScale()
	anchors.topMargin: 1
	//x: timeline.getOffset(index)*((timeline.steps*4)/10)
	
	border.width: 1
	
	border.color:  isSelected? "red" : mouseArea.containsMouse ? "green": "black" 
	color: isSelected? "#2897c5" : mouseArea.containsMouse ? "#a3d3ff": "white"
	
	Image {
		id: imagepreview
		visible: ((parent.width > 20) ? true : false)
		//visible: ((keyframeModel.duration < 4) ? false : true)
		anchors.left: parent.left
		anchors.topMargin: 1
		anchors.leftMargin: 1
		y:1
		height: parent.height-2
		width: parent.width-1
		horizontalAlignment: Image.AlignLeft
		fillMode: Image.PreserveAspectFit
		cache: false
		smooth: false
		source: "image://previewprovider/"+keyframeModel.cutoutID+"_"+keyframeModel.colortableID//keyframeModel.image //"../resources/test.jpg"
	}
	/*
	Connections {
		target: thePublisherObjectExposedFromC++
		onNewImage: imagepreview.setImage(image)
	}
	*/
	
	Rectangle {
		
		color: "#80FFFFFF"
		
		anchors.top: parent.top
		anchors.right: parent.right
		anchors.rightMargin: 4
		anchors.topMargin: 1
		
		Text {
			font.pointSize: 7.5
			text: keyframeModel.duration
			color: activePalette.text
		}
		width: childrenRect.width
		height: childrenRect.height
		visible: ((childrenRect.width > parent.width-5) ? false : true)
	}

	Menu { 
		id: contextMenu
		property int frame: 0
		MenuItem {
			text: qsTr('Split at '+contextMenu.frame)
			onTriggered:{
				if(layerModel) { layerModel.splitKeyframe(contextMenu.frame);timeline.updateTimeline(); }
			}
		}
		
		MenuItem {
			text: qsTr('Swap next')
			visible: keyframeModel.hasNext
			onTriggered:{
				if(keyframeModel) { keyframeModel.swapNext() }
			}
		}
		
		MenuItem {
			text: qsTr('Swap previous ')
			visible: keyframeModel.hasPrevious
			onTriggered:{
				if(keyframeModel) { keyframeModel.swapPrevious() }
			}
		}
		MenuSeparator { }
		MenuItem {
			text: qsTr('Delete')
			onTriggered:{
				if(layerModel) { layerModel.removeKeyframe(keyframeModel); timeline.updateTimeline(); }
			}
		}
	}
	
	MouseArea {
		id: mouseArea
		hoverEnabled: true 
		anchors.fill: parent
		acceptedButtons: Qt.LeftButton | Qt.RightButton
		
		onClicked: { 
			if(mouse.button & Qt.LeftButton){
				if(isSelected)
					root.deselected(root)
				else
					root.selected(root)
					
			}else if(mouse.button & Qt.RightButton ){
				contextMenu.frame =  keyframeModel.start + (mouse.x)/getTrackScale()
				contextMenu.popup();
			}
				//spritedata.selectToggle(keyframeModel, true);
				//timeline.updateTimeline();
		}
	}
	
	Rectangle {
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		
		anchors.topMargin: 5;
		anchors.bottomMargin: 5;
		
		height: 30
		width: (parent.width > 6)? 3 : 2
		//x: parent.width - width
		color: (mouseAreaLeft.containsMouse || mouseAreaLeft.drag.active )? "lightgreen": "transparent"
		
		MouseArea {
			id:mouseAreaLeft
			hoverEnabled: true 
			anchors.fill: parent
			cursorShape:Qt.SizeHorCursor
			
			drag.target: parent
			drag.axis: Drag.XAxis
			drag.minimumX: -5000
			drag.maximumX: 5000
			
			onReleased:{
				//keyframeModel.start += parent.x/((timeline.steps*4)/10)
				keyframeModel.setStart( keyframeModel.start+(parent.x/((timeline.steps*4)/10) ), snapMove)
				parent.x = 0
				rightTracker.x = parent.parent.width - rightTracker.width
				timeline.updateTimeline();
			}
		}
		
		Rectangle {
			color: "#FAFFFFFF"
			anchors.left: parent.right
			anchors.top: parent.top
			Text {
				text: keyframeModel.start + Math.floor(parent.parent.x/((timeline.steps*4)/10))
			}
			width: childrenRect.width
			height: childrenRect.height
			visible: (mouseAreaLeft.containsMouse || mouseAreaLeft.drag.active )
		}
	}
	
	
	Rectangle {
		id: rightTracker
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		
		anchors.topMargin: 5;
		anchors.bottomMargin: 5;
		
		
		height: 30
		width: (parent.width > 6)? 3 : 2
		x: parent.width - width
		color: (mouseAreaRight.containsMouse || mouseAreaRight.drag.active )? "red": "transparent"
		
		MouseArea {
			id:mouseAreaRight
			hoverEnabled: true 
			anchors.fill: parent
			cursorShape:Qt.SizeHorCursor
			
			drag.target: parent
			drag.axis: Drag.XAxis
			drag.minimumX: 1
			drag.maximumX: 5000
			
			onReleased:{
				//keyframeModel.duration = (parent.x+parent.width)/((timeline.steps*4)/10)
				keyframeModel.setDuration((parent.x+parent.width)/((timeline.steps*4)/10), snapMove)
				timeline.updateTimeline();
				//parent.x = parent.parent.width - parent.width
			}
		}
		
		Rectangle {
			color: "#FAFFFFFF"
			anchors.right: parent.left
			anchors.top: parent.top
			Text {
				text: keyframeModel.start + Math.floor((parent.parent.x+parent.parent.width)/((timeline.steps*4)/10))
			}
			width: childrenRect.width
			height: childrenRect.height
			visible: (mouseAreaRight.containsMouse || mouseAreaRight.drag.active )
		}
	}
	
	Connections{
		target: keyframeModel
		onOnDurationChanged:{
			rightTracker.x = width - rightTracker.width
		}
	}

}