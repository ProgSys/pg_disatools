/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2016  ProgSys
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
#include <spriteSheetEditor/SpriteViewImageProvider.h>
#include <QDebug>

SpriteViewImageProvider::SpriteViewImageProvider(SpriteData* data):
	QQuickImageProvider(QQmlImageProviderBase::Image),
	m_data(data)
	{}

QImage SpriteViewImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize){
	//return QImage("resources/test.jpg");

	if(!m_data || id.isEmpty())
		return QImage();

	const int spritesheetID = id.toInt();

	if(spritesheetID >= 0 && spritesheetID < m_data->getNumberOfSpriteSheets() ){
		if(spritesheetID >= m_data->getNumberOfSpriteSheets() ) return QImage("resources/test.jpg");



		assert_Test("Sprite sheet ID out of bound!", spritesheetID >= m_data->getNumberOfSpriteSheets());
		const SpriteSheet* sheet = m_data->getSpriteSheet(spritesheetID);

		assert_Test("Colortable size is invalid!", sheet->getSizeOfColorTable() <= 0);

		const PG::UTIL::IDImage& sheetID = sheet->getSpriteSheet();
		QImage qimg( sheetID.getWidth() , sheetID.getHeight(), QImage::Format_RGBA8888 );

		unsigned int count = 0;
		for(unsigned char c: sheetID){
			qimg.bits()[count] = c;
			qimg.bits()[count+1] = c;
			qimg.bits()[count+2] = c;
			qimg.bits()[count+3] = 255;
			count+=4;
		}


		if(m_data->getNumberOfColortables())
			for(const Cutout* cut: m_data->getCutouts()){
				if(cut->isExternalSheet() || cut->getSheetID() != spritesheetID || cut->isHidden()) continue;
				const int trueEndX = (cut->getX()+cut->getWidth() > qimg.width())? qimg.width() : cut->getX()+cut->getWidth();
				const int trueEndY = (cut->getY()+cut->getHeight() > qimg.height())? qimg.height() : cut->getY()+cut->getHeight();
				if(trueEndY <= 0 || trueEndX <= 0) continue;

				for(int y = cut->getY(); y < trueEndY; y++)
					for(int x = cut->getX(); x < trueEndX; x++){
						const int address = (y*qimg.width()+x)*4;
						assert_Test("Address out of bound!", address >= qimg.width()*qimg.height()*4);
						const int id = sheetID.get(x,y);
						assert_Test("ID out of bound!", m_data->getColortable().size() <= id);

						const unsigned int colorTableIndex = cut->getDefaultColorTable()*16 + id;

						assert_Test("Color table Index out of bound!", colorTableIndex >= m_data->getColortable().size());

						const QColor& color = m_data->getColortable()[colorTableIndex];

						qimg.bits()[address] = color.red();
						qimg.bits()[address+1] = color.green();
						qimg.bits()[address+2] = color.blue();
						qimg.bits()[address+3] = color.alpha();
					}
			}



		return qimg;

	}

	return QImage();
}


SpriteViewImageProvider::~SpriteViewImageProvider() {
	// TODO Auto-generated destructor stub
}

