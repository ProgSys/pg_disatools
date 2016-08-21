/*
 * SpriteViewImageProvider.cpp
 *
 *  Created on: 21.08.2016
 *      Author: ProgSys
 */

#include <spriteSheetEditor/SpriteViewImageProvider.h>

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
		const PG::UTIL::IDImage& sheetID = m_data->getSpriteSheet(spritesheetID)->getSpriteSheet();
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
				for(int y = cut->getY(); y < cut->getY()+cut->getHeight(); y++)
					for(int x = cut->getX(); x < cut->getX()+cut->getWidth(); x++){
						const int address = (y*qimg.width()+x)*4;
						assert_Test("Address out of bound!", address >= qimg.width()*qimg.height()*4);
						const int id = sheetID.get(x,y);
						assert_Test("ID out of bound!", m_data->getColortable().size() <= id);
						const unsigned int colorTableID = (cut->getDefaultColorTable() >= m_data->getNumberOfColortables())? 0 :  cut->getDefaultColorTable();
						assert_Test("Color table ID out of bound!", colorTableID*16 + id >= m_data->getColortable().size());
						const QColor& color = m_data->getColortable()[colorTableID*16 + id];

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

