import QtQuick 2.4
import MyTimeLine 0.1
import MyKeyframe 0.1
import MyCutout 0.1
import MySpriteData 0.1
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.0

Rectangle { 
	
    SystemPalette { id: activePalette }
    color: activePalette.dark
	
	property int activeSpriteSheet: 0
	property var zoom: 2.0
	property var selected: 0

	function getAbsolutePosition(node) {
      var returnPos = {};
      returnPos.x = 0;
      returnPos.y = 0;
      if(node !== undefined && node !== null) {
          var parentValue = getAbsolutePosition(node.parent);
          returnPos.x = parentValue.x + node.x;
          returnPos.y = parentValue.y + node.y;
      }
      return returnPos;
	}
	
	Connections{
		target:spritedata
		onColortableChanged: {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
	}
	
  	Connections{
		target: spritedata.getSpriteSheet(activeSpriteSheet)
		onOnNumberOfCutoutsChanged: {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
	}
	
	Menu { 
		id: contextMenu
		property var posX: 0
		property var posY: 0
		MenuItem {
			text: qsTr('Unhide All')
			onTriggered:{spritedata.unhideAllCutouts()
				spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
			}
		}
		
		MenuItem {
			text: qsTr('Add sprite')
			onTriggered:{
				spritedata.addCutout(activeSpriteSheet,parent.posX,parent.posY,20,20)
				spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
			}
		}
	}

		
	Image {
		anchors.fill: parent
		fillMode: Image.Tile
		source: "../pattern.png"
		
	
		MouseArea {
			anchors.fill: parent
			hoverEnabled: true
			acceptedButtons: Qt.RightButton
			onClicked: { 
					contextMenu.posX = mouse.x;
					contextMenu.posY = mouse.y;
					contextMenu.popup();
				}
				
			onPositionChanged: {
				mousePosIndicator.mouseX = mouse.x
				mousePosIndicator.mouseY = mouse.y
				mousePosIndicator.visible = true
			}
			
			onExited:{
				mousePosIndicator.visible = false
			}
		}
	}

	Item{
		id: topmenu
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		width: 25
		z: 1000
		
		Row{
			spacing: 2
			Button {
				height: 24
				width: 24
				text: "<"
				onClicked: {
					if(spritedata.sheetsSize){
						(activeSpriteSheet > 0)? activeSpriteSheet-- :  activeSpriteSheet = spritedata.sheetsSize-1 ;
						selected = 0
						spriteimage.source = "image://imageprovider/"+activeSpriteSheet
						}
				}
			}
			
			Text{
				font.pointSize: 12
				text: activeSpriteSheet+"/"+spritedata.sheetsSize
			}
			
			Button {
				height: 24
				width: 24
				text: ">"
				onClicked: {
					if(spritedata.sheetsSize){
							(activeSpriteSheet < spritedata.sheetsSize-1 )? activeSpriteSheet++ :  activeSpriteSheet = 0 ;
							selected = 0
							spriteimage.source = "image://imageprovider/"+activeSpriteSheet
						}
				}
			}
			
			Button {
				height: 24
				width: 24
				text: "+"
				onClicked: {
					(zoom >= 4)? zoom = 4 : zoom += 0.5
				}
			}
			
			Button {
				height: 24
				width: 24
				text: "-"
				onClicked: {
					(zoom <= 0.5)? zoom = 0.5 : zoom -= 0.5
				}
			}
			
			Item{ width: 20; height: 1 }
			
			Button {
				height: 24
				width: 24
				text: ""
				iconSource: "../materials/icons/newcutout.png"
				onClicked: {
					spritedata.addCutout(activeSpriteSheet);
				}
			}
			
			Button {
				height: 24
				width: 24
				text: ""
				iconSource: "../materials/icons/newspritesheet.png"
				onClicked: {
					spritedata.addNewSpriteSheet();
				}
			}
			
			Button {
				height: 24
				width: 24
				text: ""
				iconSource: "../materials/icons/settings.png"
				onClicked: {
					spritedata.editSpriteSheet(activeSpriteSheet);
				}
			}
			
			Item{ width: 20; height: 1 }
			Text{
				text: spritedata.sheetsSize? ("width: "+spritedata.getSpriteSheet(activeSpriteSheet).width+" height: "+spritedata.getSpriteSheet(activeSpriteSheet).height +" colors: "+spritedata.getSpriteSheet(activeSpriteSheet).colors ) : ""
			}
		}
	}


	Item {
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.topMargin: 25
		anchors.bottomMargin: 25
		
		visible: spritedata.sheetsSize != 0 && activeSpriteSheet < spritedata.sheetsSize 
		
		ScrollView {
			id: scroll
			anchors.fill: parent
			clip:false
			Item {
				Item {
					Image {
						id: spriteimage
						anchors.fill: parent
						fillMode: Image.PreserveAspectFit
						cache: false
						smooth: false
						//source: "image://imageprovider/"+activeSpriteSheet+"?"+spritedata.sheetsSize
						source: ""
						
						Connections {
							target: spritedata
							onOnNumberOfSheetsChanged: { spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
						}
							
						
						Repeater {
							model: spritedata.sheetsSize? spritedata.getSpriteSheet(activeSpriteSheet): 0
							delegate: Rectangle {
								property var cutout: spritedata.getCutout(model.display)
								
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
								border.color: (selected == cutout)?"red":"green"
								
							

								Menu { 
									id: cutoutContextMenu
									MenuItem {
										text: qsTr('Hide')
										onTriggered:{cutout.hidden = true; selected = 0
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
										text: qsTr('Delete')
										onTriggered:{
											selected = 0
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
												(selected == cutout)? selected = 0: selected = cutout 
											}else if (mouse.button == Qt.RightButton){
												cutoutContextMenu.popup();
											}
									}
									
									onPositionChanged: {
										mousePosIndicator.mouseX = parent.x + mouse.x -scroll.flickableItem.contentX
										mousePosIndicator.mouseY = parent.y + mouse.y + 25 -scroll.flickableItem.contentY
										mousePosIndicator.visible = true
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
					}
					width: (spritedata.sheetsSize > activeSpriteSheet)? spritedata.getSpriteSheet(activeSpriteSheet).width*zoom: 0
					height: (spritedata.sheetsSize > activeSpriteSheet)? spritedata.getSpriteSheet(activeSpriteSheet).height*zoom: 0
					
				}
				
				width: childrenRect.width+100
				height: childrenRect.height+100
			}	
		}
	}
	
	Rectangle{
		id: bottommenu
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.bottom: parent.bottom
		height: 25
		color: activePalette.alternateBase
		
		CustomBorder
        {
            commonBorder: false
            lBorderwidth: 0
            rBorderwidth: 0
            tBorderwidth: 1
            bBorderwidth: 0
            borderColor: "black"
        }
		
		Row{
			spacing: 2
			//collortableID
			Image{source: "../materials/icons/colortable.png"
				TooltipArea {text: "Colortable ID"}
			}
			TextField {
					width: 30; placeholderText: qsTr("ID")
					validator: IntValidator {bottom: 0; top: 9999;}
					text: (selected)? selected.colortable: ""
					
					onEditingFinished: {
						if(selected && selected.colortable != text) {
							selected.colortable = (text >= spritedata.colortableSize)? spritedata.colortableSize-1: text ; 
							spriteimage.source = "";
							spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
							spritedata.refresh();
						}
					}
			}
			
			//position
			Image{source: "../materials/icons/position.png"
				TooltipArea {text: "Position"}
			}
			TextField {
					width: 40; placeholderText: qsTr("x")
					validator: IntValidator {bottom: -9999; top: 9999;}
					text: (selected)? selected.x: ""
					
					onEditingFinished: { 
						if(selected) {
							selected.x = text ; spritedata.refresh();
							spriteimage.source = "";
							spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
						}
					}
			}
			TextField {
					width: 40; placeholderText: qsTr("y")
					validator: IntValidator {bottom: -9999; top: 9999;}
					text: (selected)? selected.y: ""
					
					onEditingFinished: {
						if(selected) {
							selected.y = text ; spritedata.refresh()
							spriteimage.source = "";
							spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
						}
					}
			}
			
			
			//Offset
			Image{source: "../materials/icons/size.png"
				TooltipArea {text: "Size"}
			}
			TextField {
					width: 40; placeholderText: qsTr("Width")
					validator: IntValidator {bottom: 0; top: 9999;}
					text: (selected)? selected.width: ""
					onEditingFinished: { 
						if(selected) {
							selected.width = text ; spritedata.refresh()
							spriteimage.source = "";
							spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
						}
					}
			}
			TextField {
					width: 40; placeholderText: qsTr("Height")
					validator: IntValidator {bottom: 0; top: 9999;}
					text: (selected)? selected.height: ""
					onEditingFinished: { 
						if(selected) {
							selected.height = text ; spritedata.refresh()
							spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
						}
					}
			}
		}
	}
	
	Rectangle {
			id: mousePosIndicator
			color: "#FAFFFFFF"
			property int mouseX: 0
			property int mouseY: 0
			x: mouseX+15
			y: mouseY+15
			Text {
				text: Math.round((scroll.flickableItem.contentX +parent.mouseX)/zoom) +":"+ Math.round((scroll.flickableItem.contentY +parent.mouseY-25)/zoom)
			}
			width: childrenRect.width
			height: childrenRect.height
			visible: false
	}

}