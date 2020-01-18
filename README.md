# Unofficial tools for Disgaea PC

## Builds

Please make a backup of your files before using this tools!

You can download the latest version of the tools here:


### Newest version
* **[Version v0.8.5 alpha](https://www.dropbox.com/s/l1tfcnrbqire384/Disgaea%20Tools%20v0.8.5%20alpha.zip?dl=0)**


[Last change log](https://github.com/ProgSys/pg_disatools/blob/master/changelog.txt):
```
===  05.04.2019 - v0.8.1 ===
  * General:
	- All tools are now 64bit
	- Updated Qt to 5.12.2
  * Disa PC File Manager v0.5.6 alpha:
	- Fixed bug in openning and saving DATA.DAT
	- Fixed bug in decompressing IMY
	- Added search field
  * TX2 Editor v0.0.3 alpha:
	- You now can convert to FONT (BGRA8888 16 Colors)
===  04.12.2019 - v0.8.2 ===
  * Disa PC File Manager v0.5.7 alpha:
	- Added utility to modify memory allocation (Thx to Kelebek1) 
===  12.01.2020 - v0.8.3 ===
  * General:
	- Updated Qt to 5.12.6
  * Sprite Sheet Editor v0.4.1 alpha:
	- Weapons now can be properly edited
		- External sprite sheets are now included in the sprite sheet list 
		- External sprite sheets have a open button to open a sprite sheet as preview (.sh or .pgs)
		- Additional external sprite sheets can be added, but only one for each external ID
	- Added a toggle to isolate a selected sprite
	- Added a dropdown for sprite selection
	- Fixed crash exporting 1001_FILE.SH
	- Added new icons
	- Added tooltips
===  13.01.2020 - v0.8.4 ===
  * Sprite Sheet Editor v0.4.2 alpha:
	- Added ability to copy and paste color sets in color table view to clipboard
  * Disa PC File Manager v0.5.8 alpha:
	- Sprite sheets now have a image preview
===  13.01.2020 - v0.8.5 ===
  * Sprite Sheet Editor v0.4.3 alpha:
	- A hovered color in color table view will now also highlight that color in sprite view
	- You can now use your mouse wheel to modify the values inside text fields
	- Added basic undo (CTRL-Z). Note: it is basic.
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
* [Version v0.8.4 alpha](https://www.dropbox.com/s/6502gpkox1rddz5/Disgaea%20Tools%20v0.8.4%20alpha.zip?dl=0)
---

[![paypal](https://www.paypalobjects.com/de_DE/DE/i/btn/btn_donate_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=CD32DJ9YJKCUQ)


## Content

#### Disa PC File Manager

This gui application allows you to extract and insert files into Disgaea PC .DAT and .MPP files.
Note that the position of most files in .DAT are hardcoded, deleting files could break the file order and so the archive itself.


![gui image](https://github.com/ProgSys/pg_disatools/blob/master/preview/File_Manager_v0.5.8.png)

#### Sprite Sheet Editor

This gui application allows you to view and perform some basic editing to Disgaea PC sprite sheets.
You can export them back to the original game format to view them ingame, but this function is currently not fully tested, 
so if there are any problems let me know.
Also you can export the sprites as .png or .tga and import them back!

![gui image](https://github.com/ProgSys/pg_disatools/blob/master/preview/Sprite_Sheet_Editor_v0.4.1_preview.png)

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

![gui image](https://github.com/ProgSys/pg_disatools/blob/master/preview/Data%20Editor%20v0.0.5%20alpha_preview.png)


#### TX2 Editor

This tool allows you to open, view, convert and export TX2 images.

![gui image](https://github.com/ProgSys/pg_disatools/blob/master/preview/TX2%20Editor%20v0.0.1%20alpha_preview.png)

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
