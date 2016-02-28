package main;

import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;
import javax.swing.JPanel;

import com.sun.jna.Pointer;

import pspfs_v1.DLLBridge;

public class ImagePanel extends JPanel{

    /**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private BufferedImage image;

    public ImagePanel() {
       try {                
          image = ImageIO.read(new File("D:/Users/ProgSys/pg_disatools/pg_pspfsExtractor/bin/i_image.jpg"));
       } catch (IOException ex) {
    		System.out.println("[ERROR] Couldn't display preview image! "+ex.toString());
       }
    }
    
    @SuppressWarnings("unused")
	public void displayTX2Image(Pointer dxt, int size){
    	if(dxt == null || size == 0) return;
    	
    	int width = 0;
    	int height = 0;
    	Pointer rgbaImage = null;
    	
    	DLLBridge.uncompressTX2Image(dxt,size,width,height,rgbaImage );
    	
    	if(rgbaImage == null || width == 0 || height == 0) return;
    	
    	image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
    	
    	for(int y = 0; y < height; y++)
	    	for(int x = 0; x < width; x++){
	    		int index = ((width*y)+x)*4;
	    		char r = rgbaImage.getChar(index);
	    		char g = rgbaImage.getChar(index+1);
	    		char b = rgbaImage.getChar(index+2);
	    		int rgb = (r << 16) | (g << 8) | b;
	    		image.setRGB(x, y, rgb);
	    	}
  
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        g.drawImage(image, 0, 0, this.getWidth(),this.getHeight(), null); // see javadoc for more info on the parameters            
    }

}
