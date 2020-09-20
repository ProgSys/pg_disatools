import QtQuick 2.15
import QtQuick.Controls 1.4
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
	color: isSelected? "#2897c5" : mouseArea.containsMouse ? "#a3d3ff": (keyframeModel.cutoutID == selectedCutoutID)? "#d7ecff": "white"
	
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
		source: "image://previewprovider/"+keyframeModel.cutoutID+"_"+keyframeModel.colortableID
	}
	
	Connections {
		target: spritedata
		function onColorTableChanged() {imagepreview.source = ""; imagepreview.source = "image://previewprovider/"+keyframeModel.cutoutID+"_"+keyframeModel.colortableID; }
		function onCurrentColorTableChanged(){ imagepreview.source = ""; imagepreview.source = "image://previewprovider/"+keyframeModel.cutoutID+"_"+keyframeModel.colortableID; }
		function onRefresh(){imagepreview.source = ""; imagepreview.source = "image://previewprovider/"+keyframeModel.cutoutID+"_"+keyframeModel.colortableID; }
		function onAllColorTablesChanged() {imagepreview.source = ""; imagepreview.source = "image://previewprovider/"+keyframeModel.cutoutID+"_"+keyframeModel.colortableID; }
	}

	
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
				if(layerModel) { 
					spritedata.pushUndoLayer(layerModel);
					layerModel.splitKeyframe(contextMenu.frame);timeline.updateTimeline();
				}
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
			text: qsTr('Center anchor')
			onTriggered:{
				if(keyframeModel) { 
					keyframeModel.anchorx = -spritedata.getCutout(keyframeModel.cutoutID).width/2;
					keyframeModel.anchory = -spritedata.getCutout(keyframeModel.cutoutID).height/2;
				}
			}
		}
		
		MenuItem {
			text: qsTr('Replace')
			enabled: selectedCutoutID != -1
			onTriggered:{
				if(keyframeModel) { 
					keyframeModel.cutoutID = selectedCutoutID ; keyframeModel.colortableID = selectedCutout.colortable  
				}
			}
		}
		
		MenuItem {
			text: qsTr('Replace fit')
			enabled: selectedCutoutID != -1
			onTriggered:{
				if(keyframeModel) { 
					spritedata.pushUndoLayer(layerModel);
					var oldCutout = spritedata.getCutout(keyframeModel.cutoutID)
					keyframeModel.scalex = 100* (((keyframeModel.scalex/100.0)*oldCutout.width)/selectedCutout.width)
					keyframeModel.scaley = 100* (((keyframeModel.scaley/100.0)*oldCutout.height)/selectedCutout.height)
					keyframeModel.anchorx = (keyframeModel.anchorx/oldCutout.width) * selectedCutout.width;
					keyframeModel.anchory = (keyframeModel.anchory/oldCutout.height) * selectedCutout.height;
					keyframeModel.cutoutID = selectedCutoutID ; keyframeModel.colortableID = selectedCutout.colortable  
				}
			}
		}
		
		Menu {
			title: qsTr('Copy from selected')
			enabled: selectedItem && selectedItem.elementType == 0
			MenuItem {
				text: "All"
				onTriggered:{
					if(selectedItem && selectedItem.elementType == 0) { 
						spritedata.pushUndoLayer(layerModel);
						keyframeModel.cutoutID = selectedItem.keyframeModel.cutoutID;
						keyframeModel.colortableID = selectedItem.keyframeModel.colortable 
						keyframeModel.scalex = selectedItem.keyframeModel.scalex;
						keyframeModel.scaley = selectedItem.keyframeModel.scaley;
						keyframeModel.offsetx = selectedItem.keyframeModel.offsetx;
						keyframeModel.offsety = selectedItem.keyframeModel.offsety
						keyframeModel.anchorx = selectedItem.keyframeModel.anchorx;
						keyframeModel.anchory = selectedItem.keyframeModel.anchory;
						keyframeModel.rotation = selectedItem.keyframeModel.rotation;
						
						keyframeModel.transparency = selectedItem.keyframeModel.transparency;
						keyframeModel.mirroredHorizontally = selectedItem.keyframeModel.mirroredHorizontally;
						keyframeModel.mirroredVertically = selectedItem.keyframeModel.mirroredVertically;
						keyframeModel.adaptive = selectedItem.keyframeModel.adaptive;
					}
				}
			}
			MenuItem {
				text: "All without ID"
				onTriggered:{
					if(selectedItem && selectedItem.elementType == 0) { 
						spritedata.pushUndoLayer(layerModel);
						keyframeModel.scalex = selectedItem.keyframeModel.scalex;
						keyframeModel.scaley = selectedItem.keyframeModel.scaley;
						keyframeModel.offsetx = selectedItem.keyframeModel.offsetx;
						keyframeModel.offsety = selectedItem.keyframeModel.offsety
						keyframeModel.anchorx = selectedItem.keyframeModel.anchorx;
						keyframeModel.anchory = selectedItem.keyframeModel.anchory;
						keyframeModel.rotation = selectedItem.keyframeModel.rotation;
						
						keyframeModel.transparency = selectedItem.keyframeModel.transparency;
						keyframeModel.mirroredHorizontally = selectedItem.keyframeModel.mirroredHorizontally;
						keyframeModel.mirroredVertically = selectedItem.keyframeModel.mirroredVertically;
						keyframeModel.adaptive = selectedItem.keyframeModel.adaptive;
					}
				}
			}
			MenuItem {
				text: "Fit to selected"
				onTriggered:{
					if(selectedItem && selectedItem.elementType == 0) { 
						spritedata.pushUndoLayer(layerModel);
						var selectedCutout = spritedata.getCutout(selectedItem.keyframeModel.cutoutID)
						var currentCutout = spritedata.getCutout(keyframeModel.cutoutID)
						keyframeModel.scalex = selectedItem.keyframeModel.scalex * (selectedCutout.width/currentCutout.width)
						keyframeModel.scaley = selectedItem.keyframeModel.scaley * (selectedCutout.height/currentCutout.height)
					    keyframeModel.anchorx = selectedItem.keyframeModel.anchorx * (currentCutout.width/selectedCutout.width)
						keyframeModel.anchory = selectedItem.keyframeModel.anchory * (currentCutout.height/selectedCutout.height)

						keyframeModel.offsetx = selectedItem.keyframeModel.offsetx;
						keyframeModel.offsety = selectedItem.keyframeModel.offsety
						keyframeModel.rotation = selectedItem.keyframeModel.rotation;
						
						keyframeModel.transparency = selectedItem.keyframeModel.transparency;
						keyframeModel.mirroredHorizontally = selectedItem.keyframeModel.mirroredHorizontally;
						keyframeModel.mirroredVertically = selectedItem.keyframeModel.mirroredVertically;
						keyframeModel.adaptive = selectedItem.keyframeModel.adaptive;
					}
				}
			}
			MenuSeparator { }
			MenuItem {
				text: "Offset"
				onTriggered:{
					if(selectedItem && selectedItem.elementType == 0) { 
						spritedata.pushUndoLayer(layerModel);
						keyframeModel.offsetx = selectedItem.keyframeModel.offsetx;
						keyframeModel.offsety = selectedItem.keyframeModel.offsety
					}
				}
			}
            MenuItem {
				text: "Scale"
				onTriggered:{
					if(selectedItem && selectedItem.elementType == 0) { 
						spritedata.pushUndoLayer(layerModel);
						keyframeModel.scalex = selectedItem.keyframeModel.scalex;
						keyframeModel.scaley = selectedItem.keyframeModel.scaley;
					}
				}
			}
			MenuItem {
				text: "Anchor"
				onTriggered:{
					if(selectedItem && selectedItem.elementType == 0) { 
						spritedata.pushUndoLayer(layerModel);
						keyframeModel.anchorx = selectedItem.keyframeModel.anchorx;
						keyframeModel.anchory = selectedItem.keyframeModel.anchory;
					}
				}
			}
			MenuItem {
				text: "Rotation"
				onTriggered:{
					if(selectedItem && selectedItem.elementType == 0) { 
						spritedata.pushUndoLayer(layerModel);
						keyframeModel.rotation = selectedItem.keyframeModel.rotation;
					}
				}
			}
			MenuItem {
				text: "Mirror"
				onTriggered:{
					if(selectedItem && selectedItem.elementType == 0) { 
						spritedata.pushUndoLayer(layerModel);
						keyframeModel.mirroredHorizontally = selectedItem.keyframeModel.mirroredHorizontally;
						keyframeModel.mirroredVertically = selectedItem.keyframeModel.mirroredVertically;
					}
				}
			}
		}
		
		MenuSeparator { }
		MenuItem {
			iconSource:  "../materials/icons/delete.png"
			text: qsTr('Delete')
			onTriggered:{
				if(layerModel) { 
					spritedata.pushUndoLayer(layerModel);
					var buffer = timeline;
					layerModel.removeKeyframe(keyframeModel); buffer.updateTimeline();
					}
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
			forceActiveFocus();
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
				spritedata.pushUndoLayer(layerModel);
				keyframeModel.setStart( keyframeModel.start+(parent.x/((timeline.steps*4)/10) ), snapMove)
				parent.x = 0
				rightTracker.x = parent.parent.width - rightTracker.width
				timeline.updateTimeline();
				forceActiveFocus();
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
				spritedata.pushUndoLayer(layerModel);
				keyframeModel.setDuration((parent.x+parent.width)/((timeline.steps*4)/10), snapMove)
				timeline.updateTimeline();
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
		function onOnDurationChanged(){ rightTracker.x = width - rightTracker.width; }
	}

}