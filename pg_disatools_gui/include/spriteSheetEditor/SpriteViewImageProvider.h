/*
 * SpriteViewImageProvider.h
 *
 *  Created on: 21.08.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_SPRITESHEETEDITOR_SPRITEVIEWIMAGEPROVIDER_H_
#define INCLUDE_SPRITESHEETEDITOR_SPRITEVIEWIMAGEPROVIDER_H_

#include <QImage>
#include <QQuickImageProvider>
#include <files/SpriteData.h>

class SpriteViewImageProvider : public QQuickImageProvider{
public:
	SpriteViewImageProvider(SpriteData* data = nullptr);

	QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

	virtual ~SpriteViewImageProvider();
private:
	SpriteData* m_data;
};

#endif /* INCLUDE_SPRITESHEETEDITOR_SPRITEVIEWIMAGEPROVIDER_H_ */
