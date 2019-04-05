/*
 * ImageLib.h
 *
 *  Created on: 17.12.2017
 *      Author: ProgSys
 */

#ifndef INCLUDE_SPRITESHEETEDITOR_IMAGELIB_H_
#define INCLUDE_SPRITESHEETEDITOR_IMAGELIB_H_

#include <QImage>
#include <QVector>
#include <QColor>



namespace ImageLib {

void reduceColors(QImage& image, unsigned int targetColors);
void reassignColors(QImage& image, const QVector<QColor>& targetColors);

} /* namespace ImageLib */

#endif /* INCLUDE_SPRITESHEETEDITOR_IMAGELIB_H_ */
