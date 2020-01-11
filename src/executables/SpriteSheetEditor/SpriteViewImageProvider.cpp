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
#include <SpriteViewImageProvider.h>
#include <Util/Misc/ResourcePath.h>
#include <QDebug>

SpriteViewImageProvider::SpriteViewImageProvider(SpriteData* data) :
	QQuickImageProvider(QQmlImageProviderBase::Image),
	m_data(data)
{}

void insertIntoImage(const SpriteData* sprite, QImage& qimg, const Cutout* cut, int spritesheetID, const PG::UTIL::IDImage& sheetID, bool doFill = false) {
	if (cut->getSheetID() != spritesheetID || cut->isHidden()) return;
	int x, y, startX, startY, trueEndX, trueEndY;

	if (doFill) {
		startX = 0;
		startY = 0;
		trueEndX = qimg.width();
		trueEndY = qimg.height();
	}
	else {
		startX = cut->getX();
		startY = cut->getY();
		trueEndX = (cut->getX() + cut->getWidth() > qimg.width()) ? qimg.width() : cut->getX() + cut->getWidth();
		trueEndY = (cut->getY() + cut->getHeight() > qimg.height()) ? qimg.height() : cut->getY() + cut->getHeight();
	}

	if (startX < 0 || startY < 0 || trueEndY <= 0 || trueEndX <= 0)
		return;

	for (y = startY; y < trueEndY; ++y) {
		for (x = startX; x < trueEndX; ++x) {
			const int address = (y * qimg.width() + x) * 4;
			assert_Test("Address out of bound!", address >= qimg.width() * qimg.height() * 4);
			const int id = sheetID.get(x, y);

			//ID out of bound!
			if (sprite->getColorTable().size() <= id) {
				qimg.bits()[address] = 255;
				qimg.bits()[address + 1] = 0;
				qimg.bits()[address + 2] = 255;
				qimg.bits()[address + 3] = 255;
				continue;
			}
			//assert_Test("ID out of bound!", m_data->getColorTable().size() <= id);

			unsigned int colorTableIndex = cut->getDefaultColorTable() * 16 + id;
			if (colorTableIndex >= sprite->getColorTable().size())
				colorTableIndex = id;

			assert_Test("Color table Index out of bound!", colorTableIndex >= m_data->getColorTable().size());

			const QColor& color = sprite->getColorTable()[colorTableIndex];

			qimg.bits()[address] = color.red();
			qimg.bits()[address + 1] = color.green();
			qimg.bits()[address + 2] = color.blue();
			qimg.bits()[address + 3] = color.alpha();
		}
	}
}

QImage SpriteViewImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize) {

	if (!m_data || id.isEmpty())
		return QImage();

	const int spritesheetID = id.toInt();

	if (spritesheetID >= 0 && spritesheetID < m_data->getNumberOfSpriteSheets()) {
		if (spritesheetID >= m_data->getNumberOfSpriteSheets()) return QImage(getResourcePath() + "/test.jpg");

		assert_Test("Sprite sheet ID out of bound!", spritesheetID >= m_data->getNumberOfSpriteSheets());
		const SpriteSheet* sheet = m_data->getSpriteSheet(spritesheetID);

		const PG::UTIL::IDImage& sheetID = sheet->getSpriteSheet();
		if (size)* size = QSize(sheetID.getWidth(), sheetID.getHeight());
		if (sheetID.empty()) 
			return QImage();

		assert_Test("Colortable size is invalid!", sheet->getSizeOfColorTable() <= 0);

		QImage qimg(sheetID.getWidth(), sheetID.getHeight(), QImage::Format_RGBA8888);

		unsigned int count = 0;
		for (unsigned char c : sheetID) {
			qimg.bits()[count] = c;
			qimg.bits()[count + 1] = c;
			qimg.bits()[count + 2] = c;
			qimg.bits()[count + 3] = 255;
			count += 4;
		}

		if (m_data->getNumberOfColortables() > 0) {
			if (m_data->getIsolateSelection() && m_data->getSelected()) {
				insertIntoImage(m_data, qimg, m_data->getSelected(), spritesheetID, sheetID, true);
			}
			else {
				for (const Cutout* cut : m_data->getCutouts())
					insertIntoImage(m_data, qimg, cut, spritesheetID, sheetID);
			}
		}


		return qimg;

	}

	return QImage();
}


SpriteViewImageProvider::~SpriteViewImageProvider() {
	// TODO Auto-generated destructor stub
}

