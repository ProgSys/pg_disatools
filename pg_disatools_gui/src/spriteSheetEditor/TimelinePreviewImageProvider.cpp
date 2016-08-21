/*
 * TimelinePreviewImageProvider.cpp
 *
 *  Created on: 17.08.2016
 *      Author: ProgSys
 */

#include <spriteSheetEditor/TimelinePreviewImageProvider.h>
#include <QImage>
#include <QPainter>
#include <QDebug>
TimelinePreviewImageProvider::TimelinePreviewImageProvider(SpriteData* data) :
//QQuickImageProvider(QQuickImageProvider::Pixmap)
QQuickImageProvider(QQmlImageProviderBase::Image),
m_data(data)
{}


QPixmap TimelinePreviewImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    int width = 100;
    int height = 50;

    if (size)
        *size = QSize(width, height);
    QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : width,
                   requestedSize.height() > 0 ? requestedSize.height() : height);
    pixmap.fill(QColor(id).rgba());

    // write the color name
    QPainter painter(&pixmap);
    QFont f = painter.font();
    f.setPixelSize(20);
    painter.setFont(f);
    painter.setPen(Qt::black);
    if (requestedSize.isValid())
        painter.scale(requestedSize.width() / width, requestedSize.height() / height);
    painter.drawText(QRectF(0, 0, width, height), Qt::AlignCenter, id);

    return pixmap;
}

QImage TimelinePreviewImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize){
	//return QImage("resources/test.jpg");

	QStringList idSplit = id.split("_");
	if(!m_data || idSplit.size() != 2)
		return QImage();

	if(idSplit[0] == "sheet"){
		const unsigned int spritesheetID = idSplit[1].toInt();
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
						const QColor& color = m_data->getColortable()[colorTableID*16 + id];

						qimg.bits()[address] = color.red();
						qimg.bits()[address+1] = color.green();
						qimg.bits()[address+2] = color.blue();
						qimg.bits()[address+3] = color.alpha();
					}
			}



		return qimg;

	}

	const unsigned int cutoutID = idSplit[0].toInt();
	const unsigned int colortableID = idSplit[1].toInt();
	//qInfo() <<"Requested preview image: '"<<cutoutID<<"' '"<<colortableID<<"'";

	return m_data->getSprite(cutoutID, colortableID);
}

TimelinePreviewImageProvider::~TimelinePreviewImageProvider() {
	// TODO Auto-generated destructor stub
}

