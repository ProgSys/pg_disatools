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
#include <tx2Editor/TX2ImageProvider.h>

TX2ImageProvider::TX2ImageProvider(PG::FILE::tx2Image* image):
	QQuickImageProvider(QQmlImageProviderBase::Image),
	m_image(image){
	// TODO Auto-generated constructor stub

}

QImage TX2ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize){
	if(!m_image || m_image->getWidth() == 0 || m_image->getHeight() == 0) return QImage("resources/disgaea-d2-artwork-3.jpg");

	QImage img(m_image->getWidth(), m_image->getHeight(), QImage::Format_RGBA8888);
	m_image->getRGBAData((char*)img.bits(), 0);

	return img;
}

void TX2ImageProvider::setModel(PG::FILE::tx2Image* image){
	m_image = image;
}

TX2ImageProvider::~TX2ImageProvider() {
	// TODO Auto-generated destructor stub
}

