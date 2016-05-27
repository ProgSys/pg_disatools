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
	connect(this, SIGNAL( onCurrentAnimationChanged(int) ),gl, SLOT( setAnimation(int) ));
	connect(m_timeline, SIGNAL( renderKeyframe(int) ),gl, SLOT( renderKeyframe(int) ));
	connect(m_timeline, SIGNAL( render() ),gl, SLOT( renderKeyframe() ));
	connect(this, SIGNAL( render() ),gl, SLOT( renderKeyframe() ));
}

Timeline* SpritePlayer::getTimeline() const{
	return m_timeline;
}

bool SpritePlayer::openSH(const QString& file){
	close();

	if(file.isEmpty())
		return false;

	qDebug()<<" Player opening: "<<file;
	if(m_spriteSheet->importSH(file.toStdString())){
		close();
		return false;
	}
	qDebug()<<" Player opening success: "<<file;

	if(m_glView)
		m_glView->open(m_spriteSheet);
	emit render();

	for(const PG::FILE::keyframe& key: m_spriteSheet->getAnimation(m_currentAnimation).keyframes)
		m_timeline->addKeyframe(key.durration);

	//add animation to dropDown
	unsigned int i = 0;
	for(const PG::FILE::animation& ani: m_spriteSheet->getAnimations()){
		const QString str(QString::number(i)+ ": Size: "+QString::number(ani.keyframes.size())+", ID: "+QString::number(ani.ID)+", name: "+QString::fromStdString(ani.name));
		emit animationAdded(str);
		i++;
	}

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
	//qDebug()<<" Selected Animation: "<<index<<" last: "<<m_currentAnimation;
	if(m_currentAnimation == index) return;
	m_currentAnimation = index;
	if(m_currentAnimation < 0)
		m_currentAnimation = 0;
	if(m_currentAnimation >= m_spriteSheet->getNumberOfAnimations())
		m_currentAnimation = m_spriteSheet->getNumberOfAnimations()-1;

	m_timeline->clear();
	for(const PG::FILE::keyframe& key: m_spriteSheet->getAnimation(m_currentAnimation).keyframes)
		m_timeline->addKeyframe(key.durration);

	emit onCurrentAnimationChanged(m_currentAnimation);
	emit render();
	m_timeline->setTracker(0);
	if(m_timeline->isPlaying())
		m_timeline->play();
	else
		m_timeline->pause();
}

SpritePlayer::~SpritePlayer() {
	if(m_spriteSheet)
		delete m_spriteSheet;
	if(m_timeline)
		delete m_timeline;
}

