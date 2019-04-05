import QtQuick 2.4;
import MyTimeLine 0.1
import MyKeyframe 0.1
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4

Rectangle { 
	id: root
    SystemPalette { id: activePalette }
    color: activePalette.dark
	//border.width: 1
	//border.color:"black"
	
	function getTracksWidth() {
		return timeline.width*((timeline.steps*4)/10)+200
	}
	
	Row {
		//Rectangle {
		//	width: 100
		//	height: 20
		//	color: activePalette.dark
		//}
		
		anchors.top: parent.top
		Column {
			//Layer  {
				//layername: "Test Name"
			//}
			
			Flickable {
				interactive: false
				contentX: scrollView.flickableItem.contentX
				//contentY: scrollView.flickableItem.contentY
				width: root.width 
				height:  ruler.height
				contentWidth: (getTracksWidth() < root.width)? root.width : getTracksWidth()
				//contentHeight: 500
				
				Item{
				
					Ruler{
						id: ruler
						width: (getTracksWidth() < root.width)? root.width : getTracksWidth()  //timeline.width*((timeline.steps*4)/10)+200
						height: 20
						stepSize: timeline.steps
						timeScale: timeline.timeScale
					}
					
					MouseArea {
						width: (getTracksWidth() < root.width)? root.width : getTracksWidth() 
						height: 20
						onPositionChanged:{
							var key = ((mouse.x/((timeline.steps*4)/10))).toFixed(0)
							
							if(key < 0) key = 0
							if(key > timeline.width) key = timeline.width

							timeline.tracker = key
						}
						onClicked:{
							var key = ((mouse.x/((timeline.steps*4)/10))).toFixed(0)
							
							if(key < 0) key = 0
							if(key > timeline.width) key = timeline.width
							
							timeline.tracker = key
						}
					}
				
					//tracker
					Rectangle {
						id: trackerbox
						anchors.top: root.top
						width: 5
						height: 20
						color: "#565a65"
						x: -2 
						//z: 0
					}
					
					Binding {
						target: trackerbox
						property: "x"
						value: timeline.tracker*((timeline.steps*4)/10)-2
						when: timeline.trackerChanged
					}
					
					Rectangle {
						color: "#b0FFFFFF"
						anchors.bottom: trackerbox.bottom
						anchors.left: trackerbox.right
						anchors.leftMargin: 1
						anchors.topMargin: 0
							Text { 
								//font.pointSize: 7.5
								text: timeline.tracker 
							}
						width: childrenRect.width
						height: childrenRect.height
					}
				}
			}
			
			
			Rectangle {
				id: layerHeaders
				width: 128
				height: root.height-ruler.height
				color: "red"
			}
			
			ScrollView {
				id: scrollView
				anchors.top: layerHeaders.anchors.top
				anchors.left: layerHeaders.anchors.right
				width: root.width - layerHeaders.width 
				height: root.height-ruler.height
				//Image { source: "img.png" }
				horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOn
				verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn
				Item {
					width: getTracksWidth()
					height: timeline.animation.layerSize * 45
					/*
					Text {
						y:100
						text: getTracksWidth()
					}
					*/
					//every keyframe
					Repeater {
						id: tracklist
						anchors.fill: parent;
						//model: timeline.animation
						model: timeline.animation
						delegate: Rectangle {
							property var displayModel: model.display

							anchors.topMargin: 2
							y:2 + index*41;
							x: 0
							width: displayModel.duration*((timeline.steps*4)/10)
							
							//every layer
							Repeater {
								model: displayModel
								delegate: KeyframeElement {
									keyframeModel: model.display
								}
							}// Repaeter 2 end
							
						}//delegate
					}// Repaeter end
			
					Rectangle {
						anchors.top: parent.top
						//anchors.right: trackerbox.right
						//anchors.bottom: parent.bottom
						width: 1;
						height: 500;
						color: "black"
						z: 1000
						x: trackerbox.x+2//timeline.tracker*((timeline.steps*4)/10)
					}
				} // Item end
			}
		}// Column end
	}
}