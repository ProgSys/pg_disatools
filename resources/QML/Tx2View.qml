import QtQuick 2.4
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.0
import MyTX2Editor 0.1

Rectangle { 
	
    SystemPalette { id: activePalette }
    color: activePalette.dark
	
	property var zoom: 1.0

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
		target:tx2editor
		function onImageChanged() {tx2image.source = ""; tx2image.source = "image://tx2imageprovider/img"}
	}
	
	Menu { 
		id: contextMenu
		MenuItem {
			text: qsTr('Zoom In')
			onTriggered:{
				zoom = zoom+0.2;
			}
		}
		
		MenuItem {
			text: qsTr('Zoom Out')
			onTriggered:{
				zoom = zoom-0.2;
				if(zoom < 0.2)
					zoom = 0.2;
			}
		}
		
		MenuItem {
			text: qsTr('Reset zoom')
			onTriggered:{
				zoom = 1
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


	ScrollView {
		id: scroll
		anchors.fill: parent
		clip:false
		Item {
			Item {
				Image {
					id: tx2image
					anchors.fill: parent
					fillMode: Image.PreserveAspectFit
					cache: false
					//smooth: false
					source: "image://tx2imageprovider/img"
					//source: "../disgaea-d2-artwork-3.jpg"
				}
				width: tx2image.sourceSize.width * zoom;
				height: tx2image.sourceSize.height * zoom;
				
			}
			
			width: childrenRect.width+100
			height: childrenRect.height+100
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
				text: Math.round((scroll.flickableItem.contentX +parent.mouseX)/zoom) +":"+ Math.round((scroll.flickableItem.contentY +parent.mouseY)/zoom)
			}
			width: childrenRect.width
			height: childrenRect.height
			visible: false
	}

}