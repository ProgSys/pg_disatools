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

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.io.UnsupportedEncodingException;
import java.nio.BufferUnderflowException;
import java.nio.file.DirectoryNotEmptyException;
import java.nio.file.Files;
import java.nio.file.NoSuchFileException;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreePath;

import pspfs_v1.PSPv1FileInfo;

public class PSPv1Reader {
	private String m_path;
	private int m_numberoffiles;
	private List<PSPv1FileInfo> m_fileInfos;
	
	public PSPv1Reader(){
		m_path = "";
		m_numberoffiles = 0;
		m_fileInfos = null;
	}
	
	public PSPv1Reader(String path){
		m_fileInfos = null;
		open(path);
	}
	
	public void open(String path){
		if(m_fileInfos != null)
			m_fileInfos.clear();
		
		m_path = path;
		m_numberoffiles = 0;
		
		if(m_path.length() == 0){
			System.out.println("[ERROR] Given filepath is empty!");
			return;
		}
		
		
		try {
			//Open file
			File file = new File(m_path);
			LittleEndianDataInputStream  stream = new LittleEndianDataInputStream (new FileInputStream(file));
			
			//magic number 8Byte
			if(!readString(stream,8).equals("PSPFS_V1")){
				System.out.println("[ERROR] Magic number does not equal 'PSPFS_V1'!");
				return;
			}

			//number of files
			m_numberoffiles = stream.readInt();
			stream.readInt(); //NULL
			
			//read file infos
			m_fileInfos = new ArrayList<PSPv1FileInfo>(m_numberoffiles);
			
			byte b[] = new byte[44];
			for(int i = 0; i < m_numberoffiles;i++){
				stream.read(b);
				int filesize = stream.readInt();
				int fileoffset = stream.readInt();
				m_fileInfos.add(new PSPv1FileInfo(b,filesize,fileoffset));
			}

			stream.close();
			
		} catch (BufferUnderflowException e) {
			System.out.println (e.toString());
			System.out.println("[ERROR] Could not open file: "+path);
		}catch(UnsupportedEncodingException e){
			System.out.println (e.toString());
			System.out.println("[ERROR] Couldn't convert filename into UTF-8!");
		}catch(IOException e){
			System.out.println (e.toString());
			System.out.println("[ERROR] Couldn't raed file table!");
		}
	}
	
	private String readString(LittleEndianDataInputStream in, int size) throws UnsupportedEncodingException, IOException{
		byte [] bytes = new byte[size];
		in.read(bytes);
		return new String(bytes, "UTF-8");
	}
	
	/**
	 * Extract one file from archive by its name.
	 * @param name String - Name of the file to be extracted.
	 * @param path String - Path where the file should be saved.
	 * @return true if could not be extracted or found
	 */
	public boolean extract(String name,String path){
		if(m_numberoffiles == 0) return true;
		
		//Find file
		if(name.length() == 0){
			System.out.println("[WARNING] Filename is empty, so I can't find the file!");
			return false;
		}
		
		for(PSPv1FileInfo info: m_fileInfos){
			if(info.name.equals(name)){
				return extract(info,path);
			}
		}
		
		
		return false;
	}
	
	/**
	 * Extract some files
	 * @param tree - Selected nodes from the tree menu.
	 * @param path - String where it should be saved.
	 */
	public boolean extractFiles(TreePath[] tree,String path){
		if(m_numberoffiles == 0) return true;
		
		//Find file
		if(tree.length == 0){
			System.out.println("[WARNING] Tree is empty, so I can't find the file!");
			return true;
		}
		
		int i = 0;
		for(PSPv1FileInfo info: m_fileInfos){
			for(TreePath node: tree){
   	    	  	if(node.getPathCount() > 1){
   	    	  		if(info.name.equals(node.getLastPathComponent().toString())){
   	    	  			i++;
   	    	  			System.out.println(i+"/"+tree.length);
   	    	  			extract(info,path);
   	    	  		}
   	    	  	}
			}
		}
		
		return false;
	}
	
	private boolean extract(PSPv1FileInfo info,String path){
		//TODO make this more nice, and sleep a bit more
		if(info.name.length() == 0 ){
			System.out.println("[WARNING] File has no name!");
			return false;
		}
		
		String fullPath = path+"/" + info.name;
		System.out.println("[INFO] Saving file to: "+fullPath);
		FileOutputStream out = null;
		try {
			byte b[] = new byte[info.size];
			File file = new File(m_path);
			DataInputStream stream = new DataInputStream(new FileInputStream(file));
			stream.skip(info.offset);
			stream.read(b);
			stream.close();
			
			out = new FileOutputStream(fullPath);
			out.write(b);
			out.close();
			
		}catch(IndexOutOfBoundsException e){
			System.out.println (e.toString());
			System.out.println("[ERROR] Something went wrong with the array size!");
			return true;
		}catch(IOException e){
			System.out.println (e.toString());
			System.out.println("[ERROR] Could not save file: "+fullPath);
			return true;
		}
		
		return false;
	}
	
	public boolean add(File[] filesarray){
		if(filesarray.length <= 0 || m_fileInfos == null) return true;
		
		List<File> files = new ArrayList<File>(Arrays.asList(filesarray));
		
		//calculate number of files
		int numberOfFiles = 0;
		int numberOfFilesToAdd = files.size();
		for(PSPv1FileInfo info: m_fileInfos){
			numberOfFiles++;
			for(File file: files){
				if(file.getName().equals(info.name)){
					info.size = -1; //mark for easyer searching
					numberOfFilesToAdd--;
					break;
				}
			}
		}
		numberOfFiles += numberOfFilesToAdd;
		
		File fold = new File(m_path);
		File fnew = new File(fold.getParent()+"\\DATA_BUFFER.DAT");
		
		try {
			RandomAccessFile writer = new RandomAccessFile(fnew,"rw");
			writer.writeBytes("PSPFS_V1");
			
			byte intbytes[] = new byte[4];
			
			intbytes[0] = (byte) (numberOfFiles & 0xFF);
			intbytes[1] = (byte) ((numberOfFiles >>> 8) & 0xFF);
			intbytes[2] = (byte) ((numberOfFiles >>> 16) & 0xFF);
			intbytes[3] = (byte) ((numberOfFiles >>> 24) & 0xFF);
			writer.write(intbytes); // size
			
			writer.writeInt(0); // NULL
			
			//reserve space
			byte b[] = new byte[52];
			for(int i = 0; i < numberOfFiles; i++){
				writer.write(b);
			}
			
			
			List<PSPv1FileInfo> newfileInfos = new ArrayList<PSPv1FileInfo>(numberOfFiles);
			RandomAccessFile reader = new RandomAccessFile(fold,"r");

			int lastOffset = 16+numberOfFiles*52;
			//reader.skip(16+m_fileInfos.size()*52);
			System.out.println("numberOfFiles: "+numberOfFiles+" Start offfset: "+lastOffset);
			//go over the old files
			for(PSPv1FileInfo info: m_fileInfos){
				if(info.size == -1){
					//replace a file
					//find file
					File fileToAdd = null;
					for(File file: files){
						if(file.getName().equals(info.name)){
							fileToAdd = file;
							files.remove(file);
							break;
						}
					}
					
					if(fileToAdd != null){
						DataInputStream filein = new DataInputStream(new FileInputStream(fileToAdd));
						int size = filein.available();
						System.out.println("replace: " + info.name+ " with: "+fileToAdd.getName().toUpperCase()+ " original size: "+info.size+" new size: "+size+ " offset: "+lastOffset);
						byte filebytes[] = new byte[size];
						filein.readFully(filebytes);
						filein.close();
						writer.write(filebytes);
						
						newfileInfos.add( new PSPv1FileInfo( fileToAdd.getName().toUpperCase().getBytes(), size, lastOffset ));
						lastOffset = lastOffset+size;
					}
					
				}else{
					if(info.name.equals("DUMMY.DAT")){
						//write the dummy, it is at the wrong place though
						byte filebytes[] = new byte[1396];
						filebytes[876] = 'D';
						filebytes[877] = 'U';
						filebytes[878] = 'M';
						filebytes[879] = 'M';
						filebytes[880] = 'Y';
						writer.write(filebytes);
						
						lastOffset = lastOffset+1396;
						newfileInfos.add(info);
					}else{
						//just copy old file
						byte filebytes[] = new byte[info.size];
						reader.seek(info.offset);
						reader.read(filebytes);
						writer.write(filebytes);
						
						info.offset = lastOffset;
						lastOffset = lastOffset+info.size;
						newfileInfos.add(info);
					}
				}
				
			}
			//add new files at back
			for(File file: files){
				DataInputStream filein = new DataInputStream(new FileInputStream(file));
				int size = filein.available();
				byte filebytes[] = new byte[size];
				filein.readFully(filebytes);
				filein.close();
				writer.write(filebytes);
				
				newfileInfos.add(new PSPv1FileInfo( file.getName().toUpperCase().getBytes(), size, lastOffset ));
				lastOffset = lastOffset+size;
			}
			
			reader.close();
			m_fileInfos = newfileInfos;
			
			 System.out.println("Write file table");

			 //write file table
			writer.seek(0);
			writer.skipBytes(16);
			for(PSPv1FileInfo info: m_fileInfos){
				writer.write(info.name.getBytes());
				writer.skipBytes(44-info.name.length());
				
				intbytes[0] = (byte) (info.size & 0xFF);
				intbytes[1] = (byte) ((info.size >>> 8) & 0xFF);
				intbytes[2] = (byte) ((info.size >>> 16) & 0xFF);
				intbytes[3] = (byte) ((info.size >>> 24) & 0xFF);
				
				writer.write(intbytes);
			    
				intbytes[0] = (byte) (info.offset & 0xFF);
				intbytes[1] = (byte) ((info.offset >>> 8) & 0xFF);
				intbytes[2] = (byte) ((info.offset >>> 16) & 0xFF);
				intbytes[3] = (byte) ((info.offset >>> 24) & 0xFF);
				
				writer.write(intbytes);
			}
			writer.close();
			
		}catch(IndexOutOfBoundsException e){
			System.out.println (e.toString());
			System.out.println("[ERROR] Something went wrong with the array size!");
			fnew.delete();
			return true;
		}catch(IOException e){
			System.out.println (e.toString());
			System.out.println("[ERROR] Couldn't extract files! ");
			fnew.delete();
			return true;
		}
		
		//replace old file
		if(fnew.exists()){
			fold.delete();
			fnew.renameTo(fold);
		}
		
		return false;
	}
	
	
	public boolean delete(TreePath[] tree){
		if(tree.length <= 0 || m_fileInfos == null) return true;
		
		int startFileNumber = m_fileInfos.size();
		for(TreePath node: tree){
			System.out.println("Remove: "+node.getLastPathComponent().toString());
			
			//System.out.println("[INFO] Found no file to remove!");
			Iterator<PSPv1FileInfo> i = m_fileInfos.iterator();
			while (i.hasNext()) {
				PSPv1FileInfo s = i.next();
				if(s.name.equals(node.getLastPathComponent().toString()))
					i.remove();
			}
			
		}
		if(startFileNumber == m_fileInfos.size()){
			System.out.println("[INFO] Found no file to remove!");
			return true;
		}
		
		int filetablebytes = m_fileInfos.size()*52;
		
		File fold = new File(m_path);
		File fnew = new File(fold.getParent()+"\\DATA_BUFFER.DAT");
		
		try{
			RandomAccessFile writer = new RandomAccessFile(fnew,"rw");
			
			writer.writeBytes("PSPFS_V1");
	
			byte intbytes[] = new byte[4];
			intbytes[0] = (byte) (m_fileInfos.size() & 0xFF);
			intbytes[1] = (byte) ((m_fileInfos.size() >>> 8) & 0xFF);
			intbytes[2] = (byte) ((m_fileInfos.size() >>> 16) & 0xFF);
			intbytes[3] = (byte) ((m_fileInfos.size() >>> 24) & 0xFF);
			writer.write(intbytes); // size
			
			writer.writeInt(0); // NULL
			
			//reserve space
			byte b[] = new byte[52];
			for(int i = 0; i < m_fileInfos.size(); i++){
				writer.write(b);
			}
			
			
			List<PSPv1FileInfo> newfileInfos = new ArrayList<PSPv1FileInfo>(m_fileInfos.size());
			RandomAccessFile reader = new RandomAccessFile(fold,"r");
			
			int lastOffset = 16+filetablebytes;
			for(PSPv1FileInfo info: m_fileInfos){
				if(info.name.equals("DUMMY.DAT")){
					//write the dummy, it is at the wrong place though
					byte filebytes[] = new byte[1396];
					filebytes[876] = 'D';
					filebytes[877] = 'U';
					filebytes[878] = 'M';
					filebytes[879] = 'M';
					filebytes[880] = 'Y';
					writer.write(filebytes);
					
					lastOffset = lastOffset+1396;
					newfileInfos.add(info);
				}else{
					//just copy old file
					byte filebytes[] = new byte[info.size];
					reader.seek(info.offset);
					reader.read(filebytes);
					writer.write(filebytes);
					
					info.offset = lastOffset;
					lastOffset = lastOffset+info.size;
					newfileInfos.add(info);
				}
			}
			reader.close();
			
			System.out.println("Write file table");

			 //write file table
			writer.seek(0);
			writer.skipBytes(16);
			for(PSPv1FileInfo info: m_fileInfos){
				writer.write(info.name.getBytes());
				writer.skipBytes(44-info.name.length());
				
				intbytes[0] = (byte) (info.size & 0xFF);
				intbytes[1] = (byte) ((info.size >>> 8) & 0xFF);
				intbytes[2] = (byte) ((info.size >>> 16) & 0xFF);
				intbytes[3] = (byte) ((info.size >>> 24) & 0xFF);
				
				writer.write(intbytes);
			    
				intbytes[0] = (byte) (info.offset & 0xFF);
				intbytes[1] = (byte) ((info.offset >>> 8) & 0xFF);
				intbytes[2] = (byte) ((info.offset >>> 16) & 0xFF);
				intbytes[3] = (byte) ((info.offset >>> 24) & 0xFF);
				
				writer.write(intbytes);
			}
			writer.close();
		
		}catch(IndexOutOfBoundsException e){
			System.out.println (e.toString());
			System.out.println("[ERROR] Something went wrong with the array size!");
			fnew.delete();
			return true;
		}catch(IOException e){
			System.out.println (e.toString());
			System.out.println("[ERROR] Couldn't extract files! ");
			fnew.delete();
			return true;
		}
		
		//replace old file
		if(fnew.exists()){
			fold.delete();
			fnew.renameTo(fold);
		}
		
		return false;
	}
	/**
	 * Create tree.
	 * @param top
	 */
	public boolean createTree(DefaultMutableTreeNode top,
			boolean ogg, boolean tx2, boolean mpp,
			boolean arc, boolean dat, boolean fad,
			boolean ffm, boolean mpd, boolean pbd,
			boolean png, boolean phd){
		if(m_numberoffiles == 0) return true;
		
		
		DefaultMutableTreeNode item = null;
		top.setUserObject(m_path);
		for(PSPv1FileInfo info: m_fileInfos){
			String ext = getFileExt(info.name);
			if( (ogg || !ext.equals("ogg")) 
					&&  (tx2 || !ext.equals("tx2")) 
					&&  (mpp || !ext.equals("mpp")) 
					&&  (arc || !ext.equals("arc")) 
					&&  (dat || !ext.equals("dat")) 
					&&  (fad || !ext.equals("fad")) 
					&&  (ffm || !ext.equals("ffm")) 
					&&  (mpd || !ext.equals("mpd")) 
					&&  (pbd || !ext.equals("pbd")) 
					&&  (png || !ext.equals("png")) 
					&&  (phd || !ext.equals("phd")) 
					){
				item = new DefaultMutableTreeNode(info);
				top.add(item);
			}
		}
		
		return false;
	}
	
	public boolean createTree(DefaultMutableTreeNode top){
		if(m_numberoffiles == 0) return true;
		
		
		DefaultMutableTreeNode item = null;
		top.setUserObject(m_path);
		for(PSPv1FileInfo info: m_fileInfos){
			item = new DefaultMutableTreeNode(info);
			top.add(item);
		}
		
		return false;
	}
	
	public int getNumberOfFiles(){
		return m_numberoffiles;
	}
	
	private String getFileExt(String str){
         String extension = "";

         int i = str.lastIndexOf('.');
         if (i > 0) 
             extension = str.substring(i+1);
         
         return extension.toLowerCase();
	}

	
	
	
}
