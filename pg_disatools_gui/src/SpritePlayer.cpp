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

SpritePlayer::SpritePlayer(QWidget *parent): QObject(parent) {
	m_spriteSheet = new PG::FILE::SpriteAnimation();
	m_timeline = new Timeline();
}

void SpritePlayer::connectGLWidget(GLWidget *gl){
	m_glView = gl;
	connect(this, SIGNAL( onCurrentAnimationChanged(int index) ),gl, SLOT( setAnimation(int index) ));
	connect(m_timeline, SIGNAL( renderKeyframe(int keyframe) ),gl, SLOT( renderKeyframe(int keyframe) ));

}

Timeline* SpritePlayer::getTimeline() const{
	return m_timeline;
}

bool SpritePlayer::openSH(const QString& file){
	close();

	if(file.isEmpty())
		return false;

	if(!m_spriteSheet->importSH(file.toStdString())){
		close();
		return false;
	}

	if(m_glView)
		m_glView->open(m_spriteSheet);


	for(const PG::FILE::keyframe& key: m_spriteSheet->getAnimation(m_currentAnimation).keyframes)
		m_timeline->addKeyframe(key.durration);

	return true;

}

void SpritePlayer::close(){
	m_currentAnimation = 0;
	m_timeline->clear();
	m_spriteSheet->clear();
}

bool SpritePlayer::isOpen() const{
	return m_spriteSheet->isOpen();
}

void SpritePlayer::setAnimation(int index){
	if(m_currentAnimation == index) return;
	m_currentAnimation = index;
	if(m_currentAnimation < 0)
		m_currentAnimation = 0;
	if(m_currentAnimation >= m_spriteSheet->getNumberOfAnimations())
		m_currentAnimation = m_spriteSheet->getNumberOfAnimations()-1;
	onCurrentAnimationChanged(m_currentAnimation);
}

SpritePlayer::~SpritePlayer() {
	if(m_spriteSheet)
		delete m_spriteSheet;
	if(m_timeline)
		delete m_timeline;
}

