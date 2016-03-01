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

import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;
import javax.swing.JLabel;
import javax.swing.JPanel;

import com.sun.jna.Pointer;
import com.sun.jna.ptr.PointerByReference;

import pspfs_v1.DLLBridge;
import pspfs_v1.Size;

public class ImagePanel extends JPanel{

    /**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private BufferedImage image;
	private BufferedImage imageRGB;
	private JLabel lbdllError;
	
    public ImagePanel(JLabel lbError) {
    	image = null;
    	imageRGB = null;
    	lbdllError = lbError;
    }
    
    @SuppressWarnings("unused")
	public void displayTX2Image(Pointer dxt, int size){
    	if(dxt == null || size == 0) {
    		image = null;
    		imageRGB = null;
    		return;
    	}
    	
    	
    	Size imageSize = new Size();
    	PointerByReference rgbaImage = new PointerByReference();
    	
    	if(DLLBridge.uncompressTX2Image(dxt,size,imageSize,rgbaImage )){
    		lbdllError.setText("<html>libpg_disatools.dll<br>not found!</html>");
    		image = null;
    		imageRGB = null;
    		return;
    	}
    	
    	if(imageSize.isZero()){
    		image = null;
    		imageRGB = null;
    		return;
    	}
    	
    	Pointer p = rgbaImage.getValue();
    	image = new BufferedImage(imageSize.x, imageSize.y, BufferedImage.TYPE_INT_ARGB);
    	imageRGB = new BufferedImage(imageSize.x, imageSize.y, BufferedImage.TYPE_INT_RGB);
    	for(int y = 0; y < imageSize.y ; y++)
	    	for(int x = 0; x < imageSize.x; x++){
	    		int index = ((imageSize.x*y)+x)*4;
	    		int r = p.getChar(index) & 0xFF;
	    		int g = p.getChar(index+1) & 0xFF;
	    		int b = p.getChar(index+2) & 0xFF;
	    		int a = p.getChar(index+3) & 0xFF;
	    		int rgb = (a << 24) | (r << 16) | (g << 8) | b;
	    		image.setRGB(x, y, rgb);
	    		imageRGB.setRGB(x, y, rgb);
	    	}
  
    }

    public void clearImage(){
    	image = null;
    	imageRGB = null;
    }
    
    public void saveImage(File outfile, String type){
    	if(image == null || imageRGB == null) return;
    	
    	if(type == "jpg"){
        	try {
        		ImageIO.write(imageRGB, type, outfile);
    		} catch (IOException e) {
    			e.printStackTrace();
    		}
        	return;
    	}
    	
    	try {
			ImageIO.write(image, type, outfile);
		} catch (IOException e) {
			e.printStackTrace();
		}
    }

    public boolean hasImage(){
    	return image != null;
    }
    
    
    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        g.drawImage(imageRGB, 0, 0, this.getWidth(),this.getHeight(), null); // see javadoc for more info on the parameters            
    }

}
