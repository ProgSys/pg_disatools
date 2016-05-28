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
#ifndef SPRITEPLAYER_H_
#define SPRITEPLAYER_H_

#include <QObject>
#include <spriteSheetEditor/Timeline.h>
#include <pg/files/PG_SpriteAnimation.h>
#include <GLWidget.h>

class SpritePlayer: public QObject {
	Q_OBJECT
public:
	SpritePlayer(QWidget *parent = 0);
	void connectGLWidget(GLWidget *gl);

	Timeline* getTimeline() const;

	virtual ~SpritePlayer();
public slots:
	/*!
	 * @returns true on success
	 */
	bool openSH(const QString& file);
	void close();
	bool isOpen() const;
	void setAnimation(int index);

signals:
	void onCurrentAnimationChanged(int setAnimation);
	void animationAdded(const QString& name);
	void render();
private:
	PG::FILE::SpriteAnimation* m_spriteSheet = nullptr;
	Timeline* m_timeline = nullptr;
	GLWidget* m_glView = nullptr;
	int m_currentAnimation = 0;
};

#endif /* SPRITEPLAYER_H_ */
