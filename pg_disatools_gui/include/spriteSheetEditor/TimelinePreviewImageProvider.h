/*
 * TimelinePreviewImageProvider.h
 *
 *  Created on: 17.08.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_SPRITESHEETEDITOR_TIMELINEPREVIEWIMAGEPROVIDER_H_
#define INCLUDE_SPRITESHEETEDITOR_TIMELINEPREVIEWIMAGEPROVIDER_H_

#include <QImage>
#include <QQuickImageProvider>
#include <files/SpriteData.h>


class TimelinePreviewImageProvider : public QQuickImageProvider{
public:
	TimelinePreviewImageProvider(SpriteData* data = nullptr);

	QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
	QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

	virtual ~TimelinePreviewImageProvider();
private:
	SpriteData* m_data;
};

#endif /* INCLUDE_SPRITESHEETEDITOR_TIMELINEPREVIEWIMAGEPROVIDER_H_ */
