/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2016 ProgSys
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */
package pspfs_v1;

import java.io.UnsupportedEncodingException;

/**
 * This class contains informations about a file inside the PSP FileSystem v1.<br>
 * The Informations are:<br>
 * <ul>
 * <li> <b>Name of the file:</b> String 44byte (needs trimming)</li>
 * <li> <b>Length of file:</b> Int32 4byte</li>
 * <li> <b>Starting offset:</b> Int32 4byte </li>
 * </ul>
 */
public class PSPv1FileInfo {
	public String name;
	public int size;
	public int offset;
	
	public PSPv1FileInfo(){
		name = "";
		size = 0;
		offset = 0;
	}
	
	/**
	 * Contains informations about a file inside the PSP FileSystem v1.
	 * @see NisFileInfo
	 *
	 * @param bitname byte[] Name of the file.
	 * @param filesize int Data lenght.
	 * @param fileoffset int Starting offset.
	 */
	public PSPv1FileInfo(byte bitname[],int filesize,int fileoffset){
		setName(bitname);
		size = filesize;
		offset = fileoffset;
	}
	
	/**
	 * Converts byte array into a UTF-8 String.
	 * @param bitname byte[] The byte string to convert.
	 * @return true, if successful.
	 */
	public boolean setName(byte bitname[]){
		try{
			this.name = new String(bitname, "UTF-8").trim();
		}catch(UnsupportedEncodingException e){
			System.out.println (e.toString());
			System.out.println("[WARNING] Could not convert filename into UTF-8!");
			this.name = "";
			this.size = 0;
			return false;
		}
		
		if(this.name.length() == 0){
			System.out.println("[WARNING] Filename is empty!");
			return false;
		}
		
		return true;
		
	}
	
	/**
	 * @return true, if file size is not zero.
	 */
	public boolean isValid(){
		return size != 0;
	}
	
    @Override 
    public String toString() {
       return name;
    }
    
    /**
     * Get the end offset of the file (start offset+size).
     * @return End offset of file.
     */
    public int getEndOffset(){
    	return offset+size;
    }
}
