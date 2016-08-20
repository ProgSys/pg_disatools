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

#include <spriteSheetEditor/Timeline.h>
#include <QDebug>



///// TIMELINE /////

Timeline::Timeline(SpriteData* aniData, QObject *parent)
    : QObject(parent)
{
	connect(&m_time, SIGNAL(timeout()), this, SLOT(loop()));
	if(aniData){
		connect(aniData, SIGNAL(onAnimationChanged(SpriteAnimation* )), this, SLOT(setAnimation(SpriteAnimation*)));
		connect(aniData, SIGNAL(onRefresh()), this, SLOT(updateTimeline()));
	}
    //m_keyframes.push_back(new Keyframe(10, this));
    //m_keyframes.push_back(new Keyframe(410, this));
    //m_keyframes.push_back(new Keyframe(15, this));
    //m_keyframes.push_back(new Keyframe(47, this));
}

int Timeline::getSteps() const
{
    return m_steps;
}

float Timeline::getTimeScale() const
{
    return m_timeScale;
}

int Timeline::getWidth() const
{
	return m_totalTrackSize;
}

QObject* Timeline::getAnimation() const{
	return m_currAnimation;
}

int Timeline::getSize() const
{
	if(!m_currAnimation) return 0;
    return m_currAnimation->getTotalFrames();
}

int Timeline::getTracker() const
{
    return m_tracker;
}


void Timeline::setTracker(int tracker)
{
    if(tracker < 0) tracker = 0;
    //qDebug()<<"tracker: "<<QString::number(tracker);
    if(m_tracker != tracker){
        m_tracker = tracker;
        emit currentFrame(m_tracker);
        emit trackerChanged();
        pause();
    }
}

void Timeline::setTimeScale(float scale){
	if(m_timeScale == scale) return;
	m_timeScale = scale;
	emit timeScaleChanged();
}

void Timeline::setSteps(int steps){
	if(m_steps == steps) return;
	m_steps = steps;
	emit stepsChanged();
}

void Timeline::clear(){
	m_tracker = 0;
	m_totalTrackSize = 0;
	m_time.stop();

	emit currentFrame(0);
	emit totalFrames(0);
	emit trackerChanged();
	emit onAnimationChanged();
	emit widthChanged();
}

void Timeline::setAnimation(SpriteAnimation* ani){
	m_time.stop();
	if(!ani){
		m_currAnimation = nullptr;
		clear();
		return;
	}

	if(m_currAnimation == ani){
		m_tracker = 0;
		emit currentFrame(0);
		emit trackerChanged();
		return;
	}

	m_currAnimation = ani;
	qDebug()<<"Animation set to: "<<m_currAnimation->getName() <<" with "<<ani->getNumberOfLayers()<<" layers.";
	m_tracker = 0;
	m_totalTrackSize = m_currAnimation->getTotalFrames();

	emit totalFrames(m_totalTrackSize);
	emit currentFrame(0);
	emit trackerChanged();
	emit onAnimationChanged();
	emit widthChanged();

	if(isPlaying())
		m_time.start(ONEFRAME_ANIMATION_SPEED);
}

void Timeline::updateTimeline(){
	emit render();
	if(m_currAnimation && m_totalTrackSize != m_currAnimation->getTotalFrames()){
		m_totalTrackSize = m_currAnimation->getTotalFrames();
		emit totalFrames(m_totalTrackSize);
		emit widthChanged();
		m_currAnimation->refresh();
	}
}

void Timeline::loop(){
	nextFrame();
	m_time.start(ONEFRAME_ANIMATION_SPEED);
}

void Timeline::nextFrame(){
	if(m_totalTrackSize <= 0) return;
	m_tracker++;
	if(m_tracker > m_totalTrackSize)
		m_tracker = 0;

	emit currentFrame(m_tracker);
	emit trackerChanged();
	//emit render();

}
void Timeline::previousFrame(){
	if(m_totalTrackSize <= 0) return;
	m_tracker--;
	if(m_tracker < 0)
		m_tracker = m_totalTrackSize;


	emit currentFrame(m_tracker);
	emit trackerChanged();
	//emit render();
}
void Timeline::nextKeyframe(){
	if(!m_currAnimation) return;

	int lastFrame = m_totalTrackSize;
	for(const Layer* lay: m_currAnimation->getLayers()){
		for(const Keyframe* key: lay->getKeyframes()){
			if(m_tracker >= key->getStart() && m_tracker < key->getEnd() &&  lastFrame > key->getEnd())
				lastFrame = key->getEnd();
		}
	}
	if(lastFrame == m_totalTrackSize)
		m_tracker = 0;
	else
		m_tracker = lastFrame;

	emit currentFrame(m_tracker);
	emit trackerChanged();
	//emit render();
}
void Timeline::previousKeyframe(){
	if(!m_currAnimation) return;

	int lastFrame = 0;
	for(const Layer* lay: m_currAnimation->getLayers()){
		for(const Keyframe* key: lay->getKeyframes()){
			if(m_tracker > key->getStart() && m_tracker <= key->getEnd() &&  lastFrame < key->getStart())
				lastFrame = key->getStart();
		}
	}
	if(lastFrame == 0)
		m_tracker = m_totalTrackSize;
	else
		m_tracker = lastFrame;

	emit currentFrame(m_tracker);
	emit trackerChanged();
	//emit render();
}
void Timeline::pause(){
	m_playing = false;
	if(m_totalTrackSize <= 0) return;
	m_time.stop();
	emit onPause();
}
void Timeline::play(){
	m_playing = true;
	if(m_totalTrackSize <= 0) return;
	m_time.start(ONEFRAME_ANIMATION_SPEED);
	emit onPlay();
}

bool Timeline::isPlaying() const{
	return m_playing;
}


Timeline::~Timeline()
{
}


