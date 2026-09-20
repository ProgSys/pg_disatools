# Unofficial tools for Disgaea PC

## Builds

Please make a backup of your files before using the tools!

You can download the latest version of the tools here:


# Newest version
* **[Version v1.0.8 alpha](https://www.dropbox.com/scl/fi/wynb5gtmqyb397iw515yj/Disgaea-Tools-v1.0.8-alpha.zip?rlkey=lpsdggr08lq8dncotyld6jf9h&dl=0)**

[Last change log](https://github.com/ProgSys/pg_disatools/blob/master/changelog.txt):
```
===  12.10.2025 - v1.0.6  ===
 * Disa Map Editor v0.3.6 alpha
 	- Fixed z Scale not working correctly in inspector
	- Added duplicate button for selected object in inspector
 * Disa Mod Manager v0.3.6 alpha
    - Added check if "Microsoft Visual C++ Redistributable" for x86 (32bit) are installed
	- Improved installed version check
 * DisaReverseEngineering v4.5.0
	- Fixed level setting when adding character via terminal
	- Properly implemented the ability to add new magic/skills
	- Added new command to print all loaded characters
	
===  12.10.2025 - v1.0.7  ===
 * Disa Map Editor v0.3.7 alpha
	- Fixed the ability to throw units into oblivion
	- Fixed the ability to select stuff with right click
 * DisaReverseEngineering v4.6.0
	- You can now set the mentor with the AddCharacter (ac) command
	- Fixed derived units not copying skills
	
===  14.10.2025 - v1.0.8  ===
 * Disa Map Editor v0.3.8 alpha
	- Fixed some gizmos not visible in object mode
 * DisaReverseEngineering v4.6.1
	- Updated recruitment, see "Adding-Custom-Characters" Wiki for more info!
	    - Changed "isRecruitable" to "recruitmentType", allowing for more precise settings.
		- Changed "unknownValue0x83" to "manaCostScale", allowing to scale the mana cost of a unit in dark assembly
```

## Content

#### Disa PC File Manager

This gui application allows you to extract and insert files into Disgaea PC .DAT and .MPP files.
Note that the position of most files in .DAT are hardcoded, deleting files could break the file order and so the archive itself.

![gui image](https://github.com/ProgSys/pg_disatools/blob/master/preview/File_Manager_v0.5.8.png)

#### Disa Map Editor

[Disa Map Editor](https://github.com/ProgSys/pg_disatools/wiki/Map-Editor) allows you to not only modify existing maps but also create brand-new ones to share with others. 
To achieve this, a custom "Campaigns" menu has been seamlessly added to the game through partial reverse engineering.

![gui image](https://raw.githubusercontent.com/ProgSys/pg_disatools/master/preview/Map_Editor_v0.2.9_preview.png)

#### Sprite Sheet Editor

This tool lets you view and edit sprite sheets, and you can easily export them back to the original game format.

Keep in mind that if your edited data becomes too large, the game might not load correctly. However, you **can expand** the allocated space using the File Manager.

As always, the App isn't perfect, so if you encounter any issues, I would greatly appreciate your feedback!

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
### Pre release versions:
* [Version v1.0.9 alpha](https://www.dropbox.com/scl/fi/uipzx2w36hg75eqaj5bfs/Disgaea-Tools-v1.0.9-alpha-prerelese.zip?rlkey=yyy0wuqfz3lmsyex039sx7amv&dl=0)

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
* [Version v0.9.4 alpha](https://www.dropbox.com/s/9gz829c4eh2yf23/Disgaea%20Tools%20v0.9.4%20alpha.zip?dl=0)
* [Version v0.9.8 alpha](https://www.dropbox.com/scl/fi/b9hr5rtc42rp9x7ggtbvu/Disgaea-Tools-v0.9.8-alpha.zip?rlkey=tsu3fgzowyickizq1fzyqe5m3&dl=0)
* [Version v1.0.0 alpha](https://www.dropbox.com/scl/fi/ndlkk49s16b2mb2vjbw2g/Disgaea-Tools-v1.0.0-alpha.zip?rlkey=fqpdx69tru4v9dvmvcnmppnf0&dl=0)
* [Version v1.0.1 alpha](https://www.dropbox.com/scl/fi/1j0ndx9r5myiksbdxygpd/Disgaea-Tools-v1.0.1-alpha.zip?rlkey=5qgksppysog8owe7y6r6u5893&dl=0)
* [Version v1.0.2 alpha](https://www.dropbox.com/scl/fi/l0h4fjgcx4qp2kdnf0rog/Disgaea-Tools-v1.0.2-alpha.zip?rlkey=rj66s7ehzcprqnxsj1lkn7bsd&dl=0)
* [Version v1.0.5 alpha](https://www.dropbox.com/scl/fi/zg0t1gk6lk7rc58wjsrfo/Disgaea-Tools-v1.0.5-alpha.zip?rlkey=towf71mk682dg4zxq6xsou6bz&dl=0)
* [Version v1.0.6 alpha](https://www.dropbox.com/scl/fi/feh0yqdqg4vrs5p885nej/Disgaea-Tools-v1.0.6-alpha.zip?rlkey=l1e4aghzwi78sasgbblxxdnbp&dl=0)
* [Version v1.0.7 alpha](https://www.dropbox.com/scl/fi/9ta6s49vpelchnfml4f1k/Disgaea-Tools-v1.0.7-alpha.zip?rlkey=xt93bageykcldlbiclp2efphu&dl=0)
---

# Disclaimer && License
This tools are unofficial and they are not perfect and may have errors. I do not work for Nippon Ichi Software, Inc. or NIS America, Inc. . 

## QT projects
All the QT projects (whith use the Qt Librarys), require the GNU LESSER GENERAL PUBLIC LICENSE.
The source code of these projects are located in the folder 'src'

### GNU Lesser General Public License (LGPL)

	Copyright (C) 2024  ProgSys

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, version 3 of the License.

	This program is distributed in the hope that it will be useful,
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see http://doc.qt.io/qt-5/lgpl.html
	or http://www.gnu.org/licenses/
