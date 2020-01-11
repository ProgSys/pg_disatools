import QtQuick 2.4
import QtQuick.Controls 2.12
import MyTimeLine 0.1
import MyKeyframe 0.1
import MyCutout 0.1
import MySpriteData 0.1
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.0

Rectangle { 
	id: root
    SystemPalette { id: activePalette }
    color: activePalette.dark
	
	property int activeSpriteSheet: 0
	property var zoom: 2.0
	readonly property var cutoutModel: spritedata.sheetsSize? spritedata.getSpriteSheet(activeSpriteSheet): null

	signal cutoutSelected(var id, var cutout)
	
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
	
	function getPosX(mouse){
		return Math.round((scroll.flickableItem.contentX +mouse.x)/zoom);
	}
	
	Connections{
		target: spritedata
		onCurrentColorTableChanged: {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		onColorTableChanged: {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		onAllColorTablesChanged: {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		onSpriteSheetChanged: {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
	}
	
  	Connections{
		target: spritedata.getSpriteSheet(activeSpriteSheet)
		onOnNumberOfCutoutsChanged: {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		onOnWidthChanged: {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		onOnHeightChanged: {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		onNumberOfColorsChanged: {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
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
			text: qsTr('Add sprite at ('+contextMenu.posX+", "+contextMenu.posY+")")
			onTriggered:{
				spritedata.addCutout(activeSpriteSheet,contextMenu.posX,contextMenu.posY,20,20)
				spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
			}
		}
		
		MenuSeparator { }
		
		MenuItem {
			//"Will export the sprite sheet using the colors from the selected sprite"
			visible: spritedata.selected 
			text: "Export as color"
			onTriggered: spritedata.exportSpriteSheet( spritedata.selected );
		} 
		
		MenuItem {
			//"Will import a sprite sheet using the colors from the selected sprite"
			visible: spritedata.selected 
			text: "Import as color"
			onTriggered:{
				spritedata.importSpriteSheetAsColor(spritedata.selected)
				spriteimage.refresh();
			}
		}
		
		MenuSeparator { visible: spritedata.selected }
		
		MenuItem {
			text: qsTr('Import sheet as color')
			onTriggered:{
				spritedata.importSpriteAsColorForSheet(activeSpriteSheet)
				spriteimage.refresh();
			}
		}
		
		MenuItem {
			text: qsTr('Import sheet as IDs')
			onTriggered:{
				spritedata.importSpriteAsIDs(activeSpriteSheet, 0,0, spritedata.getSpriteSheet(activeSpriteSheet).width, spritedata.getSpriteSheet(activeSpriteSheet).height)
				spriteimage.refresh();
			}
		}
		
		MenuSeparator { }
		MenuItem {
			text: qsTr('Auto find cutouts')
			onTriggered:{
				spritedata.autoFindCutouts(activeSpriteSheet)
				spriteimage.refresh();
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
			acceptedButtons: Qt.LeftButton | Qt.RightButton
			onClicked: { 
					if(mouse.button & Qt.LeftButton){
						if(spritedata.selected) {spritedata.selected = null; root.cutoutSelected(-1, spritedata.selected);} 
					}else{
						contextMenu.posX = Math.round((scroll.flickableItem.contentX +mouse.x)/zoom);
						contextMenu.posY = Math.round((scroll.flickableItem.contentY +mouse.y-25)/zoom);
						contextMenu.popup();
					}
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
				iconSource: "../materials/icons/item_previous.png"
				onClicked: {
					if(spritedata.sheetsSize){
						(activeSpriteSheet > 0)? activeSpriteSheet-- :  activeSpriteSheet = spritedata.sheetsSize-1 ;
						spritedata.selected = null
						root.cutoutSelected(-1, spritedata.selected);
						spriteimage.source = "image://imageprovider/"+activeSpriteSheet
						}
				}
				
				PGToolTip {
					text: "Previous"
				}
			}
			
			Text{
				width: 60
				font.pointSize: 12
				verticalAlignment: Text.AlignVCenter
				horizontalAlignment: Text.AlignHCenter
		
				text: {
					if(spritedata.sheetsSize == 0) return "--";
					var sheet = spritedata.getSpriteSheet(activeSpriteSheet);
					if(sheet && sheet.isExternal)
						return (activeSpriteSheet+1)+"E/"+spritedata.sheetsSize;
					else return (activeSpriteSheet+1)+"/"+spritedata.sheetsSize
				}
				
				TooltipArea {
				text: {
					var sheet = spritedata.getSpriteSheet(activeSpriteSheet);
					if(sheet){
						if(sheet.isExternal)
							return "External sprite sheet Nr. "+(activeSpriteSheet+1)
						else return "Sprite sheet Nr. "+(activeSpriteSheet+1)
					}
					return "Empty"
					}
				}
			}
			
			Button {
				height: 24
				width: 24
				iconSource: "../materials/icons/item_next.png"
				onClicked: {
					if(spritedata.sheetsSize){
							(activeSpriteSheet < spritedata.sheetsSize-1 )? activeSpriteSheet++ :  activeSpriteSheet = 0 ;
							spritedata.selected = null
							root.cutoutSelected(-1, spritedata.selected);
							spriteimage.source = "image://imageprovider/"+activeSpriteSheet
						}
				}
				
				PGToolTip {
					text: "Next"
				}
			}
			
			Button {
				height: 24
				width: 24
				text: ""
				iconSource: "../materials/icons/zoom_in.png"
				onClicked: {
					(zoom >= 4)? zoom = 4 : zoom += 0.5
				}
				PGToolTip {
					text: "Zoom in"
				}
			}
			
			Button {
				height: 24
				width: 24
				text: ""
				iconSource: "../materials/icons/zoom_out.png"
				onClicked: {
					(zoom <= 0.5)? zoom = 0.5 : zoom -= 0.5
				}
				PGToolTip {
					text: "Zoom out"
				}
			}
			Item{ width: 20; height: 1 }
			
			Button {
				height: 24
				width: 24
				text: ""
				iconSource: spritedata.isolateSelection? "../materials/icons/isolate_selection_on.png": "../materials/icons/isolate_selection_off.png"
				onClicked: {
					spritedata.isolateSelection = !spritedata.isolateSelection;
				}
				
				PGToolTip {
					text: "Isolate selection"
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
				
				PGToolTip {
					text: "Add new sprite"
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
				
				PGToolTip {
					text: "Add new sprite sheet"
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
				
				PGToolTip {
					text: "Sprite sheet settings"
				}
			}
			
			Button {
				height: 24
				width: 24
				text: "O"
				enabled: {
					var sheet = spritedata.getSpriteSheet(activeSpriteSheet);
					sheet? sheet.isExternal: false
				}
				//iconSource: "../materials/icons/settings.png"
				onClicked: {
					spritedata.openExternalSpriteSheet(activeSpriteSheet);
				}
				
				PGToolTip {
					text: "Open external sprite sheet"
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
						function refresh() {
							spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
						}
	
						Connections {
							target: spritedata
							onOnNumberOfSheetsChanged: spriteimage.refresh();
							onIsolateSelectionChanged: spriteimage.refresh();
							onSelectedChanged: if(spritedata.isolateSelection) spriteimage.refresh();
						}
							
						
						Repeater {
							model: cutoutModel
							delegate: CutoutElement{ cutout: spritedata.getCutout(model.display)}
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
					text: (spritedata.selected)? spritedata.selected.colortable: ""
					
					onEditingFinished: {
						if(spritedata.selected && spritedata.selected.colortable != text) {
							spritedata.selected.colortable = (text >= spritedata.colortableSize)? spritedata.colortableSize-1: text ; 
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
					text: (spritedata.selected)? spritedata.selected.x: ""
					
					onEditingFinished: { 
						if(spritedata.selected) {
							spritedata.selected.x = text ; spritedata.refresh();
							spriteimage.source = "";
							spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
						}
					}
			}
			TextField {
					width: 40; placeholderText: qsTr("y")
					validator: IntValidator {bottom: -9999; top: 9999;}
					text: (spritedata.selected)? spritedata.selected.y: ""
					
					onEditingFinished: {
						if(spritedata.selected) {
							spritedata.selected.y = text ; spritedata.refresh()
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
					text: (spritedata.selected)? spritedata.selected.width: ""
					onEditingFinished: { 
						if(spritedata.selected) {
							spritedata.selected.width = text ; spritedata.refresh()
							spriteimage.source = "";
							spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
						}
					}
			}
			TextField {
					width: 40; placeholderText: qsTr("Height")
					validator: IntValidator {bottom: 0; top: 9999;}
					text: (spritedata.selected)? spritedata.selected.height: ""
					onEditingFinished: { 
						if(spritedata.selected) {
							spritedata.selected.height = text ; spritedata.refresh()
							spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;
						}
					}
			}
		}
		
		Row{
			anchors.right: parent.right
			anchors.top: parent.top
			visible: bottommenu.width > 320
			Text {
					width: 40; 
					text: (spritedata.selected)? spritedata.getCoutoutIndex(spritedata.selected): ""
			}
			
			SelectionDorpDown{
				model: cutoutModel
				selectedId: (spritedata.selected)? spritedata.getCoutoutIndex(spritedata.selected): -1
				onSelected: spritedata.selected = spritedata.getCutout(value)
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