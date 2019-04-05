import QtQuick 2.4
import MyTimeLine 0.1
import MyKeyframe 0.1
import MyCutout 0.1
import MySpriteData 0.1
import MyMarker 0.1
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4
////http://blogofmu.com/2011/04/18/drag-and-drop-reorder-list-in-qml-qt/
Rectangle {
    id: wholeBody
    width: 360
    height: 360

    Component {
        id: starwarsDelegate
		
        Rectangle {
		property var layerModel: model.display
            id: starwarsDelegateBorder
            border.color: "black"
            width: wholeBody.width
            height: 40
            Row {
                spacing: 10
                Text { text: layerModel.name }
            }
            MouseArea {
                id: dragArea
                anchors.fill: parent
                property int positionStarted: 0
                property int positionEnded: 0
                property int positionsMoved: Math.floor((positionEnded - positionStarted)/40)
                property int newPosition: index + positionsMoved
                property bool held: false
                drag.axis: Drag.YAxis
                onPressAndHold: {
                    starwarsDelegateBorder.z = 2,
                    positionStarted = starwarsDelegateBorder.y,
                    dragArea.drag.target = starwarsDelegateBorder,
                    starwarsDelegateBorder.opacity = 0.5,
                    starwarsList.interactive = false,
                    held = true
                    drag.maximumY = (wholeBody.height - 20 - 1 + starwarsList.contentY),
                    drag.minimumY = 0
                }
                onPositionChanged: {
                    positionEnded = starwarsDelegateBorder.y;
                }
                onReleased: {
                    if (Math.abs(positionsMoved) < 1 && held == true) {
                        starwarsDelegateBorder.y = positionStarted,
                        starwarsDelegateBorder.opacity = 1,
                        starwarsList.interactive = true,
                        dragArea.drag.target = null,
                        held = false
                    } else {
                        if (held == true) {
                            if (newPosition < 1) {
                                starwarsDelegateBorder.z = 1,
                                timeline.animation.move(layerModel,0),
                                starwarsDelegateBorder.opacity = 1,
                                starwarsList.interactive = true,
                                dragArea.drag.target = null,
                                held = false
                            } else if (newPosition > starwarsList.count - 1) {
                                starwarsDelegateBorder.z = 1,
                                timeline.animation.move(layerModel,starwarsList.count - 1),
                                starwarsDelegateBorder.opacity = 1,
                                starwarsList.interactive = true,
                                dragArea.drag.target = null,
                                held = false
                            }
                            else {
                                starwarsDelegateBorder.z = 1,
                                timeline.animation.move(layerModel,newPosition),
                                starwarsDelegateBorder.opacity = 1,
                                starwarsList.interactive = true,
                                dragArea.drag.target = null,
                                held = false
                            }
                        }
                    }
                }
            }
        }
    }
    ListView {
        id: starwarsList
        anchors.fill: parent
        model: (timeline.animation)? timeline.animation : 0
        delegate: starwarsDelegate
    }
}