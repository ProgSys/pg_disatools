/*
 *  Copyright (C) 2016  ProgSys
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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