import QtQuick 2.9
import QtQuick.Controls 2.12 as Con2
import MyTimeLine 0.1
import MyKeyframe 0.1
import MyCutout 0.1
import MySpriteData 0.1
import QtGraphicalEffects 1.0
import QtQuick.Dialogs 1.0
import QtQuick.Controls 1.4

Rectangle { 
	id: root
    SystemPalette { id: activePalette }

			
    color: activePalette.dark
	
	property int activeSpriteSheet: 0
	property var zoom: 2.0
	readonly property var zoomMin: 0.1 //out
	readonly property var zoomMax: 55.0 //in
	readonly property var cutoutModel: spritedata.sheetsSize? spritedata.getSpriteSheet(activeSpriteSheet): null
	property bool pixelBrushActive: false 
	
	onPixelBrushActiveChanged:{
		spritedata.selected = null
	}

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
		return Math.round((scroll.contentX +mouse.x)/zoom);
	}
	
	onActiveSpriteSheetChanged: {
		spritedata.selected = null
		root.cutoutSelected(-1, spritedata.selected);
		spriteimage.source = "image://imageprovider/"+activeSpriteSheet
	}
	
	Connections{
		target: spritedata
		function onCurrentColorTableChanged() {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		function onColorTableChanged() {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		function onAllColorTablesChanged() {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		function onSpriteSheetChanged() {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		function onSpriteSheetAdded() { activeSpriteSheet = spritesheetID}
	}
	
  	Connections{
		target: spritedata.getSpriteSheet(activeSpriteSheet)
		function onOnNumberOfCutoutsChanged() {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		function onOnWidthChanged() {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		function onOnHeightChanged() {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
		function onNumberOfColorsChanged() {spriteimage.source = ""; spriteimage.source = "image://imageprovider/"+activeSpriteSheet;}
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
					if(mouse.button & Qt.RightButton){
						contextMenu.posX = Math.round((scroll.contentX +mouse.x)/zoom);
						contextMenu.posY = Math.round((scroll.contentY +mouse.y-25)/zoom);
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
			IconToolButton {
				image: "../materials/icons/item_previous.png"
				onClicked: {
					if(spritedata.sheetsSize){
						(activeSpriteSheet > 0)? activeSpriteSheet-- :  activeSpriteSheet = spritedata.sheetsSize-1 ;
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
			
			IconToolButton {
				image:"../materials/icons/item_next.png"
				onClicked: {
					if(spritedata.sheetsSize){
							(activeSpriteSheet < spritedata.sheetsSize-1 )? activeSpriteSheet++ :  activeSpriteSheet = 0 ;
					}
				}
				
				PGToolTip {
					text: "Next"
				}
			}
			
			IconToolButton {
				image: "../materials/icons/zoom_in.png"
				onClicked: {
					zoom = Math.min(zoomMax, zoom + zoom * 0.2)
				}
				PGToolTip {
					text: "Zoom in"
				}
			}
			
			IconToolButton {
				image: "../materials/icons/zoom_out.png"
				onClicked: {
					zoom = Math.max(zoomMin, zoom - zoom * 0.2)
				}
				PGToolTip {
					text: "Zoom out"
				}
			}
			IconToolButton {
				image: "../materials/icons/zoom_reset.png"
				onClicked: {
					zoom = 2.0
					scroll.contentX = 0;
					scroll.contentY = 0;
				}
				PGToolTip {
					text: "Reset zoom"
				}
			}
			Item{ width: 20; height: 1 }
			
			IconToolButton {
				image: spritedata.isolateSelection? "../materials/icons/isolate_selection_on.png": "../materials/icons/isolate_selection_off.png"
				onClicked: {
					spritedata.isolateSelection = !spritedata.isolateSelection;
				}
				
				PGToolTip {
					text: "Isolate selection"
				}
			}
			
			IconToolButton {
				image: root.pixelBrushActive? "../materials/icons/pixel_on.png": "../materials/icons/pixel_off.png"
				onClicked: {
					root.pixelBrushActive = !root.pixelBrushActive;
				}
				
				PGToolTip {
					text: "Pixel brush (Control: Erase (second color) Alt: Pic color)"
				}
			}
			
			Item{ width: 20; height: 1 }
			
			IconToolButton {
				image: "../materials/icons/newcutout.png"
				
				onClicked: {
					spritedata.addCutout(activeSpriteSheet);
				}
				
				PGToolTip {
					text: "Add new sprite"
				}
			}
			
			IconToolButton {
				image: "../materials/icons/newspritesheet.png"
				onClicked: {
					spritedata.addNewSpriteSheet();
				}
				
				PGToolTip {
					text: "Add new sprite sheet"
				}
			}
			
			IconToolButton {
				image: "../materials/icons/settings.png"
				
				onClicked: {
					spritedata.editSpriteSheet(activeSpriteSheet);
				}
				
				PGToolTip {
					text: "Sprite sheet settings"
				}
			}
			
			IconToolButton {
				enabled: {
					var sheet = spritedata.getSpriteSheet(activeSpriteSheet);
					sheet? sheet.isExternal: false
				}
				image: "../materials/icons/open_external.png"
				
				onClicked: {
					spritedata.openExternalSpriteSheet(activeSpriteSheet);
				}
				
				PGToolTip {
					text: "Open external sprite sheet"
				}
			}
			
			Item{ width: 20; height: 1 }
			Text{
				text:  {
					if(spritedata.sheetsSize){
						var sheet = spritedata.getSpriteSheet(activeSpriteSheet);
						if(sheet.isExternal)
							return "External ID: " + sheet.externalID;
						else 
							return "Width: "+sheet.width+" Height: "+sheet.height +" Colors: "+sheet.colors 
							
					}else {
						return "";
					}
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
		anchors.bottomMargin: 25
		
		visible: spritedata.sheetsSize != 0 && activeSpriteSheet < spritedata.sheetsSize 

		Flickable  {
			id: scroll
			anchors.fill: parent
			clip:false
			topMargin: 1000
			leftMargin: 1000
			rightMargin: 1000
			bottomMargin: 1000
			contentX:0
			contentY:0;
			
			contentWidth : spriteimageContainer.width 
			contentHeight: spriteimageContainer.height 
			//wheelEnabled : false
			Con2.ScrollBar.vertical: Con2.ScrollBar { width:15 }
			Con2.ScrollBar.horizontal: Con2.ScrollBar { height:15 }
			pixelAligned: true
			Item {
				MouseArea {
					x: -1000
					y: -1000
					width:3000
					height:3000
					acceptedButtons: Qt.LeftButton 
					onClicked: { 
						if(spritedata.selected) {spritedata.selected = null; root.cutoutSelected(-1, spritedata.selected);} 
					}
				}
				Rectangle{
					x: -10000
					width:20000
					height: 1
					color: "#B25F5F"
				}
				Rectangle{
					y: -10000
					width: 1
					height: 20000
					color: "#625FB2"
				}
				Item {
					id: spriteimageContainer
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
							function onRefresh(){ spriteimage.refresh();}
							function onOnNumberOfSheetsChanged(){ spriteimage.refresh();}
							function onSelectedColorIdChanged(){ spriteimage.refresh();}
							function onIsolateSelectionChanged(){ spriteimage.refresh();}
							function onUpdateSpriteSheetImage(){ spriteimage.refresh();}
							function onSelectedChanged(){ if(spritedata.isolateSelection) spriteimage.refresh();}
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
		
		MouseArea{
			anchors.fill: parent
			acceptedButtons: Qt.MiddleButton
			cursorShape: undefined
			propagateComposedEvents: true
			preventStealing: true
			
			property int startStartDragX: 0
			property int startStartDragY: 0
			
			onWheel:{
				var newZoom = Math.max( Math.min(zoom + (zoom/wheel.angleDelta.y) * 7 , zoomMax), zoomMin);
				var diff = zoom/newZoom;
				
				var x = scroll.contentX + wheel.x
				var y = scroll.contentY + wheel.y 
				scroll.contentX += x - x * diff;
				scroll.contentY += y - y * diff;
				zoom = newZoom;
				wheel.accepted = true;
			}
			
			onPressed: {
				startStartDragX = mouse.x;
				startStartDragY = mouse.y;
			}
			
			onPositionChanged:{
				if(!(mouse.buttons & Qt.MiddleButton)) return;
				
				scroll.contentX += startStartDragX - mouse.x;
				scroll.contentY += startStartDragY - mouse.y;
				startStartDragX = mouse.x;
				startStartDragY = mouse.y;
				mousePosIndicator.mouseX = mouse.x
				mousePosIndicator.mouseY = mouse.y + 25
				mouse.accepted = false;
			}
		}
		
		MouseArea{
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton | Qt.RightButton
			propagateComposedEvents: false
			preventStealing: true
			hoverEnabled: root.pixelBrushActive
			enabled: root.pixelBrushActive
			property var xImage: 0
			property var yImage: 0
			property var color: 0
			property var clearColor: 0
			
			onPressed: {
				if(mouse.buttons & Qt.RightButton) {
					root.pixelBrushActive = false;
					return;
				}
				
				var sheet = spritedata.getSpriteSheet(activeSpriteSheet);
	
				if(mouse.modifiers & Qt.AltModifier) {
					if(mouse.modifiers & Qt.ControlModifier){
						clearColor = spritedata.getColorIndex(activeSpriteSheet, xImage, yImage);
					}else{
						color = spritedata.getColorIndex(activeSpriteSheet, xImage, yImage);
					}
					return;
				}
				
				var targetColor = mouse.modifiers & Qt.ControlModifier? clearColor: color;
				
				spritedata.addUndoSpriteSheetPixel(activeSpriteSheet, xImage, yImage, targetColor);
				spritedata.setColorIndex(activeSpriteSheet, xImage, yImage, targetColor);
				
			}
			
			onReleased: {
				spritedata.pushUndoSpriteSheetPixels();
			}
			
			onPositionChanged:{
				
				xImage = Math.floor((scroll.contentX + mouse.x)/zoom);
				yImage = Math.floor((scroll.contentY + mouse.y)/zoom);
				
				mousePosIndicator.mouseX = mouse.x;
				mousePosIndicator.mouseY = mouse.y;
				
				if(mouse.buttons & Qt.LeftButton && (mouse.modifiers & Qt.AltModifier) == 0) {
					var targetColor = mouse.modifiers & Qt.ControlModifier? clearColor: color;
					spritedata.addUndoSpriteSheetPixel(activeSpriteSheet, xImage, yImage, targetColor);
					spritedata.setColorIndex(activeSpriteSheet, xImage, yImage, targetColor)
				}
			}
			
			Rectangle{
				id: brush
				visible: root.pixelBrushActive && parent.containsMouse 
				width: zoom
				height: zoom
				x: parent.xImage * zoom - scroll.contentX;
				y: parent.yImage * zoom - scroll.contentY;
				color: "transparent"
				border.color: activePalette.light
				border.width: 1
				
				Rectangle{
					anchors.rightMargin : 1
					anchors.leftMargin : 1
					anchors.topMargin : 1
					anchors.bottomMargin : 1
					anchors.fill: parent
					color: "transparent"
					border.color: activePalette.dark
					border.width: 1
				}
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
			PGIntField {
					width: 30; placeholderText: qsTr("ID")
					enabled: spritedata.selected
					text: (spritedata.selected)? spritedata.selected.colortable: ""
					
					onEdited: {
						if(spritedata.selected && spritedata.selected.colortable != value) {
							spritedata.selected.colortable = (value >= spritedata.colortableSize)? spritedata.colortableSize-1: value ; 
							spritedata.refresh();
						}
					}
			}
			
			//position
			Image{source: "../materials/icons/position.png"
				TooltipArea {text: "Position"}
			}
			PGIntField {
					placeholderText: qsTr("x")
					enabled: spritedata.selected
					text: (spritedata.selected)? spritedata.selected.x: ""
					
					onEdited: { 
						if(spritedata.selected && spritedata.selected.x != value) {
							spritedata.pushUndoPosition(spritedata.selected);
							spritedata.selected.x = value ; spritedata.refresh();
						}
					}
			}
			PGIntField {
					placeholderText: qsTr("y")
					enabled: spritedata.selected
					text: (spritedata.selected)? spritedata.selected.y: ""
					
					onEdited: {
						if(spritedata.selected && spritedata.selected.y != value) {
							spritedata.pushUndoPosition(spritedata.selected);
							spritedata.selected.y = value ; spritedata.refresh();
						}
					}
			}
			
			
			//Offset
			Image{source: "../materials/icons/size.png"
				TooltipArea {text: "Size"}
			}
			PGIntField {
				placeholderText: qsTr("Width")
				enabled: spritedata.selected
				text: (spritedata.selected)? spritedata.selected.width: ""
				onEdited: { 
					if(spritedata.selected && spritedata.selected.width != value) {
						spritedata.pushUndoPosition(spritedata.selected);
						spritedata.selected.width = value ; spritedata.refresh();
					}
				}
			}
			PGIntField {
				placeholderText: qsTr("Height")
				enabled: spritedata.selected
				text: (spritedata.selected)? spritedata.selected.height: ""
				onEdited: { 
					if(spritedata.selected && spritedata.selected.height != value) {
						spritedata.pushUndoPosition(spritedata.selected);
						spritedata.selected.height = value ;
						spritedata.refresh();
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
				text: Math.floor((scroll.contentX + parent.mouseX)/zoom) +":"+ Math.floor((scroll.contentY +parent.mouseY-25)/zoom)
			}
			width: childrenRect.width
			height: childrenRect.height
			visible: false
	}

}