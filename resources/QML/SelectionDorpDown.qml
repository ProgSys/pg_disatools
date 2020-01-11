import QtQuick 2.4
import QtQuick.Controls 2.12

Button {
	property alias model: listView.model
	property var selectedId: -1
	
	signal selected(var value);
	
	height: 20
	width:20
	text: "^"
	checked: popup.opened
	onClicked: popup.opened? popup.close(): popup.open()
	enabled: model && model.size > 0

	Popup {
        id: popup
        width: 20
        height: model? Math.min(root.height-100, model.size*20) : 0
		
		x: Math.round((parent.width - width))
        y: Math.round((parent.height - height - 20))
		
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

		Overlay.modal: Rectangle {
			 color: "transparent"
		}
	
		contentItem: ListView {
			id: listView
			clip:true
			
			anchors.fill: parent
			anchors.topMargin: 1
			delegate:  Rectangle {
				width: 20;
				height: 20;
				border.color: selectedId == display? "red": mouseArea.containsMouse? "#2897C5": "black"
				color: selectedId == display? "#C52828"  : mouseArea.containsMouse? "white": "#E6E6E6"
				Text {
					anchors.horizontalCenter: parent.horizontalCenter 
					anchors.verticalCenter: parent.verticalCenter 
					text: display
				}
				MouseArea{
					id: mouseArea
					anchors.fill: parent
					hoverEnabled: true
					onClicked: {selected(display); popup.close();}
				}
			}
		}
    }
}