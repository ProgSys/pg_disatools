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
import java.awt.EventQueue;

import javax.swing.JFrame;

import java.awt.FlowLayout;

import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.JTree;
import javax.swing.JButton;

import java.awt.GridLayout;
import java.awt.Component;

import javax.swing.Box;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.ItemEvent;

import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreePath;
import javax.swing.border.BevelBorder;
import javax.swing.JScrollPane;
import javax.swing.JPanel;

import java.awt.Color;
import java.awt.Panel;
import java.util.Enumeration;

import pspfs_v1.PSPv1Reader;

import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;
import javax.swing.BoxLayout;

import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.io.File;

import javax.swing.event.TreeSelectionListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.SwingConstants;

import com.sun.jna.Memory;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.PointerByReference;

public class MainWindow {

	private PSPv1Reader nis;
	private JFrame frmDisgaeaPcFile;
	JTree file_tree;
	DefaultMutableTreeNode tree;
	JLabel lbInfo;
	JCheckBox chckbxtx;
	JCheckBox chckbxmpp;
	JCheckBox chckbxogg;
	JCheckBox chckbxarc;
	JCheckBox chckbxdat;
	JCheckBox chckbxfad;
	JCheckBox chckbxffm;
	JCheckBox chckbxmpd;
	JCheckBox chckbxpbd;
	JCheckBox chckbxpng;
	JCheckBox chckbxphd;
	
	JButton btnExtract;
	JButton btnInsertFiles;
	JButton btnDeleteSelected;
	
	ImagePanel preview;
	private JLabel lbdllError;
	
	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					MainWindow window = new MainWindow();
					window.frmDisgaeaPcFile.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the application.
	 */
	public MainWindow() {
		nis = null;
		tree = null;
		initialize();
	}

	/**
	 * Initialize the contents of the frame.
	 */
	private void initialize() {
		frmDisgaeaPcFile = new JFrame();
		frmDisgaeaPcFile.setTitle("Disgaea PC File extractor v0.1 alpha");
		frmDisgaeaPcFile.setResizable(false);
		frmDisgaeaPcFile.setBounds(100, 100, 440, 590);
		frmDisgaeaPcFile.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frmDisgaeaPcFile.getContentPane().setLayout(null);
		
		btnExtract = new JButton("Extract selected");
		btnExtract.setEnabled(false);
		btnExtract.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				TreePath[] nodes = file_tree.getSelectionPaths();
			    
			    if (nis != null && nodes != null & nodes.length > 0){
					//extract files
					JFileChooser chooser = new JFileChooser(); 
					chooser.setCurrentDirectory(new java.io.File("."));
					chooser.setDialogTitle("Target Folder");
					chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
					chooser.setAcceptAllFileFilterUsed(false);
	 
					if (chooser.showOpenDialog(null) == JFileChooser.APPROVE_OPTION) {
							System.out.println("getCurrentDirectory(): " +  chooser.getCurrentDirectory());
							System.out.println("getSelectedFile() : "  +  chooser.getSelectedFile());
						
							nis.extractFiles(nodes, chooser.getSelectedFile().getAbsolutePath());
							lbInfo.setText("Files extracted to "+chooser.getSelectedFile().getAbsolutePath());
					} else {
						  lbInfo.setText("No target folder selected!");
						  System.out.println("No target folder selected!");
					 }
			 
			    }
			}

		});
		btnExtract.setBounds(277, 293, 150, 23);
		frmDisgaeaPcFile.getContentPane().add(btnExtract);
		
		btnInsertFiles = new JButton("Insert files");
		btnInsertFiles.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (nis != null){
					//open Data file
					 JFileChooser openFile = new JFileChooser();
					 openFile.setMultiSelectionEnabled(true);
					 openFile.showOpenDialog(null);
					 File[] files = openFile.getSelectedFiles();
					 lbInfo.setText("Please Wait!");
					 
					 if(files.length == 0){
						 lbInfo.setText("No files selected!");
						 return;
					 }else{
						 for(File f: files){
							 System.out.println("File to insert: "+ f.getPath());
						 }
						 nis.add(files);
						 buildFileTree();
						 lbInfo.setText(nis.getNumberOfFiles()+" files inside.");
					 }
		             
				}else{
					 lbInfo.setText("Please open a data file first!");
				}
			}
		});
		btnInsertFiles.setEnabled(false);
		btnInsertFiles.setBounds(277, 319, 150, 23);
		frmDisgaeaPcFile.getContentPane().add(btnInsertFiles);
		
		JLabel lblFilters = new JLabel("Filters:");
		lblFilters.setBounds(277, 71, 46, 14);
		frmDisgaeaPcFile.getContentPane().add(lblFilters);
		
		JButton btnOpenData = new JButton("Open Data");
		btnOpenData.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				//open Data file
				 JFileChooser openFile = new JFileChooser();
				 int success = openFile.showOpenDialog(null);
				 if(success != JFileChooser.APPROVE_OPTION)
					 return;
				 
	             String filepath = openFile.getSelectedFile().getPath();
	             
	             nis = new PSPv1Reader(filepath);
	             if(nis.getNumberOfFiles() == 0){
	            	 //display error
	            	 nis = null;
	            	 lbInfo.setText("Couldn't read file!");
	            	 
	            	 file_tree.setEnabled(false);
	            	 btnInsertFiles.setEnabled(false);
	            	 btnDeleteSelected.setEnabled(false);
	            	 file_tree.setModel(new DefaultTreeModel(
	            				new DefaultMutableTreeNode("Please open a DATA.DAT archive") {
	            					{
	            					}
	            				}
	            			));
	            	 
	            	 return;
	             }else{
	            	 lbInfo.setText(nis.getNumberOfFiles()+" files inside.");
	             }
	             buildFileTree();	 
	             file_tree.setEnabled(true);
	             btnInsertFiles.setEnabled(true);
			}
		});
		btnOpenData.setBounds(277, 10, 150, 23);
		frmDisgaeaPcFile.getContentPane().add(btnOpenData);
		
		JScrollPane scrollPane = new JScrollPane();
		scrollPane.setBounds(10, 10, 255, 540);
		frmDisgaeaPcFile.getContentPane().add(scrollPane);
		
		file_tree = new JTree();
		file_tree.addTreeSelectionListener(new TreeSelectionListener() {
			public void valueChanged(TreeSelectionEvent arg0) {
				TreePath[] nodes = file_tree.getSelectionPaths();
				    
				    if (nodes == null || nodes.length == 0){  //Nothing is selected.  
				    	btnExtract.setEnabled(false);
				    	btnDeleteSelected.setEnabled(false);
				    }else{
				    	if(nis != null){
				    		btnExtract.setEnabled(true);
				    		btnDeleteSelected.setEnabled(true);
				    	
					    	String name = nodes[0].getLastPathComponent().toString();
					    	String ext = name.substring(name.lastIndexOf('.'));
					    	if(ext.equals(".TX2")){
					    		PointerByReference p = new PointerByReference();
					    		int size = nis.extractMemory(name, p);
					    		if(size > 16){
					    			preview.displayTX2Image(p.getValue(), size);
					    		}else{
					    			preview.clearImage();
					    		}	
					    	}else{
					    		preview.clearImage();
					    	}
					    	
			    			preview.validate();
			    			preview.repaint();
				    	}
				    	
				    }
			}
		});
		file_tree.setModel(new DefaultTreeModel(
			new DefaultMutableTreeNode("Please open a DATA.DAT archive") {
				{
				}
			}
		));
		file_tree.setBorder(null);
		file_tree.setEnabled(false);
		scrollPane.setViewportView(file_tree);
		
		btnDeleteSelected = new JButton("Delete selected");
		btnDeleteSelected.setEnabled(false);
		btnDeleteSelected.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				TreePath[] nodes = file_tree.getSelectionPaths();
			    
			    if (nis != null && nodes != null & nodes.length > 0){
			    	lbInfo.setText("Please Wait!");
			    	nis.delete(nodes);
			    	buildFileTree();
			    }
			    lbInfo.setText(nis.getNumberOfFiles()+" files inside.");
			}

		});
		btnDeleteSelected.setBounds(277, 371, 150, 23);
		frmDisgaeaPcFile.getContentPane().add(btnDeleteSelected);
		
		lbdllError = new JLabel("");
		
		preview = new ImagePanel(lbdllError);
		preview.setBackground(Color.GRAY);
		preview.setBounds(277, 400, 150, 150);
		frmDisgaeaPcFile.getContentPane().add(preview);
		preview.add(lbdllError);
		preview.addMouseListener(new PopClickListener(preview));
		
		JPanel panel = new JPanel();
		panel.setBounds(277, 87, 153, 169);
		frmDisgaeaPcFile.getContentPane().add(panel);
		GridBagLayout gbl_panel = new GridBagLayout();
		gbl_panel.columnWidths = new int[]{0, 0, 0};
		gbl_panel.rowHeights = new int[]{0, 0, 0, 0, 0, 0, 0};
		gbl_panel.columnWeights = new double[]{0.0, 0.0, Double.MIN_VALUE};
		gbl_panel.rowWeights = new double[]{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Double.MIN_VALUE};
		panel.setLayout(gbl_panel);
		
		chckbxarc = new JCheckBox("*.ARC");
		GridBagConstraints gbc_chckbxarc = new GridBagConstraints();
		gbc_chckbxarc.insets = new Insets(0, 0, 5, 5);
		gbc_chckbxarc.gridx = 0;
		gbc_chckbxarc.gridy = 0;
		panel.add(chckbxarc, gbc_chckbxarc);
		chckbxarc.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				 buildFileTree();	
			}
		});
		chckbxarc.setSelected(true);
		
		chckbxdat = new JCheckBox("*.DAT");
		GridBagConstraints gbc_chckbxdat = new GridBagConstraints();
		gbc_chckbxdat.insets = new Insets(0, 0, 5, 0);
		gbc_chckbxdat.gridx = 1;
		gbc_chckbxdat.gridy = 0;
		panel.add(chckbxdat, gbc_chckbxdat);
		chckbxdat.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				 buildFileTree();	
			}
		});
		chckbxdat.setSelected(true);
		
		chckbxfad = new JCheckBox("*.FAD");
		GridBagConstraints gbc_chckbxfad = new GridBagConstraints();
		gbc_chckbxfad.insets = new Insets(0, 0, 5, 5);
		gbc_chckbxfad.gridx = 0;
		gbc_chckbxfad.gridy = 1;
		panel.add(chckbxfad, gbc_chckbxfad);
		chckbxfad.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				 buildFileTree();	
			}
		});
		chckbxfad.setSelected(true);
		
		chckbxffm = new JCheckBox("*.FFM");
		GridBagConstraints gbc_chckbxffm = new GridBagConstraints();
		gbc_chckbxffm.insets = new Insets(0, 0, 5, 0);
		gbc_chckbxffm.gridx = 1;
		gbc_chckbxffm.gridy = 1;
		panel.add(chckbxffm, gbc_chckbxffm);
		chckbxffm.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				 buildFileTree();	
			}
		});
		chckbxffm.setSelected(true);
		
		chckbxmpd = new JCheckBox("*.MPD");
		GridBagConstraints gbc_chckbxmpd = new GridBagConstraints();
		gbc_chckbxmpd.insets = new Insets(0, 0, 5, 5);
		gbc_chckbxmpd.gridx = 0;
		gbc_chckbxmpd.gridy = 2;
		panel.add(chckbxmpd, gbc_chckbxmpd);
		chckbxmpd.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				 buildFileTree();	
			}
		});
		chckbxmpd.setSelected(true);
		
		chckbxmpp = new JCheckBox("*.MPP");
		GridBagConstraints gbc_chckbxmpp = new GridBagConstraints();
		gbc_chckbxmpp.insets = new Insets(0, 0, 5, 0);
		gbc_chckbxmpp.gridx = 1;
		gbc_chckbxmpp.gridy = 2;
		panel.add(chckbxmpp, gbc_chckbxmpp);
		chckbxmpp.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				if(nis != null){
					tree = new DefaultMutableTreeNode("Files");
					createTreeModel(tree);
					file_tree.setModel(new DefaultTreeModel( tree));
				}
			}
		});
		chckbxmpp.setSelected(true);
		
		chckbxogg = new JCheckBox("*.OGG");
		GridBagConstraints gbc_chckbxogg = new GridBagConstraints();
		gbc_chckbxogg.insets = new Insets(0, 0, 5, 5);
		gbc_chckbxogg.gridx = 0;
		gbc_chckbxogg.gridy = 3;
		panel.add(chckbxogg, gbc_chckbxogg);
		chckbxogg.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				 buildFileTree();	
			}
		});
		chckbxogg.setSelected(true);
		
		chckbxpbd = new JCheckBox("*.PBD");
		GridBagConstraints gbc_chckbxpbd = new GridBagConstraints();
		gbc_chckbxpbd.insets = new Insets(0, 0, 5, 0);
		gbc_chckbxpbd.gridx = 1;
		gbc_chckbxpbd.gridy = 3;
		panel.add(chckbxpbd, gbc_chckbxpbd);
		chckbxpbd.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				 buildFileTree();	
			}
		});
		chckbxpbd.setSelected(true);
		
		chckbxpng = new JCheckBox("*.PNG");
		GridBagConstraints gbc_chckbxpng = new GridBagConstraints();
		gbc_chckbxpng.insets = new Insets(0, 0, 5, 5);
		gbc_chckbxpng.gridx = 0;
		gbc_chckbxpng.gridy = 4;
		panel.add(chckbxpng, gbc_chckbxpng);
		chckbxpng.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				 buildFileTree();	
			}
		});
		chckbxpng.setSelected(true);
		
		chckbxphd = new JCheckBox("*.PHD");
		GridBagConstraints gbc_chckbxphd = new GridBagConstraints();
		gbc_chckbxphd.insets = new Insets(0, 0, 5, 0);
		gbc_chckbxphd.gridx = 1;
		gbc_chckbxphd.gridy = 4;
		panel.add(chckbxphd, gbc_chckbxphd);
		chckbxphd.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				 buildFileTree();	
			}
		});
		chckbxphd.setSelected(true);
		
		chckbxtx = new JCheckBox("*.TX2");
		GridBagConstraints gbc_chckbxtx = new GridBagConstraints();
		gbc_chckbxtx.insets = new Insets(0, 0, 0, 5);
		gbc_chckbxtx.gridx = 0;
		gbc_chckbxtx.gridy = 5;
		panel.add(chckbxtx, gbc_chckbxtx);
		chckbxtx.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				 buildFileTree();	
			}
		});
		chckbxtx.setSelected(true);
		
		lbInfo = new JLabel("No data archive open");
		lbInfo.setVerticalAlignment(SwingConstants.TOP);
		lbInfo.setBounds(277, 35, 150, 14);
		frmDisgaeaPcFile.getContentPane().add(lbInfo);
		
		JButton btnAbout = new JButton("About");
		btnAbout.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				JOptionPane.showMessageDialog(null, 
						"This tool allows you to add and extract files form the Disgaea PC pspfs_v1 archive. \n"
						+ "\nCopyright (C) 2016  ProgSys"
						+ "\n\nThis program is free software: you can redistribute it and/or modify"
						+ "\nit under the terms of the GNU General Public License as published by"
						+ "\nthe Free Software Foundation, either version 3 of the License, or"
						+ "\n(at your option) any later version."

						+ "\n\nThis program is distributed in the hope that it will be useful,"
						+ "\nbut WITHOUT ANY WARRANTY; without even the implied warranty of"
						+ "\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
						+ "\nGNU General Public License for more details."

						+ "\n\nYou should have received a copy of the GNU General Public License"
						+ "\nalong with this program.  If not, see <http://www.gnu.org/licenses/>."
						);
			
			}
		});
		btnAbout.setBounds(277, 267, 150, 23);
		frmDisgaeaPcFile.getContentPane().add(btnAbout);
	}
	
	private void buildFileTree(){
		if(nis != null){
			tree = new DefaultMutableTreeNode("Files");
			createTreeModel(tree);
			file_tree.setModel(new DefaultTreeModel( tree));
		}
	}
	
	private void createTreeModel(DefaultMutableTreeNode treenode){
        nis.createTree(treenode,
       		 chckbxogg.isSelected(), chckbxtx.isSelected(), chckbxmpp.isSelected(),
       		 chckbxarc.isSelected(), chckbxdat.isSelected(), chckbxfad.isSelected(),
       		 chckbxffm.isSelected(), chckbxmpd.isSelected(), chckbxpbd.isSelected(),
       		 chckbxpng.isSelected(), chckbxphd.isSelected());
	}
}
