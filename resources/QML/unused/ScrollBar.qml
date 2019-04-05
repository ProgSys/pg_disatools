import QtQuick 2.0

Item {
   id: scrollbar

    property Flickable flickable: undefined
    property int basicHeight: 10
	property int scrollBarMinWidth: 10
    property int expandedWidth: 20
    property alias color : scrl.color
   
	height: basicHeight
	anchors.bottom: flickable.bottom;
    anchors.left: flickable.left;
	anchors.right: flickable.right;
	
	clip: true
    visible: flickable.visible
    z:1
	
	Binding {
        target: scrollbar
        property: "width"
        value: expandedWidth
        when: ma.drag.active || ma.containsMouse
    }
    Behavior on width {NumberAnimation {duration: 150}}
   
	//background
	Rectangle {
		anchors.fill: parent
		color: "#f8f8f8"
	}
   
    Rectangle {
        id: scrl
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		clip: true
        
        width: (( flickable.visibleArea.widthRatio * flickable.width) < scrollBarMinWidth)? scrollBarMinWidth: flickable.visibleArea.widthRatio * flickable.width
        visible: flickable.visibleArea.widthRatio < 1.0
        color: "#cdcdcd"

		Binding {
            target: scrl
            property: "x"
            value: !isNaN(flickable.visibleArea.widthRatio) ? (ma.drag.maximumX * flickable.contentX) / (flickable.contentWidth * (1 - flickable.visibleArea.widthRatio)) : 0
            when: !ma.drag.active
        }

        Binding {
            target: flickable
            property: "contentX"
            value: ((flickable.contentWidth * (1 - flickable.visibleArea.widthRatio)) * scrl.x) / ma.drag.maximumX
            when: ma.drag.active && flickable !== undefined
        }

		
		MouseArea {
            id: ma
            anchors.fill: parent
            hoverEnabled: true
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: 0
            drag.maximumX: flickable.width - scrl.width
            preventStealing: true
			
			onEntered:{
				color = "#a6a6a6"
			}
			
			onExited:{
				color = "#cdcdcd"
			}
        }
		
    }
}