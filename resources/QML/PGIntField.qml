import QtQuick 2.12
import QtQuick.Controls 2.12
import SpriteSheetEditor 0.1

TextField {
	id: field
	
	property alias min: valid.bottom
	property alias max: valid.top
	
	signal edited(int value);
	
	width: 40; 
	height: 20; 
	validator: IntValidator {id: valid; bottom: 0; top: 9999;}
	selectByMouse: true
	selectedTextColor: "white"
	topPadding: 1
	bottomPadding: 1
	leftPadding: 4
	rightPadding: 4
	inputMethodHints: Qt.ImhDigitsOnly
	
	onEditingFinished:{
		edited(parseInt(text));
	}
	
	PGIntFieldLisener{
		spriteData: spritedata
		onMouseWheel: {
			var original = parseInt(text);
			var value = original + delta;
			if(value < min) value = min;
			else if(value > max) value = max;
			edited(value);
		}
	}
}