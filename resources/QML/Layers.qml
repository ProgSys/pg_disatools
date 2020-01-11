import QtQuick 2.4
import QtQuick.Controls 1.4
import MyTimeLine 0.1
import MyKeyframe 0.1

Rectangle {
	Repeater {
		model: timeline.animation
		delegate:
	}
}