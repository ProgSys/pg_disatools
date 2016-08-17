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
	QStringList idSplit = id.split("_");
	if(!m_data || idSplit.size() != 2)
		return QImage();

	const unsigned int cutoutID = idSplit[0].toInt();
	const unsigned int colortableID = idSplit[1].toInt();
	//qInfo() <<"Requested preview image: '"<<cutoutID<<"' '"<<colortableID<<"'";

	return m_data->getSprite(cutoutID, colortableID);
}

TimelinePreviewImageProvider::~TimelinePreviewImageProvider() {
	// TODO Auto-generated destructor stub
}

