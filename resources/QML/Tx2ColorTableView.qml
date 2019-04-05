import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.2
import MyTX2Editor 0.1

Rectangle { 
	id:root
    SystemPalette { id: activePalette }
    color: activePalette.dark
	
	Connections{
		target:tx2editor
		onImageChanged: {colorRepeater.model = 0; colorRepeater.model = tx2editor.numberOfColors; }
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
			tx2editor.setColor(colorDialog.color, colorIndex);
		}
	}

	Menu { 
		id: colorContextMenu
		property int index: -1;
		
		MenuItem {
			text: qsTr('Export color table')
			onTriggered:{
				tx2editor.exportColorTable(tx2editor.currColorTable);
			}
		}
		
		MenuItem {
			text: qsTr('Import color table')
			onTriggered:{
				tx2editor.importColorTable(tx2editor.currColorTable);
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
					if(tx2editor.currColorTable == 0)
						tx2editor.currColorTable = tx2editor.numberOfColorTables-1;
					else
						tx2editor.currColorTable--;
				}
			}
			
			Text{
				font.pointSize: 12
				text: (tx2editor.currColorTable+1)+"/"+tx2editor.numberOfColorTables
			}
			
			Button {
				height: 24
				width: 24
				iconSource: "../materials/icons/item_next.png"
				onClicked: {
					if(tx2editor.currColorTable == tx2editor.numberOfColorTables-1)
						tx2editor.currColorTable = 0;
					else
						tx2editor.currColorTable++;
				}
			}
			
			Button {
				height: 24
				width: 24
				iconSource: "../materials/icons/add.png"
				onClicked: {
					tx2editor.addColorTable(tx2editor.currColorTable);
					tx2editor.currColorTable++;
				}
			}
			
			Button {
				height: 24
				width: 24
				iconSource: "../materials/icons/duplicateColorTable.png"
				onClicked: {
					tx2editor.addColorTable(tx2editor.currColorTable, true);
					tx2editor.currColorTable++;
				}
			}
			
			Button {
				height: 24
				width: 24
				iconSource: "../materials/icons/delete.png"
				onClicked: {
					tx2editor.removeColorTable(tx2editor.currColorTable);
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
					model: tx2editor.numberOfColors
					delegate: Rectangle {
						width: 30;
						height: 30;
						color: tx2editor.getColor(index)

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
						}
						border.width: mouseArea.containsMouse? 2: 1
						border.color: mouseArea.containsMouse? "red": "black"
					}
				}//reapeater end
			}
		}//scroll view end
	}
}