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
