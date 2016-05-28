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
	if(aniData)
		connect(aniData, SIGNAL(onAnimationChanged(SpriteAnimation* )), this, SLOT(setAnimation(SpriteAnimation*)));
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

int Timeline::getSize() const
{
	if(!m_currAnimation) return 0;
    return m_currAnimation->getKeyframes().size();
}

int Timeline::getTracker() const
{
    return m_tracker;
}

int Timeline::getOffset(int index) const
{
	if(!m_currAnimation) return 0;
    int width = 0;

    for(int i = 0; i < index; ++i)
    	width += m_currAnimation->getKeyframes()[i].getDuration();
         //width += static_cast<const Keyframe* const>(m_keyframes[i])->getDuration();
    return width;
}

int Timeline::getTrackIndex() const{
	if(!m_currAnimation || m_currAnimation->getKeyframes().empty()) return -1;

	int width = 0;
	for(int i = 0; i < m_currAnimation->getKeyframes().size(); ++i){
		width += m_currAnimation->getKeyframes()[i].getDuration();
		if(m_tracker < width)
			return i;
	}

	return m_currAnimation->getKeyframes().size()-1;
}

void Timeline::setTracker(int tracker)
{
    if(tracker < 0) tracker = 0;
    //qDebug()<<"tracker: "<<QString::number(tracker);
    if(m_tracker != tracker){
        m_tracker = tracker;
        checkRender();
        emit currentFrame(m_tracker);
        emit trackerChanged();
        pause();
    }
}

void Timeline::clear(){
	m_tracker = 0;
	m_totalTrackSize = 0;
	m_time.stop();
	m_currentKeyframe = 0;

	emit currentFrame(0);
	emit currentKeyframe(0);
	emit totalFrames(0);
	emit totalKeyframes(0);
	emit trackerChanged();
	emit keyframesChanged();
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
		m_currentKeyframe = 0;
		emit currentFrame(0);
		emit currentKeyframe(0);
		emit trackerChanged();
		return;
	}

	m_currAnimation = ani;
	m_tracker = 0;
	m_currentKeyframe = 0;
	m_totalTrackSize = m_currAnimation->getTotalFrames();

	emit totalFrames(m_totalTrackSize);
	emit totalKeyframes(m_currAnimation->getNumberOfKeyframes());
	emit currentFrame(0);
	emit currentKeyframe(0);
	emit trackerChanged();
	emit keyframesChanged();

	if(isPlaying())
		m_time.start(ONEFRAME_ANIMATION_SPEED);
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

	int keyFrame = getTrackIndex();
	checkRender();

	emit currentFrame(m_tracker);
	emit trackerChanged();

}
void Timeline::previousFrame(){
	if(m_totalTrackSize <= 0) return;
	m_tracker--;
	if(m_tracker < 0)
		m_tracker = m_totalTrackSize;
	int keyFrame = getTrackIndex();
	checkRender();

	emit currentFrame(m_tracker);
	emit trackerChanged();
}
void Timeline::nextKeyframe(){
	if(m_totalTrackSize <= 0) return;
	int i = getTrackIndex();
	if(i < 0){
		m_tracker = 0;
		checkRender(0);
		emit trackerChanged();
		return;
	}
	i++;
	if(i >= m_currAnimation->getKeyframes().size())
		i = 0;
	m_tracker = getOffset(i);
	checkRender(i);
	emit trackerChanged();
}
void Timeline::previousKeyframe(){
	if(m_totalTrackSize <= 0) return;
	int i = getTrackIndex();
	i--;
	if(i < 0){
		if(m_currAnimation->getKeyframes().empty()){
			m_tracker = 0;
			checkRender(0);
		}else{
			i = m_currAnimation->getKeyframes().size()-1;
			m_tracker = getOffset(i);
			checkRender(i);
		}
		emit trackerChanged();
		return;
	}

	m_tracker = getOffset(i);
	checkRender(i);
	emit trackerChanged();
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

void Timeline::checkRender(){
	int keyFrame = getTrackIndex();
	if(keyFrame != m_currentKeyframe){
		m_currentKeyframe = keyFrame;
		emit render();
		emit currentKeyframe(m_currentKeyframe);
	}
}
void Timeline::checkRender(int keyframe){
	if(keyframe != m_currentKeyframe){
		m_currentKeyframe = keyframe;
		emit render();
		emit currentKeyframe(m_currentKeyframe);
	}
}

Timeline::~Timeline()
{
}


