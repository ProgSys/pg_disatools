import QtQuick 2.4
import MyTimeLine 0.1
import MyKeyframe 0.1
import MyCutout 0.1
import MySpriteData 0.1
import MyMarker 0.1
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4

Rectangle { 

	property var selectedItem: 0
	
	SystemPalette { id: activePalette }
	color: activePalette.dark


	function select(item){
		if(selectedItem){
			selectedItem.isSelected = false
			if(selectedItem.elementType == 0) selectedItem.keyframeModel.selected = false; spritedata.clearSelectedKey(); timeline.updateTimeline();
			
			if(selectedItem == item){
				selectedItem = 0;
				return;
			}
		}
		selectedItem = item;
		if(selectedItem){
			selectedItem.isSelected = true
			if(selectedItem.elementType == 0) selectedItem.keyframeModel.selected = true; spritedata.selectedKey = selectedItem.keyframeModel; timeline.updateTimeline();
			}
	}
	
	function deselect(item){
		if(selectedItem){
			selectedItem.isSelected = false
			if(selectedItem.elementType == 0) selectedItem.keyframeModel.selected = false; spritedata.clearSelectedKey(); timeline.updateTimeline();
			selectedItem = 0;
		}
		if(item){
			item.isSelected = false
			if(item.elementType == 0) selectedItem.keyframeModel.selected = false; spritedata.clearSelectedKey(); timeline.updateTimeline();
		}

	}
	
	Connections{
		target: timeline
		onOnAnimationChanged:{
			deselect(selectedItem)
		}
	}
	
	function getTrackScale(){
		return ((timeline.steps*4)/10)
	}
	
	function getTracksWidth() {
		return timeline.width*getTrackScale()+200
	}
	
	function getWorkspaceWidth() {
		return (timeline.animation)? timeline.animation.workspacelength*getTrackScale()+200 : 0
	}
	
	
	
	
	Rectangle{
		id: controllbar
		height: 22
		
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		
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
		
		//keyframe edit
		Row{
			spacing: 3
			visible: !selectedItem || (selectedItem && selectedItem.elementType == 0)
			//start
			Image{source: "../resources/materials/icons/start.png" 
				TooltipArea {text: "Keyframe starting frame"}
			}
			TextField {
					id: fieldstart
					//readOnly: true
					width: 40; 
					validator: IntValidator {bottom: 0; top: 9999;}
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.start: ""
					
					onEditingFinished: {
						if(selectedItem && selectedItem.elementType == 0) {
							selectedItem.keyframeModel.start = text;
							timeline.updateTimeline();
						}
					}
			}
			//diration
			Image{source: "../resources/materials/icons/duration.png"
				TooltipArea {text: "Keyframe duration"}
			}
			TextField {
					width: 40; 
					validator: IntValidator {bottom: 0; top: 9999;}
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.duration: ""
					
					onEditingFinished: {
						if(selectedItem && selectedItem.elementType == 0) {
							selectedItem.keyframeModel.duration = text
							timeline.updateTimeline();
						}
					}
			}
			
			//end
			Image{source: "../resources/materials/icons/end.png"
				TooltipArea {text: "Keyframe end frame"}
			}
			TextField {
					width: 40;
					validator: IntValidator {bottom: 0; top: 9999;}
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.start+selectedItem.keyframeModel.duration: ""
					
					onEditingFinished: {
						if(selectedItem && selectedItem.elementType == 0) {
							selectedItem.keyframeModel.duration = text-selectedItem.keyframeModel.start
							timeline.updateTimeline();
						}
					}
			}
			
			//spriteID
			Image{source: "../resources/materials/icons/spriteid.png"
				TooltipArea {text: "Sprite ID"}
			}
			TextField {
					width: 30; placeholderText: qsTr("ID")
					validator: IntValidator {bottom: 0; top: 99999;}
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.cutoutID: ""
					
					onEditingFinished: {
						if(selectedItem && selectedItem.elementType == 0) {
							selectedItem.keyframeModel.cutoutID = (text >= spritedata.cutoutSize)? spritedata.cutoutSize-1: text
							timeline.updateTimeline();
						}
					}
			}
			
			//collortableID
			Image{source: "../resources/materials/icons/colortable.png"
				TooltipArea {text: "Colortable ID"}
			}
			TextField {
					width: 30; placeholderText: qsTr("ID")
					validator: IntValidator {bottom: 0; top: 9999;}
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.colortableID: ""
					
					onEditingFinished: {
						if(selectedItem && selectedItem.elementType == 0) {
							selectedItem.keyframeModel.colortableID = (text >= spritedata.colortableSize)? spritedata.colortableSize-1: text
							timeline.updateTimeline();
							}
					}
			}
			
			//anchor
			Image{source: "../resources/materials/icons/anchor.png"
				TooltipArea {text: "Anchor"}
			}
			TextField {
					width: 40; placeholderText: qsTr("x")
					validator: IntValidator {bottom: -9999; top: 9999;}
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.anchorx: ""
					
					onEditingFinished: { selectedItem.keyframeModel.anchorx = text; timeline.updateTimeline();}
			}
			TextField {
					width: 40; placeholderText: qsTr("y")
					validator: IntValidator {bottom: -9999; top: 9999;}
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.anchory: ""
					
					onEditingFinished: { selectedItem.keyframeModel.anchory = text; timeline.updateTimeline();}
			}
			
			
			//Offset
			Image{source: "../resources/materials/icons/position.png"
				TooltipArea {text: "Offset"}
			}
			TextField {
					width: 40; placeholderText: qsTr("x")
					validator: IntValidator {bottom: -9999; top: 9999;}
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.offsetx: ""
					onEditingFinished: { if(selectedItem && selectedItem.elementType == 0) {selectedItem.keyframeModel.offsetx = text; timeline.updateTimeline();}}
					}
			TextField {
					width: 40; placeholderText: qsTr("y")
					validator: IntValidator {bottom: -9999; top: 9999;}
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.offsety: ""
					onEditingFinished: { if(selectedItem && selectedItem.elementType == 0) {selectedItem.keyframeModel.offsety = text; timeline.updateTimeline();}}
			}
			
			//Scale
			Image{source: "../resources/materials/icons/scale.png"
				TooltipArea {text: "Scale"}
			}
			TextField {
					width: 40; placeholderText: qsTr("x")
					validator: IntValidator {bottom: 0; top: 9999;}
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.scalex: ""
					onEditingFinished: { if(selectedItem && selectedItem.elementType == 0) {selectedItem.keyframeModel.scalex = text; timeline.updateTimeline();}}
			}
			TextField {
					width: 40; placeholderText: qsTr("y")
					validator: IntValidator {bottom: 0; top: 9999;}
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.scaley: ""
					onEditingFinished: { if(selectedItem && selectedItem.elementType == 0) {selectedItem.keyframeModel.scaley = text; timeline.updateTimeline();}}
			}
			
			//Rotation
			Image{source: "../resources/materials/icons/rotation.png"
				TooltipArea {text: "Rotation"}
			}
			TextField {
					width: 30; 
					validator: IntValidator {bottom: 0; top: 9999;}
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.rotation: ""
					onEditingFinished: { if(selectedItem && selectedItem.elementType == 0) {selectedItem.keyframeModel.rotation = text; timeline.updateTimeline();}}
			}
			
			//Mirror
			Image{source: "../resources/materials/icons/mirror.png"
				TooltipArea {text: "Mirror"}
			}
			TextField {
					width: 30; 
					//validator: IntValidator {bottom: 0; top: 99999;}
					readOnly: true
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.mirror: ""
					onEditingFinished: { if(selectedItem && selectedItem.elementType == 0) {selectedItem.keyframeModel.mirror = text; timeline.updateTimeline();}}
			}

			Text{ text: "U:";font.pointSize: 12}
			TextField {
					width: 40;
					//validator: IntValidator {bottom: 0; top: 99999;}
					readOnly: true
					text: (selectedItem && selectedItem.elementType == 0)? selectedItem.keyframeModel.unknown: ""
					onEditingFinished: { if(selectedItem && selectedItem.elementType == 0) {selectedItem.keyframeModel.unknown = text; timeline.updateTimeline();}}
			}
			
			Text{
				text: (selectedItem && selectedItem.elementType == 0)? ((spritedata.getCutout(selectedItem.keyframeModel.cutoutID))?  ("width: "+spritedata.getCutout(selectedItem.keyframeModel.cutoutID).width+" height: "+ spritedata.getCutout(selectedItem.keyframeModel.cutoutID).height) : "") : "";
			}
		}
		//marker edit
		Row{
			spacing: 3
			visible: selectedItem && selectedItem.elementType == 1
			
			//start
			Image{source: "../resources/materials/icons/start.png" 
				TooltipArea {text: "Marker starting frame"}
			}
			TextField {
					//readOnly: true
					width: 40; 
					validator: IntValidator {bottom: 0; top: 9999;}
					text: (selectedItem && selectedItem.elementType == 1)? selectedItem.markerModel.start: ""
					
					onEditingFinished: {
						if(selectedItem && selectedItem.elementType == 1 && selectedItem.currentAnimation.moveMarker(selectedItem.markerModel, text)){
								selectedItem.markerModel.start = text
							}
					}
			}
			
			//type
			Text{ text: "T:";font.pointSize: 12}
			TextField {
					width: 30;
					validator: IntValidator {bottom: 0; top: 32;}
					readOnly: true
					text: (selectedItem && selectedItem.elementType == 1)? selectedItem.markerModel.type: ""
					onEditingFinished: { if(selectedItem && selectedItem.elementType == 1) {selectedItem.markerModel.type = text;}}
			}
			
			//A
			Text{ text: "A:";font.pointSize: 12}
			TextField {
					width: 30;
					validator: IntValidator {bottom: -99999; top: 99999;}
					readOnly: true
					text: (selectedItem && selectedItem.elementType == 1)? selectedItem.markerModel.a: ""
					onEditingFinished: { if(selectedItem && selectedItem.elementType == 1) {selectedItem.markerModel.a = text;}}
			}
			
			//B
			Text{ text: "B:";font.pointSize: 12}
			TextField {
					width: 30;
					validator: IntValidator {bottom: -99999; top: 99999;}
					readOnly: true
					text: (selectedItem && selectedItem.elementType == 1)? selectedItem.markerModel.b: ""
					onEditingFinished: { if(selectedItem && selectedItem.elementType == 1) {selectedItem.markerModel.b = text;}}
			}
		}
		
	}
	
	Item { 
		id: leftPart
		width: 128
		anchors.top: parent.top
		anchors.bottom: controllbar.top 
		
		
		Rectangle {
			id: timelinemenu
			anchors.left: parent.left 
			anchors.right: parent.right
			height: 20
			color: "gray"
			Row{
				Button {
					height: 20
					width: 20
					text: "-"
					onClicked: (timeline.steps <= 5)? timeline.steps = 5: timeline.steps -= 5 ;
				}
				
				Button {
					height: 20
					width: 20
					text: "+"
					onClicked: (timeline.steps >= 200)? timeline.steps = 200 : timeline.steps +=5;
				}
				
				Button {
					height: 20
					width: 20
					text: "R"
					onClicked: timeline.steps = 10;
				}
				
				
				Button {
					height: 20
					width: 20
					text: ""
					iconSource: "../resources/materials/icons/newlayer_s.png"
					onClicked: if(timeline.animation) timeline.animation.addLayer()
					//TooltipArea {text: "Create new layer"}
				}
			}
		}
		
		Item {
			anchors.left: parent.left 
			anchors.right: parent.right
			anchors.top: timelinemenu.bottom
			anchors.bottom: parent.bottom
			
			
			
			Flickable {
				anchors.fill: parent
				interactive: false
				clip:true
				contentY: scrollView.flickableItem.contentY
				width: parent.width 
				height: parent.height
				//contentHeight: scrollView.flickableItem.height
				
				
				Item{
					anchors.fill: parent
					//color: "green"
					Column{
						//marker header
						Rectangle {
							height: 10
							width: 126
							border.color: "black"
							border.width: 1
							
							gradient: Gradient {
								GradientStop { position: 0.0; color: "#c3c3c3" }
								GradientStop { position: 1.0; color: "#634539" }
							}
							
							Text{
								anchors.top: parent.top
								anchors.left: parent.left
								anchors.leftMargin: 4
								anchors.topMargin: 1
								
								font.pointSize: 5
								text: "Markers"
							}
						}
					
						//leyersHeaders
						Repeater {
							model: (timeline.animation)? timeline.animation : 0
							delegate: Rectangle {
								property var layerModel: model.display
								
								
								height: 40
								width: 126
								
								
								border.color: "black"
								border.width: 1
								
								gradient: Gradient {
									GradientStop { position: 0.0; color: "#c3c3c3" }
									GradientStop { position: 1.0; color: "#858585" }
								}
								
								Text{
									anchors.top: parent.top
									anchors.left: parent.left
									anchors.leftMargin: 4
									anchors.topMargin: 4
									
									font.pointSize: 9
									text: index+": "+layerModel.name
								}
								
								Image{
									anchors.top: parent.top
									anchors.right: parent.right
									anchors.rightMargin: 4
									anchors.topMargin: 4
									source: "../resources/materials/icons/openeye.png"
									
									width: 19
									height: 19
									
									MouseArea{
										anchors.fill: parent
										onClicked: {
											if(layerModel.hidden){
												parent.source = "../resources/materials/icons/openeye.png"
												layerModel.hidden = false
											}else{
												parent.source = "../resources/materials/icons/closedeye.png"
												layerModel.hidden = true
											}
											timeline.updateTimeline()
										}
									}
								}
								
								Menu { 
									id: layerContextMenu
									MenuItem {
										text: qsTr('Move up')
										onTriggered:{
											if(timeline.animation && layerModel) { timeline.animation.moveUp(layerModel) }
										}
									}
									
									MenuItem {
										text: qsTr('Move down')
										onTriggered:{
											if(timeline.animation && layerModel) { timeline.animation.moveDown(layerModel) }
										}
									}
									
									MenuSeparator { }
									MenuItem {
										text: qsTr('Add Layer')
										onTriggered:{
											if(timeline.animation &&layerModel) { timeline.animation.addLayer(layerModel) }
										}
									}
									
									MenuItem {
										text: qsTr('Delete')
										onTriggered:{
											if(timeline.animation && layerModel) {  timeline.animation.remove(layerModel)}
										}
									}
	
								}
	
								MouseArea{
									anchors.fill: parent
									acceptedButtons: Qt.RightButton
									onClicked:layerContextMenu.popup();
								}
							}
						}
					}
				}
				
				
			}//Flickable end
		}
		
		
	}

	Item { 
		id: rightPart
		anchors.left: leftPart.right 
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.bottom: controllbar.top 
		
		
		Flickable {
			anchors.top: parent.top
			interactive: false
			clip:true
			contentX: scrollView.flickableItem.contentX
			width: rightPart.width 
			height:  ruler.height
			contentWidth: (getWorkspaceWidth() < rightPart.width)? rightPart.width : getWorkspaceWidth()
			
			Item{
				id: rulerItem
				anchors.top: parent.top
				height: ruler.height
				Ruler{
					id: ruler
					width: (getWorkspaceWidth() < rightPart.width)? rightPart.width : getWorkspaceWidth()  //timeline.width*((timeline.steps*4)/10)+200
					height: 20
					stepSize: timeline.steps
					timeScale: timeline.timeScale
				}
				
				MouseArea {
					width: (getWorkspaceWidth() < rightPart.width)? rightPart.width : getWorkspaceWidth() 
					height: 20
					onPositionChanged:{
						var key = ((mouse.x/getTrackScale())).toFixed(0)
						
						if(key < 0) key = 0
						if(key > timeline.width) key = timeline.width

						timeline.tracker = key
					}
					onClicked:{
						var key = ((mouse.x/getTrackScale())).toFixed(0)
						
						if(key < 0) key = 0
						if(key > timeline.width) key = timeline.width
						
						timeline.tracker = key
					}
				}
			
				//tracker
				Rectangle {
					id: trackerbox
					anchors.top: parent.top
					width: 5
					height: 20
					color: "#565a65"
					x: -2 
					//z: 0
				}
				
				Binding {
					target: trackerbox
					property: "x"
					value: timeline.tracker*getTrackScale()-2
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
	
		ScrollView {
			id: scrollView
			anchors.fill: parent;
			anchors.topMargin: 20

			clip:true
			//Image { source: "img.png" }
			horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOn
			verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn
			
			Item {
				width: (timeline.animation)? timeline.animation.workspacelength*getTrackScale()+200  : 200
				height: (timeline.animation)? timeline.animation.layerSize* 45: 0
				Item {
					anchors.top: rightPart.top
					anchors.left: rightPart.left
					width: parent.width-200
					
					
					Column{
					//every Marker
					Item{
						
						width: parent.parent.width
						height: 10
						
						Image {
							anchors.fill: parent
							fillMode: Image.Tile
							source: "../resources/marker_bg_loop.png"
							
							Menu { 
								id: markersMenu
								property int frame: 0
								MenuItem {
									text: qsTr('Add marker')
									onTriggered:{
										if(timeline.animation) {
											timeline.animation.addMarker(markersMenu.frame,1);
										}
									}
								}
							}

							MouseArea{
								anchors.fill: parent
								acceptedButtons: Qt.RightButton

								onClicked:{
									if(timeline.animation && mouse.x > 0){
										markersMenu.frame = mouse.x/getTrackScale() 
										markersMenu.popup();
									}
								}
							}
						}
			
						Repeater {
							id: markersRep
							model: (timeline.animation)? timeline.animation.getMarkers() : 0 
							delegate: MarkerElement {
								markerModel: model.display
								currentAnimation: timeline.animation
								elementHeight: 10
								
								onSelected:{
									select(item);
								}
								onDeselected:{
									deselect(item)
								}
							}
						}
					}
					//every keyframe
					Repeater {
						model: (timeline.animation)?  timeline.animation: 0
						delegate: Rectangle {
							property var layerMod: model.display
						
							height: 40
							width: (timeline.animation)? timeline.animation.workspacelength*getTrackScale(): 0
							color: activePalette.shadow
							
							Menu { 
								id: addKeyframeMenu
								property int frame: 0
								MenuItem {
									text: qsTr('Add keyframe')
									onTriggered:{
										if(layerMod) { layerMod.insertKeyframe(addKeyframeMenu.frame); timeline.updateTimeline(); }
									}
								}
							}
							
							MouseArea {
								anchors.fill: parent
								acceptedButtons: Qt.RightButton
								
								onClicked: { 
									addKeyframeMenu.frame = mouse.x/getTrackScale()
									addKeyframeMenu.popup();
								}
							}
							//every layer
							Repeater {
								model: layerMod
								delegate: KeyframeElement {
									keyframeModel: model.display
									layerModel: layerMod
									
									onSelected:{
										select(item);
									}
									onDeselected:{
										deselect(item)
									}
								}
							}// layer repaeter end
						}
					}
					
					}
					//tracker line
					Rectangle {
						anchors.top: parent.top
						width: 1;
						height: 500;
						color: "black"
						z: 1000
						x: trackerbox.x+2//timeline.tracker*((timeline.steps*4)/10)
					}
				} // Item end
			}
		}
	}

}