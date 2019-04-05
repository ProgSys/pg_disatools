import QtQuick 2.4
import MyCutout 0.1
import MySpriteData 0.1
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4

Rectangle {
	property var cutout
	
	function getItemColor(){
		if( spritedata && spritedata.selectedKey && spritedata.selectedKey.cutoutID == model.display){
			if(selected == cutout)
				return "#30FFFFFF"
			else
				if(mouseArea.containsMouse)
					return "#502897c5"
				else
					return "#302897c5"
		}else{
			if(selected == cutout)
				return "#30FFFFFF"
			else
				if(mouseArea.containsMouse)
					return "#50FFFFFF"
				else
					return "transparent"
		}
	}
	
	visible: (cutout)? !cutout.hidden: false
	x: cutout.x*zoom
	y: cutout.y*zoom
	
	width: cutout.width*zoom
	height: cutout.height*zoom
	
	color: getItemColor() //selected == cutout)?  "#30FFFFFF" : ((mouseArea.containsMouse)? "#50FFFFFF" :"transparent")
	border.width: 1
	border.color: "green"
	
	Menu { 
		id: cutoutContextMenu
		MenuItem {
			text: qsTr('Hide')
			onTriggered:{cutout.hidden = true
				spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
			}
		}
		MenuItem {
			text: qsTr('Unhide All')
			onTriggered:{spritedata.unhideAllCutouts()
				spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
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
					(selected == cutout)? selected = 0: selected = cutout 
				}else if (mouse.button == Qt.RightButton){
					cutoutContextMenu.popup();
				}
				
				
			}
	}
	
	/*
	Action {
		id: hideAction
		text: "Hide"
		shortcut: "H"
		onTriggered: { 
			if(selected){
				selected.hidden = true
				spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
			}
		}
		tooltip: "Hide the cutout"
	}
	*/
}
}