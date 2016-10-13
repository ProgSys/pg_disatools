# Unofficial tools for Disgaea PC

## Builds

Please make a backup of your files before using this tools!

You can download the latest version of the tools here:

###Newest version
* **[Version v0.7.3 alpha](https://dl.dropboxusercontent.com/u/33065226/disgaea/Disgaea%20Tools%20v0.7.3%20alpha.zip)**

[Last change log](https://github.com/ProgSys/pg_disatools/blob/master/changelog.txt):
```
===  13.10.2016 - v0.7.3 ===
  * Disa PC File Manager v0.5.3 alpha:
	- added some very basic file support for Disgaea 2 files
  * Sprite Sheet Editor v0.3.4 alpha:
	- fixed crash on import color sheet and color table being too small
	- out of bound color tables indexes are now displayed as pink
	- updated performance for adding/removing color table sets
```

###Older versions:
* [Version v0.3.0 alpha](https://dl.dropboxusercontent.com/u/33065226/disgaea/Disgaea%20Tools%20v0.3%20alpha.zip)
* [Version v0.4.0 alpha](https://dl.dropboxusercontent.com/u/33065226/disgaea/Disgaea%20Tools%20v0.4%20alpha.zip)
* [Version v0.5.0 alpha](https://dl.dropboxusercontent.com/u/33065226/disgaea/Disgaea%20Tools%20v0.5.0%20alpha.zip)
* [Version v0.6.1 alpha](https://dl.dropboxusercontent.com/u/33065226/disgaea/Disgaea%20Tools%20v0.6.1%20alpha.zip)
* [Version v0.6.7 alpha](https://dl.dropboxusercontent.com/u/33065226/disgaea/Disgaea%20Tools%20v0.6.7%20alpha.zip)
* [Version v0.6.8 alpha](https://dl.dropboxusercontent.com/u/33065226/disgaea/Disgaea%20Tools%20v0.6.8%20alpha.zip)
* [Version v0.7.2 alpha](https://dl.dropboxusercontent.com/u/33065226/disgaea/Disgaea%20Tools%20v0.7.2%20alpha.zip)

---

[![paypal](https://www.paypalobjects.com/de_DE/DE/i/btn/btn_donate_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=CD32DJ9YJKCUQ)


##Content
#### Disa PC File Manager

This gui application allows you to extract and insert files into Disgaea PC .DAT and .MPP files.
Note that the position of most files in .DAT are hardcoded, deleting files could break the file order and so the archive itself.

![gui image](https://dl.dropboxusercontent.com/u/33065226/disgaea/java_gui_preview7.png)

#### Sprite Sheet Editor

This gui application allows you to view and perform some basic editing to Disgaea PC sprite sheets.
You can export them back to the original game format to view them ingame, but this function is currently not fully tested, 
so if there are any problems let me know.
Also you can export the sprites as .png or .tga and import them back!

![gui image](https://dl.dropboxusercontent.com/u/33065226/disgaea/Sprite_Sheet_Editor_v0.2.8_preview.png)

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

![gui image](https://dl.dropboxusercontent.com/u/33065226/disgaea/Data%20Editor%20v0.0.5%20alpha_preview.png)


#### TX2 Editor

This tool allows you to open, view, convert and export TX2 images.

![gui image](https://dl.dropboxusercontent.com/u/33065226/disgaea/TX2%20Editor%20v0.0.1%20alpha_preview.png)

#### TX2Converter.exe

A command line tool which allows you to convert Disgaea PC *.TX2 textures into *.TGA (BGRA8888) or *.PNM (P3/P6 RBG888 No alpha) and back!
Make a backup of your files before using this tool!!!

# Disclaimer && License
This tools are unofficial and they are not perfect and may have errors. I do not work for Nippon Ichi Software, Inc. or NIS America, Inc. . 

## QT projects
All the QT projects (whith use the Qt Librarys), requires the GNU LESSER GENERAL PUBLIC LICENSE.
The source code of these projects are located in the folder 'pg_disatools_gui'

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

 
## TX2Converter.exe && libpg_disatools.dll
My libraries and command line tools use the MIT License.
The source code is located in the folder 'pg_disatools'

### The MIT License (MIT)

	Copyright (c) 2016 ProgSys

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.