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

	const unsigned int cutoutID = idSplit[0].toInt();
	const unsigned int colortableID = idSplit[1].toInt();
	//qInfo() <<"Requested preview image: '"<<cutoutID<<"' '"<<colortableID<<"'";
	return m_data->getSprite(cutoutID, colortableID);
}

TimelinePreviewImageProvider::~TimelinePreviewImageProvider() {
	// TODO Auto-generated destructor stub
}

