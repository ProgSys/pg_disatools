package main;

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

class PopClickListener extends MouseAdapter {
	ImagePanel imagebox;
	
	public PopClickListener(ImagePanel imagebox){
		this.imagebox = imagebox;
	}
	
    public void mousePressed(MouseEvent e){
        if (imagebox.hasImage() && e.isPopupTrigger())
            doPop(e);
    }

    public void mouseReleased(MouseEvent e){
        if (imagebox.hasImage() && e.isPopupTrigger())
            doPop(e);
    }

    private void doPop(MouseEvent e){
    	ImagePopUp menu = new ImagePopUp(imagebox);
        menu.show(e.getComponent(), e.getX(), e.getY());
    }
}