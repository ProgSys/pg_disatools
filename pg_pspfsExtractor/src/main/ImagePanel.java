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
	private JLabel lbdllError;
	
    public ImagePanel(JLabel lbError) {
    	image = null;
    	lbdllError = lbError;
    }
    
    @SuppressWarnings("unused")
	public void displayTX2Image(Pointer dxt, int size){
    	if(dxt == null || size == 0) {
    		image = null;
    		return;
    	}
    	
    	
    	Size imageSize = new Size();
    	PointerByReference rgbaImage = new PointerByReference();
    	
    	if(DLLBridge.uncompressTX2Image(dxt,size,imageSize,rgbaImage )){
    		lbdllError.setText("<html>libpg_disatools.dll<br>not found!</html>");
    		image = null;
    		return;
    	}
    	
    	if(imageSize.isZero()){
    		image = null;
    		return;
    	}
    	
    	Pointer p = rgbaImage.getValue();
    	image = new BufferedImage(imageSize.x, imageSize.y, BufferedImage.TYPE_INT_RGB);
    	for(int y = 0; y < imageSize.y ; y++)
	    	for(int x = 0; x < imageSize.x; x++){
	    		int index = ((imageSize.x*y)+x)*4;
	    		int r = p.getChar(index) & 0xFF;
	    		int g = p.getChar(index+1) & 0xFF;
	    		int b = p.getChar(index+2) & 0xFF;
	    		int a = p.getChar(index+3) & 0xFF;
	    		int rgb = (a << 24) | (r << 16) | (g << 8) | b;
	    		image.setRGB(x, y, rgb);
	    	}
  
    }

    public void clearImage(){
    	image = null;
    }
    
    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        g.drawImage(image, 0, 0, this.getWidth(),this.getHeight(), null); // see javadoc for more info on the parameters            
    }

}
