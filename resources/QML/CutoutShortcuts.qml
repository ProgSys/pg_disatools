import QtQuick 2.7

Item {
	property var spriteBackground: null
	Shortcut {
		enabled: isSelected
		sequence: "Up"
		autoRepeat: true
		onActivated: move(0,-1)
	}
	Shortcut {
		enabled: isSelected
		sequence: "Down"
		autoRepeat: true
		onActivated: move(0,1)
	}
	Shortcut {
		enabled: isSelected
		sequence: "Left"
		autoRepeat: true
		onActivated: move(-1,0)
	}
	Shortcut {
		enabled: isSelected
		sequence: "Right"
		autoRepeat: true
		onActivated: move(1,0)
	}
	Shortcut {
		enabled: isSelected
		sequence: "Delete"
		onActivated: {
			var buffer = spriteimage;
			spritedata.selected = null
			spritedata.removeCutoutID(model.display);
			buffer.refresh();
		}
	}
}