/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2020  ProgSys
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, version 3 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with this program.  If not, see http://doc.qt.io/qt-5/lgpl.html
 *	or http://www.gnu.org/licenses/
 */
#include <SpriteData.h>
#include <QException>
#include <Files/PG_SH.h>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <Util/PG_Exception.h>
#include <QInputDialog>
#include <QVector>
#include <QMutableListIterator>
#include <ImageLib.h>
#include <Util/Misc/ResourcePath.h>


inline QString readQString(QDataStream& in) {
	quint32 number_uint32;
	in >> number_uint32;
	QByteArray buffer(number_uint32, Qt::Uninitialized);
	in.readRawData(buffer.data(), number_uint32);
	return QString(buffer);
}


bool SpriteData::openPGSHv1(QDataStream& in) {
	quint16 number_uint16;
	quint32 number_uint32;

	QString text;
	text = readQString(in);//info text
	qDebug() << __LINE__ << ": " << text;
	if (text.isEmpty()) return false;
	text = readQString(in);
	if (text.isEmpty()) return false;

	QColorTable colorTable;
	in >> number_uint16;
	for (unsigned int i = 0; i < number_uint16; i++) {
		quint8 r, g, b, a;
		in >> r >> g >> b >> a;
		colorTable.push_back(QColor(r, g, b, a));
	}
	m_colortables.push_back(colorTable);

	text = readQString(in);//SpriteSheets

	in >> number_uint16;
	for (unsigned int i = 0; i < number_uint16; i++) {
		quint16 width, height, powerColorTable;
		quint32 savedSize;
		in >> width >> height >> powerColorTable >> savedSize;

		if (width == 0 || height == 0 || height > 6000 || width > 6000) {
			PG_INFO_STREAM("width: " << width << " height: " << height << " powerColorTable: " << powerColorTable)
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Sprite sheet bits are wrong? (width or height invalid)", QMessageBox::Ok);
			return false;
		}
		unsigned int size = width * height;
		if (size != savedSize) {
			PG_INFO_STREAM("size: " << size << " savedSize: " << savedSize)
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Sprite sheet size is different than expected! (width*height != stored size)", QMessageBox::Ok);
			return false;
		}

		m_spriteSheets.push_back(new SpriteSheet(-1, width, height, powerColorTable, this));
		if (size != 0) in.readRawData((char*)(m_spriteSheets.last()->getSpriteSheet().begin()), size);

	}

	text = readQString(in);//Cutouts
	in >> number_uint32;
	for (unsigned int i = 0; i < number_uint32; i++) {
		quint16 externalSheetID, sheetRealID, x, y, w, h, dc;
		in >> externalSheetID >> sheetRealID >> x >> y >> w >> h >> dc;

		m_cutouts.push_back(new Cutout(sheetRealID, PG::UTIL::ivec2(x, y), PG::UTIL::ivec2(w, h), dc, this));
		assert_Test("Sheet ID out of bound!", sheetRealID >= m_spriteSheets.size());
		m_spriteSheets[sheetRealID]->push_backCutoutID(m_cutouts.size() - 1);
	}

	text = readQString(in);//Animations
	in >> number_uint32;
	qDebug() << __LINE__ << ": Ani Size: " << number_uint32;
	beginInsertRows(QModelIndex(), 0, number_uint32);
	for (unsigned int a = 0; a < number_uint32; a++) {
		quint32 id, markers, layers;
		QString name;
		in >> id >> name >> markers;
		if (markers > 1400) {
			PG_INFO_STREAM("markers: " << markers)
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Number of markers is too big! (size > 1400)", QMessageBox::Ok);
			return false;
		}


		SpriteAnimation* ani = new SpriteAnimation(id, name);
		m_aniamtions.push_back(ani);
		for (unsigned int m = 0; m < markers; m++) {
			quint32 start, duration, type;
			qint16 a;
			quint16 b;
			in >> start >> duration >> type >> a >> b;
			//qDebug()<<__LINE__<<"_Markers: start: "<<start<<" duration: "<<duration<<" type: "<<type<<" a: "<<a<<" b: "<<b;
			assert_Test("Marker duration should be 1!", duration != 1);
			ani->getMarkers()->push_back(new Marker(start, duration, type, a, b, ani));
			//ani->addMarker(start,type,a,b);
		}

		in >> layers;
		if (layers > 1400) {
			PG_INFO_STREAM("layers: " << layers)
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Number of layers is too big! (size > 1400)", QMessageBox::Ok);
			return false;
		}
		//PG_INFO_STREAM("layers: "<<layers)
		for (unsigned int l = 0; l < layers; l++) {
			QString name;// = readQString(in);
			quint8 isHidden;
			in >> name >> isHidden;
			Layer* lay = new Layer(name, isHidden, ani);
			ani->getLayers().push_back(lay);
			quint32 keyframes;
			in >> keyframes;
			//PG_INFO_STREAM("keyframes: "<<keyframes<<" name: "<<name.toStdString())

			if (keyframes >= 3000) {
				PG_INFO_STREAM("keyframes: " << keyframes << " name: " << name.toStdString())
					QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Too many keyframes! (size > 3000)", QMessageBox::Ok);
				return false;
			}

			for (unsigned int k = 0; k < keyframes; k++) {
				quint16 cutoutID, colortableID;
				qint32 start, durr;
				qint16 anchorX, anchorY, offsetX, offsetY, scaleX, scaleY, roation, mirror, unknown;
				in >> cutoutID >> colortableID >> start >> durr >> anchorX >> anchorY >> offsetX >> offsetY >> scaleX >> scaleY >> roation >> mirror >> unknown;
				if (cutoutID >= m_cutouts.size()) {
					PG_INFO_STREAM("cutoutID: " << cutoutID << " size: " << m_cutouts.size())
						QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Keyframes cutoutID is bigger then the number of cutouts!", QMessageBox::Ok);
					return false;
				}
				lay->push_backKeyframe(start, durr, cutoutID, colortableID, anchorX, anchorY, scaleX, scaleY, offsetX, offsetY, roation, mirror, unknown, false);
			}

		}



	}
	return true;
}

bool SpriteData::openPGSHv2(QDataStream& in) {
	quint16 number_uint16;
	quint32 number_uint32;

	QString text;
	text = readQString(in);//info text
	qDebug() << __LINE__ << ": " << text;
	if (text.isEmpty()) return false;
	text = readQString(in);
	if (text.isEmpty()) return false;

	{
		quint16 numberOfColorTables;
		in >> numberOfColorTables;
		for (unsigned int c = 0; c < numberOfColorTables; c++) {
			QColorTable colorTable;
			in >> number_uint16;
			for (unsigned int i = 0; i < number_uint16; i++) {
				quint8 r, g, b, a;
				in >> r >> g >> b >> a;
				colorTable.push_back(QColor(r, g, b, a));
			}
			m_colortables.push_back(colorTable);
		}
	}

	text = readQString(in);//SpriteSheets

	in >> number_uint16;
	for (unsigned int i = 0; i < number_uint16; i++) {
		quint16 width, height, powerColorTable;
		quint32 savedSize;
		in >> width >> height >> powerColorTable >> savedSize;

		if (width == 0 || height == 0 || height > 6000 || width > 6000) {
			PG_INFO_STREAM("width: " << width << " height: " << height << " powerColorTable: " << powerColorTable)
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Sprite sheet bits are wrong? (width or height invalid)", QMessageBox::Ok);
			return false;
		}
		unsigned int size = width * height;
		if (size != savedSize) {
			PG_INFO_STREAM("size: " << size << " savedSize: " << savedSize)
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Sprite sheet size is different than expected! (width*height != stored size)", QMessageBox::Ok);
			return false;
		}

		m_spriteSheets.push_back(new SpriteSheet(-1, width, height, powerColorTable, this));
		if (size != 0) in.readRawData((char*)(m_spriteSheets.last()->getSpriteSheet().begin()), size);

	}
	//Cutouts
	text = readQString(in);
	in >> number_uint32;
	for (unsigned int i = 0; i < number_uint32; i++) {
		quint16 externalSheetID, sheetRealID, x, y, w, h, dc;
		in >> externalSheetID >> sheetRealID >> x >> y >> w >> h >> dc;

		m_cutouts.push_back(new Cutout(sheetRealID, PG::UTIL::ivec2(x, y), PG::UTIL::ivec2(w, h), dc, this));
		assert_Test("Sheet ID out of bound!", sheetRealID >= m_spriteSheets.size());
		m_spriteSheets[sheetRealID]->push_backCutoutID(m_cutouts.size() - 1);
	}

	//Animations
	text = readQString(in);
	in >> number_uint32;
	qDebug() << __LINE__ << ": Ani Size: " << number_uint32;
	beginInsertRows(QModelIndex(), 0, number_uint32);
	for (unsigned int a = 0; a < number_uint32; a++) {
		quint32 id, markers, layers;
		QString name;
		in >> id >> name >> markers;
		if (markers > 1400) {
			PG_INFO_STREAM("markers: " << markers)
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Number of markers is too big! (size > 1400)", QMessageBox::Ok);
			return false;
		}


		SpriteAnimation* ani = new SpriteAnimation(id, name);
		m_aniamtions.push_back(ani);
		for (unsigned int m = 0; m < markers; m++) {
			quint32 start, duration, type;
			qint16 a;
			quint16 b;
			in >> start >> duration >> type >> a >> b;
			//qDebug()<<__LINE__<<"_Markers: start: "<<start<<" duration: "<<duration<<" type: "<<type<<" a: "<<a<<" b: "<<b;
			assert_Test("Marker duration should be 1!", duration != 1);
			ani->getMarkers()->push_back(new Marker(start, duration, type, a, b, ani));
			//ani->addMarker(start,type,a,b);
		}

		in >> layers;
		if (layers > 1400) {
			PG_INFO_STREAM("layers: " << layers)
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Number of layers is too big! (size > 1400)", QMessageBox::Ok);
			return false;
		}
		//PG_INFO_STREAM("layers: "<<layers)
		for (unsigned int l = 0; l < layers; l++) {
			QString name;// = readQString(in);
			quint8 isHidden;
			in >> name >> isHidden;
			Layer* lay = new Layer(name, isHidden, ani);
			ani->getLayers().push_back(lay);
			quint32 keyframes;
			in >> keyframes;
			//PG_INFO_STREAM("keyframes: "<<keyframes<<" name: "<<name.toStdString())

			if (keyframes >= 3000) {
				PG_INFO_STREAM("keyframes: " << keyframes << " name: " << name.toStdString())
					QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Too many keyframes! (size > 3000)", QMessageBox::Ok);
				return false;
			}

			for (unsigned int k = 0; k < keyframes; k++) {
				quint16 cutoutID, colortableID;
				qint32 start, durr;
				qint16 anchorX, anchorY, offsetX, offsetY, scaleX, scaleY, roation, mirror, unknown;
				in >> cutoutID >> colortableID >> start >> durr >> anchorX >> anchorY >> offsetX >> offsetY >> scaleX >> scaleY >> roation >> mirror >> unknown;
				if (cutoutID >= m_cutouts.size()) {
					PG_INFO_STREAM("cutoutID: " << cutoutID << " size: " << m_cutouts.size())
						QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Keyframes cutoutID is bigger then the number of cutouts!", QMessageBox::Ok);
					return false;
				}
				lay->push_backKeyframe(start, durr, cutoutID, colortableID, anchorX, anchorY, scaleX, scaleY, offsetX, offsetY, roation, mirror, unknown, false);
			}

		}
	}
	return true;
}

bool SpriteData::openPGSHv3(QDataStream& in) {
	quint16 number_uint16;
	quint32 number_uint32;

	QString text;
	text = readQString(in);//info text
	qDebug() << __LINE__ << ": " << text;
	if (text.isEmpty()) return false;
	text = readQString(in);
	if (text.isEmpty()) return false;

	{
		quint16 numberOfColorTables;
		in >> numberOfColorTables;
		for (unsigned int c = 0; c < numberOfColorTables; c++) {
			QColorTable colorTable;
			in >> number_uint16;
			for (unsigned int i = 0; i < number_uint16; i++) {
				quint8 r, g, b, a;
				in >> r >> g >> b >> a;
				colorTable.push_back(QColor(r, g, b, a));
			}
			m_colortables.push_back(colorTable);
		}
	}

	text = readQString(in);//SpriteSheets

	in >> number_uint16;
	for (unsigned int i = 0; i < number_uint16; i++) {
		qint16 externalID, powerColorTable;
		quint16 width, height ;
		quint32 savedSize;
		in >> externalID  >> powerColorTable >> width >> height >> savedSize;

		if (height > 6000 || width > 6000 || powerColorTable > 5000) {
			PG_INFO_STREAM("externalID: "<< externalID << " width: " << width << " height: " << height << " powerColorTable: " << powerColorTable)
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Sprite sheet bits are wrong? (width or height invalid)", QMessageBox::Ok);
			return false;
		}
		unsigned int size = width * height;
		if (size != savedSize) {
			PG_INFO_STREAM("size: " << size << " savedSize: " << savedSize)
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Sprite sheet size is different than expected! (width*height != stored size)", QMessageBox::Ok);
			return false;
		}

		m_spriteSheets.push_back(new SpriteSheet(externalID, width, height, powerColorTable, this));
		if (size != 0) in.readRawData((char*)(m_spriteSheets.last()->getSpriteSheet().begin()), size);

	}

	text = readQString(in);//Cutouts
	in >> number_uint32;
	for (unsigned int i = 0; i < number_uint32; i++) {
		quint16 externalSheetID, sheetRealID, x, y, w, h, dc;
		in >> externalSheetID >> sheetRealID >> x >> y >> w >> h >> dc;

		m_cutouts.push_back(new Cutout(sheetRealID, PG::UTIL::ivec2(x, y), PG::UTIL::ivec2(w, h), dc, this));
		assert_Test("Sheet ID out of bound!", sheetRealID >= m_spriteSheets.size());
		m_spriteSheets[sheetRealID]->push_backCutoutID(m_cutouts.size() - 1);
	}

	text = readQString(in);//Animations
	in >> number_uint32;
	qDebug() << __LINE__ << ": Ani Size: " << number_uint32;
	beginInsertRows(QModelIndex(), 0, number_uint32);
	for (unsigned int a = 0; a < number_uint32; a++) {
		quint32 id, markers, layers;
		QString name;
		in >> id >> name >> markers;
		if (markers > 1400) {
			PG_INFO_STREAM("markers: " << markers)
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Number of markers is too big! (size > 1400)", QMessageBox::Ok);
			return false;
		}


		SpriteAnimation* ani = new SpriteAnimation(id, name);
		m_aniamtions.push_back(ani);
		for (unsigned int m = 0; m < markers; m++) {
			quint32 start, duration, type;
			qint16 a;
			quint16 b;
			in >> start >> duration >> type >> a >> b;
			//qDebug()<<__LINE__<<"_Markers: start: "<<start<<" duration: "<<duration<<" type: "<<type<<" a: "<<a<<" b: "<<b;
			assert_Test("Marker duration should be 1!", duration != 1);
			ani->getMarkers()->push_back(new Marker(start, duration, type, a, b, ani));
			//ani->addMarker(start,type,a,b);
		}

		in >> layers;
		if (layers > 1400) {
			PG_INFO_STREAM("layers: " << layers)
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Number of layers is too big! (size > 1400)", QMessageBox::Ok);
			return false;
		}
		//PG_INFO_STREAM("layers: "<<layers)
		for (unsigned int l = 0; l < layers; l++) {
			QString name;// = readQString(in);
			quint8 isHidden;
			in >> name >> isHidden;
			Layer* lay = new Layer(name, isHidden, ani);
			ani->getLayers().push_back(lay);
			quint32 keyframes;
			in >> keyframes;
			//PG_INFO_STREAM("keyframes: "<<keyframes<<" name: "<<name.toStdString())

			if (keyframes >= 3000) {
				PG_INFO_STREAM("keyframes: " << keyframes << " name: " << name.toStdString())
					QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Too many keyframes! (size > 3000)", QMessageBox::Ok);
				return false;
			}

			for (unsigned int k = 0; k < keyframes; k++) {
				quint16 cutoutID, colortableID;
				qint32 start, durr;
				qint16 anchorX, anchorY, offsetX, offsetY, scaleX, scaleY, roation, mirror, unknown;
				in >> cutoutID >> colortableID >> start >> durr >> anchorX >> anchorY >> offsetX >> offsetY >> scaleX >> scaleY >> roation >> mirror >> unknown;
				if (cutoutID >= m_cutouts.size()) {
					PG_INFO_STREAM("cutoutID: " << cutoutID << " size: " << m_cutouts.size())
						QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Keyframes cutoutID is bigger then the number of cutouts!", QMessageBox::Ok);
					return false;
				}
				lay->push_backKeyframe(start, durr, cutoutID, colortableID, anchorX, anchorY, scaleX, scaleY, offsetX, offsetY, roation, mirror, unknown, false);
			}

		}
	}
	return true;
}


// functions
bool SpriteData::open(const QString& file) {
	if (file.size() == 0) return false;
	close();

	QFile qfile(file);
	if (!qfile.open(QIODevice::ReadOnly)) return false;
	QDataStream in(&qfile);

	{

		QByteArray buffer(4, Qt::Uninitialized);
		in.readRawData(buffer.data(), 4);

		QString magic(buffer);
		if (magic != "PGSH") {
			PG_INFO_STREAM("Magic number is wrong!");
			QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error",
				"Magic number is wrong! Should be 'PGSH'!",
				QMessageBox::Ok);
			qfile.close();
			return false;
		}

		bool success = false;
		quint16 version;
		in >> version;
		switch (version)
		{
			case 1:  success = openPGSHv1(in); break;
			case 2:  success = openPGSHv2(in); break;
			case 3:  success = openPGSHv3(in); break;
		}

		if(!success) {
			PG_INFO_STREAM("Only version 1, 2 or 3 are supported!");
			QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error", "Only file format version 1, 2 or 3 are supported!", QMessageBox::Ok);
			qfile.close();
			return false;
		}
	}

	qfile.close();
	m_lastFile = file;
	m_currentColorTable = 0;
	endInsertRows();

	if (m_aniamtions.empty()) {
		m_currentAnimation = -1;
		emit onNumberOfAnimationsChanged();
		emit onCurrentAnimationChanged();
		emit onAnimationChanged(nullptr);
	}
	else {
		m_currentAnimation = 0;
		emit onNumberOfAnimationsChanged();
		emit onCurrentAnimationChanged();
		emit onAnimationChanged(m_aniamtions[m_currentAnimation]);
	}



	emit onLastFileNameChanged();
	emit numberOfColorTablesChanged();
	emit onNumberOfCutoutsChanged();
	emit numberOfSheetsChanged();
	emit allSpriteSheetsChanged();
	emit allColorTablesChanged();
	emit currentColorTableChanged();

	return true;
}
