import QtQuick 2.4
import MyTimeLine 0.1
import MyKeyframe 0.1
import MyCutout 0.1
import MySpriteData 0.1
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.0

Rectangle {
		property var cutout: 0; //spritedata.getCutout(model.display)
		
		function getItemColor(){
			if( spritedata && spritedata.selectedKey && spritedata.selectedKey.cutoutID == model.display){
				if(spritedata.selected == cutout)
					return "#30FFFFFF"
				else
					if(mouseArea.containsMouse)
						return "#502897c5"
					else
						return "#302897c5"
			}else{
				if(spritedata.selected == cutout)
					return "#30FFFFFF"
				else
					if(mouseArea.containsMouse)
						return "#50FFFFFF"
					else
						return "transparent"
			}
		}
		
		visible: {
			if(!cutout) return false;
			if(spritedata.isolateSelection && spritedata.selected)
				return spritedata.selected == cutout;
			return !cutout.hidden;
		}
		x: cutout.x*zoom
		y: cutout.y*zoom
		
		width: cutout.width*zoom
		height: cutout.height*zoom
		
		color: getItemColor() //spritedata.selected == cutout)?  "#30FFFFFF" : ((mouseArea.containsMouse)? "#50FFFFFF" :"transparent")
		border.width: 1
		border.color: (spritedata.selected == cutout)?"red":"green"
		
	

		Menu { 
			id: cutoutContextMenu
			property var posX: 0;
			property var posY: 0;
			property var cutout: 0;
			MenuItem {
				text: qsTr('Hide')
				onTriggered:{cutout.hidden = true; spritedata.selected = null
					spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
				}
			}
			MenuItem {
				text: qsTr('Unhide All')
				onTriggered:{spritedata.unhideAllCutouts()
					spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
				}
			}
			MenuSeparator { }
			MenuItem {
				text: qsTr('Export as color')
				onTriggered:{
					spritedata.exportSprite(model.display);
				}
			}
			MenuItem {
				text: qsTr('Export as IDs')
				onTriggered:{
					spritedata.exportSpriteIDs(model.display);
				}
			}
			MenuSeparator { }
			MenuItem {
				text: qsTr('Import as color')
				onTriggered:{
					spritedata.importSpriteAsColor(model.display);
					spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
				}
			}
			MenuItem {
				text: qsTr('Import as IDs')
				onTriggered:{
					spritedata.importSpriteAsIDs(model.display);
					spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
				}
			}
			MenuSeparator { }
				MenuItem {
				text: qsTr('Crop')
				onTriggered:{
					spritedata.cropCutout(model.display);
					spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
				}
			}	
			MenuSeparator { }
			
			MenuItem {
				text: qsTr('Add sprite at ('+cutoutContextMenu.posX+", "+cutoutContextMenu.posY+")")
				onTriggered:{
					spritedata.addCutout(activeSpriteSheet,cutoutContextMenu.posX,cutoutContextMenu.posY,cutout.width,cutout.height, cutout.colortable)
					spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
				}
			}
			
			MenuItem {
				text: qsTr('Add sprite right')
				onTriggered:{
					spritedata.addCutout(activeSpriteSheet,cutout.x+cutout.width,cutout.y,cutout.width,cutout.height, cutout.colortable)
					spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
				}
			}
			MenuItem {
				text: qsTr('Add sprite left')
				onTriggered:{
					spritedata.addCutout(activeSpriteSheet,cutout.x-cutout.width,cutout.y,cutout.width,cutout.height, cutout.colortable)
					spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
				}
			}
			
			MenuItem {
				text: qsTr('Add sprite above')
				onTriggered:{
					spritedata.addCutout(activeSpriteSheet,cutout.x,cutout.y-cutout.height,cutout.width,cutout.height, cutout.colortable)
					spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
				}
			}
			MenuItem {
				text: qsTr('Add sprite below')
				onTriggered:{
					spritedata.addCutout(activeSpriteSheet,cutout.x,cutout.y+cutout.height,cutout.width,cutout.height, cutout.colortable)
					spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
				}
			}
			
			MenuItem {
				text: qsTr('Delete')
				iconSource:  "../materials/icons/delete.png"
				onTriggered:{
					spritedata.selected = null
					spritedata.removeCutoutID(model.display);
					
					//spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
				}
			}
		}
		
		Rectangle {
			color: "#80FFFFFF"
			
			anchors.top: parent.top
			anchors.left: parent.left
			anchors.rightMargin: 1
			anchors.topMargin: 1
			
			Text {
				font.pointSize: 7.5
				text: model.display
				color: activePalette.text
			}
			width: childrenRect.width
			height: childrenRect.height
			visible: ((childrenRect.width > parent.width-5) ? false : true)
		}
		
		MouseArea {
			id: mouseArea
			hoverEnabled: true 
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton | Qt.RightButton
			onClicked: { 
					if (mouse.button == Qt.LeftButton){
						if(spritedata.selected == cutout) {spritedata.selected = null;root.cutoutSelected(-1, spritedata.selected); } else {spritedata.selected = cutout; root.cutoutSelected(model.display, spritedata.selected);} 
						
					}else if (mouse.button == Qt.RightButton){
						cutoutContextMenu.posX = Math.round((parent.x + mouse.x )/zoom);
						cutoutContextMenu.posY = Math.round((parent.y + mouse.y)/zoom);
						cutoutContextMenu.cutout = cutout;
						cutoutContextMenu.popup();
					}
			}
			
			onPositionChanged: {
				mousePosIndicator.mouseX = parent.x + mouse.x -scroll.flickableItem.contentX
				mousePosIndicator.mouseY = parent.y + mouse.y + 25 -scroll.flickableItem.contentY
				mousePosIndicator.visible = true
			}
		}
		
		//move drag
		MouseArea {
			anchors.top: parent.top
			anchors.left: parent.left
			width:2;
			height:2;
			hoverEnabled: true 
			cursorShape:Qt.SizeAllCursor
			drag.target: parent
			drag.axis: Drag.XAndYAxis
			drag.minimumX: 0
			drag.maximumX: 5000
			drag.minimumY: 0
			drag.maximumY: 5000
			
			onPressed:{
				if(spritedata.selected != cutout) {spritedata.selected = cutout; root.cutoutSelected(model.display, spritedata.selected);} 
			}
				
			onReleased:{
				cutout.x = Math.round(parent.x/zoom);
				cutout.y = Math.round(parent.y/zoom);
				
				var sheet = spritedata.getSpriteSheet(activeSpriteSheet);
				if(cutout.x >= sheet.width) cutout.x = sheet.width-1;
				if(cutout.y >= sheet.width) cutout.y = sheet.width-1;
				spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
			}
			
			onPositionChanged: {
				mousePosIndicator.mouseX = parent.x -scroll.flickableItem.contentX
				mousePosIndicator.mouseY = parent.y  + 25 -scroll.flickableItem.contentY
				mousePosIndicator.visible = true
			}
		}
		
		//resize drag
		Rectangle{
			x: parent.width-2;
			y: parent.height-2;
			width:2;
			height:2;
			border.color: "lightgreen"
			MouseArea {
				anchors.fill: parent
				hoverEnabled: true 
				cursorShape:Qt.SizeFDiagCursor
				drag.target: parent
				drag.axis: Drag.XAndYAxis
				drag.minimumX: 1
				drag.maximumX: 5000
				drag.minimumY: 1
				drag.maximumY: 5000
				
				onPressed:{
				   if(spritedata.selected != cutout) {spritedata.selected = cutout; root.cutoutSelected(model.display, spritedata.selected);} 
				}
				
				onReleased:{
					cutout.width = ((parent.x+2)/zoom);
					cutout.height = ((parent.y+2)/zoom);
					parent.x = parent.parent.width-2;
					parent.y = parent.parent.height-2;
					spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
				}
				
				onPositionChanged: {
					cutout.width = ((parent.x+2)/zoom);
					cutout.height = ((parent.y+2)/zoom);
					mousePosIndicator.mouseX = parent.parent.x + parent.x + 2 -scroll.flickableItem.contentX
					mousePosIndicator.mouseY = parent.parent.y + parent.y + 2 + 25 -scroll.flickableItem.contentY
					mousePosIndicator.visible = true
				}
			}
		}
	}