import QtQuick 2.4
import MySpriteData 0.1
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.2

Rectangle { 
	id:root
    SystemPalette { id: activePalette }
    color: activePalette.dark
	
	Connections{
		target:spritedata
		onColorTableChanged: {colorRepeater.model = 0; colorRepeater.model = spritedata.colorsSize; }
		onCurrentColorTableChanged: {colorRepeater.model = 0; colorRepeater.model = spritedata.colorsSize; }
		onRefresh: {colorRepeater.model = 0; colorRepeater.model = spritedata.colorsSize; }
		onAllColorTablesChanged: {colorRepeater.model = 0; colorRepeater.model = spritedata.colorsSize; }
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
			text: qsTr('Add color table set')
			onTriggered:{
				spritedata.addColors(-1,16);
			}
		}
		
		MenuItem {
			text: qsTr('Remove color table set')
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
							width: 2;
							height: 30;
							color: "blue"
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
							
							PGToolTip {
								visible: mouseArea.containsMouse
								text: mouseArea.parent.color
							}
						}
						border.width: mouseArea.containsMouse? 2: 1
						border.color: mouseArea.containsMouse? "red": "black"
					}
				}//reapeater end
			}
		}//scroll view end
	}
}