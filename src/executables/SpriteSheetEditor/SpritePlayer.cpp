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
#include <SpritePlayer.h>

SpritePlayer::SpritePlayer(QWidget* parent) : QObject(parent) {
	qRegisterMetaType< SpriteAnimation >("SpriteAnimation");
	m_aniData = new SpriteData(this);
	m_timeline = new Timeline(m_aniData, this);

	connect(m_aniData, SIGNAL(onAnimationChanged(SpriteAnimation*)), m_timeline, SLOT(setAnimation(SpriteAnimation*)));
}

void SpritePlayer::connectGLWidget(GLSpriteWidget* gl) {
	if (!gl) return;
	m_glView = gl;
	m_glView->setData(m_aniData);
	//if(m_glView) m_glView->open(m_aniData);

	connect(m_timeline, &Timeline::currentFrame, gl, &GLSpriteWidget::renderFrame);
	connect(m_timeline, &Timeline::render, gl, &GLSpriteWidget::updateFrame);
	connect(this, &SpritePlayer::render, gl, &GLSpriteWidget::updateFrame);


	connect(m_aniData, &SpriteData::allSpriteSheetsChanged, gl, &GLSpriteWidget::updateAllSpriteSheets);
	connect(m_aniData, &SpriteData::spriteSheetChanged, gl, &GLSpriteWidget::updateSpriteSheet);
	connect(m_aniData, &SpriteData::spriteSheetAdded, gl, &GLSpriteWidget::updateSpriteSheetAdded);
	connect(m_aniData, &SpriteData::spriteSheetRemoved, gl, &GLSpriteWidget::updateSpriteSheetRemove);

	connect(m_aniData, &SpriteData::refresh, gl, &GLSpriteWidget::updateFrame);

	connect(m_aniData, &SpriteData::currentColorTableChanged, gl, &GLSpriteWidget::updateFrame);
	connect(m_aniData, &SpriteData::allColorTablesChanged, gl, &GLSpriteWidget::updateAllColortables);
	connect(m_aniData, &SpriteData::colorTableChanged, gl, &GLSpriteWidget::updateColortable);
	connect(m_aniData, &SpriteData::colorTableAdded, gl, &GLSpriteWidget::addColortable);
	connect(m_aniData, &SpriteData::colorTableRemoved, gl, &GLSpriteWidget::removeColortable);
	connect(m_aniData, &SpriteData::externalSpriteSheetOpened, gl, &GLSpriteWidget::updateExternalColortable);
	connect(m_aniData, &SpriteData::selectedColorIdChanged, gl, &GLSpriteWidget::updateFrame);

}

bool SpritePlayer::open(const QString& file) {
	close();

	if (file.isEmpty())
		return false;

	if (!m_aniData->open(file)) {
		close();
		return false;
	}

	//if(m_glView) m_glView->open(m_aniData);
	emit render();

	return true;

}

bool SpritePlayer::importSH(const QString& file) {
	close();

	if (file.isEmpty())
		return false;

	if (!m_aniData->importSH(file)) {
		close();
		return false;
	}

	//if(m_glView) m_glView->open(m_aniData);
	emit render();

	return true;
}

void SpritePlayer::close() {
	m_currentAnimation = 0;
	m_timeline->clear();
	m_aniData->close();
}

bool SpritePlayer::isOpen() const {
	return m_aniData->isOpen();
}

void SpritePlayer::setAnimation(int index) {
	if (m_aniData->getCurrentAnimationIndex() == index) return;
	m_aniData->setCurrentAnimationByIndex(index);
	emit render();
	m_timeline->setTracker(0);
	if (m_timeline->isPlaying())
		m_timeline->play();
	else
		m_timeline->pause();
}

SpritePlayer::~SpritePlayer() {
	if (m_timeline)
		delete m_timeline;
	if (m_aniData)
		delete m_aniData;
}

