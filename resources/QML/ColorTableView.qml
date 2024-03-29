import QtQuick 2.4
import MySpriteData 0.1
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.2

Rectangle { 
	id:root
	
	property var hoverIndex: -1
	
    SystemPalette { id: activePalette }
    color: activePalette.dark
	
	Connections{
		target:spritedata
		function onColorTableChanged() {colorRepeater.model = 0; colorRepeater.model = spritedata.colorsSize; }
		function onCurrentColorTableChanged() {colorRepeater.model = 0; colorRepeater.model = spritedata.colorsSize; }
		function onRefresh() {colorRepeater.model = 0; colorRepeater.model = spritedata.colorsSize; }
		function onAllColorTablesChanged() {colorRepeater.model = 0; colorRepeater.model = spritedata.colorsSize; }
	}
	
	
	Image {
		anchors.fill: parent
		fillMode: Image.Tile
		source: "../pattern.png"
	}
	
	ColorDialog {
		property int colorIndex: 0
		id: colorDialog
		title: "Please choose a color"
		showAlphaChannel: true
		onAccepted: {
			console.log("You chose: " + colorDialog.color)
			spritedata.setColor(colorIndex, colorDialog.color);
			//Qt.quit()
		}
		//Component.onCompleted: visible = true
	}

	Menu { 
		id: colorContextMenu
		property int index: -1;
		MenuItem {
			text: qsTr('Add set')
			onTriggered:{
				spritedata.addColors(colorContextMenu.index-colorContextMenu.index%16,16, true);
			}
		}
		
		MenuItem {
			text: qsTr('Remove set')
			visible: spritedata.colorsSize > 16
			onTriggered:{
				spritedata.removeColors(colorContextMenu.index-colorContextMenu.index%16,16);
			}
		}
		
		MenuSeparator { }
		
		MenuItem {
			text: qsTr('Export color table')
			onTriggered:{
				spritedata.exportColortable(spritedata.colorTable);
			}
		}
		
		MenuItem {
			text: qsTr('Import color table')
			onTriggered:{
				spritedata.importColortable(spritedata.colorTable);
			}
		}
		
		MenuSeparator { }
		MenuItem {
			text: qsTr('Copy set')
			onTriggered:{
				spritedata.exportColorsToClipboard(colorContextMenu.index-colorContextMenu.index%16, 16, spritedata.colorTable);
			}
		}
		
		MenuItem {
			text: qsTr('Paste set')
			onTriggered:{
				spritedata.importColorsFromClipboard(colorContextMenu.index-colorContextMenu.index%16, spritedata.colorTable);
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
				iconSource: "../materials/icons/item_previous.png"
				onClicked: {
					if(spritedata.colorTable == 0)
						spritedata.colorTable = spritedata.colorTablesNumber-1;
					else
						spritedata.colorTable--;
				}
				PGToolTip {
					text: "Previous"
				}
			}
			
			Text{
				font.pointSize: 12
				text: (spritedata.colorTable+1)+"/"+spritedata.colorTablesNumber
			}
			
			Button {
				height: 24
				width: 24
				iconSource: "../materials/icons/item_next.png"
				onClicked: {
					if(spritedata.colorTable == spritedata.colorTablesNumber-1)
						spritedata.colorTable = 0;
					else
						spritedata.colorTable++;
				}
				PGToolTip {
					text: "Next"
				}
			}
			
			Button {
				height: 24
				width: 24
				iconSource: "../materials/icons/add.png"
				onClicked: {
					spritedata.insertColorTable(spritedata.colorTable);
				}
				PGToolTip {
					text: "Add empty colortable"
				}
			}
			
			Button {
				height: 24
				width: 24
				iconSource: "../materials/icons/duplicateColorTable.png"
				onClicked: {
					spritedata.insertColorTable(spritedata.colorTable, true);
				}
				PGToolTip {
					text: "Duplicate color table"
				}
			}
			
			Button {
				height: 24
				width: 24
				iconSource: "../materials/icons/delete.png"
				visible: spritedata.colorTablesNumber > 1
				onClicked: {
					spritedata.removeColorTable(spritedata.colorTable);
				}
				PGToolTip {
					text: "Delete color table"
				}
			}
			
			Button {
				height: 24
				width: 24
				iconSource: "../materials/icons/add.png"
				onClicked: {
					spritedata.addColors(-1,16, false);
				}
				PGToolTip {
					text: "Add set"
				}
			}
		}
	}

	Item {
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.topMargin: 25
		
		
		ScrollView {
		anchors.fill: parent
			GridLayout  {
				columns:  (root.width-20)/30
				rowSpacing: 1
				columnSpacing: 0
				Repeater {
					id: colorRepeater
					model: spritedata.colorsSize
					delegate: Rectangle {
						width: 30;
						height: 30;
						color: spritedata.getColor(index)

						Rectangle {
							anchors.top: parent.top
							anchors.left: parent.left
							x:-1;
							width: 5;
							height: 30;
							color: Math.floor(hoverIndex/16) * 16 == index? "red": "white"
							border.width: 1
							border.color: "black"
							visible: (index % 16) == 0
							
						}
						
						Rectangle {
							color: "#80FFFFFF"
							
							anchors.top: parent.top
							anchors.left: parent.left
							anchors.rightMargin: 1
							anchors.topMargin: 1
							
							Text {
								font.pointSize: 7.5
								text: index
								color: activePalette.text
							}
							width: childrenRect.width
							height: childrenRect.height
						}
						
						MouseArea {
							id: mouseArea
							hoverEnabled: true 
							anchors.fill: parent
							acceptedButtons: Qt.LeftButton | Qt.RightButton
							onClicked: { 
								if(mouse.button & Qt.LeftButton){
									colorDialog.color = parent.color;
									colorDialog.colorIndex = index;
									colorDialog.open();
								}else{
									colorContextMenu.index = index;
									colorContextMenu.popup();
									}
							}
							
							onContainsMouseChanged:{
								if(containsMouse){
									spritedata.selectedColorId = index % 16
									hoverIndex = index
								}else{
									spritedata.selectedColorId = -1
									hoverIndex = -1
								}
							}
							
							PGToolTip {
								visible: mouseArea.containsMouse
								text: mouseArea.parent.color
							}
						}
						border.width: mouseArea.containsMouse? 2: 1
						border.color: {
							if( mouseArea.containsMouse) return "red";
							if( Math.floor(hoverIndex/16)  == Math.floor(index/16)) return "#FAFAFA"
							return "black";
						}
					}
				}//reapeater end
			}
		}//scroll view end
	}
}