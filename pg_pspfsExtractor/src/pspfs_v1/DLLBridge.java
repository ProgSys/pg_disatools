package pspfs_v1;

import com.sun.jna.Library;
import com.sun.jna.Memory;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.FloatByReference;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;

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
	
	/**
	 * Uses the dll to uncompress a .tx2 image.
	 * @param bytes
	 * @param size
	 * @param sizeOut
	 * @param rgbaOut
	 * @return true, if dll not found.
	 */
	public static boolean uncompressTX2Image(Pointer bytes, int size, Size sizeOut, PointerByReference rgbaOut) {
		IntByReference width = new IntByReference (0);
		IntByReference height = new IntByReference (0);
		
		try{
			rgbaOut.setValue(DllInterface.INSTANCE.uncompressTX2Image (bytes, 10,width,height ));
		}catch(UnsatisfiedLinkError e){
			System.err.println("[ERROR] Couldn't load DLL! "+e.toString());
			return true;
		}catch(NoClassDefFoundError e){
			System.err.println("[ERROR] Couldn't load DLL! "+e.toString());
			return true;
		}
		
		sizeOut.x = width.getValue();
		sizeOut.y = height.getValue();
		
		System.out.println("width: "+sizeOut.x+" height: "+sizeOut.y);
		return false;
	}
	
	
}
