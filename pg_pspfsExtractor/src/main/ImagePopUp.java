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

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import javax.swing.JFileChooser;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.tree.TreePath;

public class ImagePopUp extends JPopupMenu {
    /**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	JMenuItem saveAsPNG;
	JMenuItem saveAsTGA;
	JMenuItem saveAsJPG;
	
	ImagePanel imagebox;
	
    public ImagePopUp(ImagePanel imagebox){
    	this.imagebox =imagebox;
    	
    	saveAsPNG = new JMenuItem("Save as PNG");
    	saveAsPNG.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				openSaveDialog("png");
			}

		});
    	
    	/*
    	saveAsTGA = new JMenuItem("Save as TGA");
    	saveAsTGA.addActionListener(new ActionListener() {
    			public void actionPerformed(ActionEvent arg0) {
    				openSaveDialog("tga");
    			}
    		});
    		*/
       	
    	saveAsJPG = new JMenuItem("Save as JPG");
    	saveAsJPG.addActionListener(new ActionListener() {
    			public void actionPerformed(ActionEvent arg0) {
    				openSaveDialog("jpg");
    			}
    		});
    	
    	
        add(saveAsPNG);
        //add(saveAsTGA);
        add(saveAsJPG);
    }
    
    private void openSaveDialog(String type){
		if(!imagebox.hasImage()) return;
		
		 
		JFileChooser chooser = new JFileChooser();
		chooser.setCurrentDirectory(new java.io.File("."));
		chooser.setDialogTitle("Save "+type);
		 FileNameExtensionFilter filter = new FileNameExtensionFilter(type, type);
		 
		chooser.setFileFilter(filter);
		
		if (chooser.showSaveDialog(null) == JFileChooser.APPROVE_OPTION) {
			File file = chooser.getSelectedFile();
			
			String fname = file.getAbsolutePath();
			String ext = "."+type;
			if(!fname.endsWith(ext) ) 
                file = new File(fname + ext);

			imagebox.saveImage(file, type);
		}
    }
}
