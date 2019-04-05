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
		onColortableChanged: {colorRepeater.model = 0; colorRepeater.model = spritedata.colorsSize; }
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
						acceptedButtons: Qt.LeftButton 
						onClicked: { 
							colorDialog.color = parent.color;
							colorDialog.colorIndex = index;
							colorDialog.open();
						}
					}
					border.width: mouseArea.containsMouse? 2: 1
					border.color: mouseArea.containsMouse? "red": "black"
				}
			}
		}
	}//scroll view end
	
	}