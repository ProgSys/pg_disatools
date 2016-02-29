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