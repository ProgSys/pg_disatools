package pspfs_v1;

import com.sun.jna.Library;
import com.sun.jna.Memory;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.FloatByReference;
import com.sun.jna.ptr.IntByReference;

public class DLLBridge {

	public interface DllInterface extends Library {
		DllInterface INSTANCE = (DllInterface) Native.loadLibrary("libpg_disatools", DllInterface.class);
 
		int multiply(float a, float b, FloatByReference result);
		
		Pointer uncompressTX2Image(Pointer bytesIn, int size, IntByReference width, IntByReference height);
 
	}
	
	public static float multiply(float a, float b) {
		FloatByReference r = new FloatByReference (0);
 
		int rc = DllInterface.INSTANCE.multiply (a, b, r);
 
		return r.getValue ();
	}
	
	
	public static void uncompressTX2Image(Pointer bytes, int size, int witdthOUT, int heightOut, Pointer rgbaOut) {
		IntByReference width = new IntByReference (0);
		IntByReference height = new IntByReference (0);
		
		rgbaOut = DllInterface.INSTANCE.uncompressTX2Image (bytes, 10,width,height );
		witdthOUT = width.getValue();
		heightOut = height.getValue();
		
		System.out.println("width: "+width.getValue()+" height: "+height.getValue());
	}
	
	
}
