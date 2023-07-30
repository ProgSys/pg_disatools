# Unofficial tools for Disgaea PC

## Builds

Please make a backup of your files before using this tools!

You can download the latest version of the tools here:


### Newest version
* **[Version v0.9.5 alpha](https://www.dropbox.com/s/57iz7xgkrnrm5oz/Disgaea%20Tools%20v0.9.5%20alpha.zip?dl=0)**


[Last change log](https://github.com/ProgSys/pg_disatools/blob/master/changelog.txt):
```
===  9.05.2020 - v0.8.8 ===
  * Sprite Sheet Editor v0.4.5 alpha:
	- Reworked "sprite sheet view" navigation
		- It now uses a Flickable instead of an ScrollArea
		- You can move the view via holding right mouse button, like in 3D view
		- You can zoom via mouse wheel, like in 3D view. Also zoom has far bigger range
		- Made resize handle easier to select (it will also become bigger on zoom in)
		- Selected cutouts now have interaction priority 
		- Improved undo on resize
	- Fixed rendering bug when adding or removing sprite sheets
  * Disa Map Viewer v0.0.3 alpha:
	- Improved model interpretation
===  10.05.2020 - v0.8.9 ===
  * Disa Map Viewer v0.0.4 alpha:
	- Added basic animation support
	- Added billboard support
        - Background is now dark
===  28.05.2020 - v0.9.0 ===
  * Disa Map Viewer v0.0.6 alpha:
	- Animations are now almost the same as in game
	- Added animated texture support
		- Note: Texture scroll speed and direction is defined inside map files, so if you just open a model the animation will be wrong
	- You can now open DATA.DAT to view all maps inside
	- Added interpretation for models that are marked hidden
		- You can display hidden models via a toggle button
  * Sprite Sheet Editor v0.4.6 alpha
	- Fixed crash when sprite id is set too high
	- Fixed crash when opening an .SH where a cutout has an invalid sheet index
===  11.06.2020 - v0.9.1 ===
  * Disa PC File Manager v0.5.9 alpha:
	- Fixed a case where the save button would not enable
	- Added the ability to export a metafest file
		- It contains meta information of each file like file size and md5 checksum 
  * Disa Map Viewer v0.0.7 alpha:
	- Improved performance
	- Scrollbar stays active when mouse is moved outside window
===  11.07.2020 - v0.9.2 ===
  * Disa PC File Manager v0.6.0 alpha:
	- Fixed .OGG play option not displaying in context menu
	- Fixed file name display in file info box
===  01.08.2020 - v0.9.3 ===
  * Disa Map Viewer v0.0.8 alpha:
	- Fixed map chunk parsing. No tiles should be missing.
	- Added Geo Panels interpretation
		- You can enable Geo Panels with a toggle button
        - Improved performance
===  27.12.2020 - v0.9.4 ===
  * General:
	- Updated Qt to 5.15.1
  * Disa PC File Manager v0.6.1 alpha:
	- Added file support for PS_FS_V1
  * Disa Map Viewer v0.0.9 alpha:
	- You can now display actor positions (like enemies)
===  30.07.2023 - v0.9.5 ===
  * Disa PC File Manager v0.6.2 alpha:
	- Better support for NEW .MPP files
  * TX2 Editor v0.0.4 alpha:
	- Added basic support for DXT4 textures
  * Disa Map Viewer v0.1.0 alpha
	- Added a toggle button to switch to the HD version of a map when a DATA.DAT file is open
	- Added a search field 
	- Several improvements to the .gltf exporter
```

### Older versions:
* [Version v0.3.0 alpha](https://www.dropbox.com/s/yraau190k8xia0i/Disgaea%20Tools%20v0.3%20alpha.zip?dl=0)
* [Version v0.4.0 alpha](https://www.dropbox.com/s/8epdstlt7e2v0w1/Disgaea%20Tools%20v0.4%20alpha.zip?dl=0)
* [Version v0.5.0 alpha](https://www.dropbox.com/s/6iuq1wh5x16jhpy/Disgaea%20Tools%20v0.5.0%20alpha.zip?dl=0)
* [Version v0.6.1 alpha](https://www.dropbox.com/s/ujg62ajrh8l86ss/Disgaea%20Tools%20v0.6.1%20alpha.zip?dl=0)
* [Version v0.6.7 alpha](https://www.dropbox.com/s/2lgtov7n53ychaa/Disgaea%20Tools%20v0.6.7%20alpha.zip?dl=0)
* [Version v0.6.8 alpha](https://www.dropbox.com/s/k7s9hjm6igsrq1r/Disgaea%20Tools%20v0.6.8%20alpha.zip?dl=0)
* [Version v0.7.2 alpha](https://www.dropbox.com/s/hc8bp3sat5wo1iz/Disgaea%20Tools%20v0.7.2%20alpha.zip?dl=0)
* [Version v0.7.9 alpha](https://www.dropbox.com/s/zxbocw141h5k4nx/Disgaea%20Tools%20v0.7.9%20alpha.zip?dl=0)
* [Version v0.8.3 alpha](https://www.dropbox.com/s/8jzpcqloz04n7h3/Disgaea%20Tools%20v0.8.3%20alpha.zip?dl=0)
* [Version v0.8.5 alpha](https://www.dropbox.com/s/l1tfcnrbqire384/Disgaea%20Tools%20v0.8.5%20alpha.zip?dl=0)
* [Version v0.8.7 alpha](https://www.dropbox.com/s/l1tfcnrbqire384/Disgaea%20Tools%20v0.8.7%20alpha.zip?dl=0)
* [Version v0.8.9 alpha](https://www.dropbox.com/s/dhy0bwv35wu5gnm/Disgaea%20Tools%20v0.8.9%20alpha.zip?dl=0)
* [Version v0.9.0 alpha](https://www.dropbox.com/s/ufcerz9cm2lcqi7/Disgaea%20Tools%20v0.9.0%20alpha.zip?dl=0)
* [Version v0.9.2 alpha](https://www.dropbox.com/s/l4fsbvxp3vry0ei/Disgaea%20Tools%20v0.9.2%20alpha.zip?dl=0)
* [Version v0.9.3 alpha](https://www.dropbox.com/s/px3jhlqee9bhcqj/Disgaea%20Tools%20v0.9.3%20alpha.zip?dl=0)
* [Version v0.9.4 alpha](https://www.dropbox.com/s/9gz829c4eh2yf23/Disgaea%20Tools%20v0.9.4%20alpha.zip?dl=0)
---


## Content

#### Disa PC File Manager

This gui application allows you to extract and insert files into Disgaea PC .DAT and .MPP files.
Note that the position of most files in .DAT are hardcoded, deleting files could break the file order and so the archive itself.


![gui image](https://github.com/ProgSys/pg_disatools/blob/master/preview/File_Manager_v0.5.8.png)

#### Sprite Sheet Editor

With this tool you can view and edit Disgaea PC sprite sheets. 
And of curse export them back to the original game format.
Note, if your data becomes too big then the game will no load anymore, 
however you can expand the allocated space via the File Manager App. 
As always the App is not perfect, so if you encounter any issues, I would greatly appreciate your feedback.

![gui image](https://raw.githubusercontent.com/ProgSys/pg_disatools/master/preview/Sprite_Sheet_Editor.png)

#### Data Editor

This tool allows you to edit some .DAT files.
The file formats are not hardcoded you can easily edit them or add new formats.
You can find the definition files at `resources/dataFiles`.

Base supported files:

* [DUNGEON.DAT](https://github.com/ProgSys/pg_disatools/wiki/DUNGEON.DAT) - Contains all maps, that can appear in the menu of the teleport lady. 
* [HOSPITAL.DAT](https://github.com/ProgSys/pg_disatools/wiki/HOSPITAL.DAT) - Contains the conditions you need to achieve to get the rewards from the hospital. 
* [CHAR_C.DAT](https://github.com/ProgSys/pg_disatools/wiki/CHAR.DAT) - Contains all the characters and their attributes. 
* CHAR_C -  Like CHAR just used unicode strings.
* CHAR_K -  Like CHAR just used unicode strings.
* MUSICSHOP - Contains all the music you can buy.
* MAGIC - Contains all the spells and their attributes.
* HABIT - Contains job titles and their attributes.
* CHARHELP - Contains descriptions of all characters.

![gui image](https://raw.githubusercontent.com/ProgSys/pg_disatools/master/preview/Data_Editor_v0.0.6_alpha_preview.png)


#### TX2 Editor

This tool allows you to open, view, convert and export TX2 images.

![gui image](https://github.com/ProgSys/pg_disatools/blob/master/preview/TX2%20Editor%20v0.0.1%20alpha_preview.png)

#### Map Viewer

Allows you to view maps and export them as GTLF.

![gui image](https://raw.githubusercontent.com/ProgSys/pg_disatools/master/preview/Map_Editor_v0.0.8_preview.png)

# Disclaimer && License
This tools are unofficial and they are not perfect and may have errors. I do not work for Nippon Ichi Software, Inc. or NIS America, Inc. . 

## QT projects
All the QT projects (whith use the Qt Librarys), require the GNU LESSER GENERAL PUBLIC LICENSE.
The source code of these projects are located in the folder 'src'

### GNU Lesser General Public License (LGPL)

	Copyright (C) 2016  ProgSys

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, version 3 of the License.

	This program is distributed in the hope that it will be useful,
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see http://doc.qt.io/qt-5/lgpl.html
	or http://www.gnu.org/licenses/
