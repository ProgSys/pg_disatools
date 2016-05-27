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

#include <Timeline.h>
#include <QDebug>

///// KEYFRAME /////
Keyframe::Keyframe(QObject *parent)
: QObject(parent)
{

}

Keyframe::Keyframe(int duration, QObject *parent)
    : QObject(parent)
{
    if(duration <= 0)
        m_duration = 1;
    else
        m_duration = duration;
}

Keyframe::Keyframe(const Keyframe &keyframe)
 : QObject(nullptr)
{
    m_duration = keyframe.getDuration();
}

void Keyframe::operator =(const Keyframe &keyframe)
{
     m_duration = keyframe.getDuration();
}

int Keyframe::getDuration() const
{
    return m_duration;
}

void Keyframe::setDuration(int duration)
{
    if(duration <= 0)
        duration = 1;
    if(duration != m_duration){
         m_duration = duration;
         emit durationChanged();
    }
}

Keyframe::~Keyframe()
{

}

///// TIMELINE /////

Timeline::Timeline(QObject *parent)
    : QObject(parent)
{
	connect(&m_time, SIGNAL(timeout()), this, SLOT(loop()));
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
    return m_keyframes.size();
}

int Timeline::getTracker() const
{
    return m_tracker;
}

int Timeline::getOffset(int index) const
{
    int width = 0;
    for(int i = 0; i < index; ++i)
         width += static_cast<const Keyframe* const>(m_keyframes[i])->getDuration();
    return width;
}

const QList<QObject*> &Timeline::getKeyframes() const
{
    return m_keyframes;
}

int Timeline::getTrackIndex() const{
	if(m_keyframes.empty()) return -1;

	int width = 0;
	for(int i = 0; i < m_keyframes.size(); ++i){
		width += static_cast<const Keyframe* const>(m_keyframes[i])->getDuration();
		if(m_tracker < width)
			return i;
	}

	return m_keyframes.size()-1;
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
	m_keyframes.clear();
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

void Timeline::addKeyframe(Keyframe* key){
	if(!key) return;
	m_keyframes.push_back(key);
	m_totalTrackSize += key->getDuration();

	emit totalFrames(m_totalTrackSize);
	emit totalKeyframes(m_keyframes.size());
	emit keyframesChanged();
	emit widthChanged();
}

void Timeline::addKeyframe(int duration){
	if(duration <= 0){
		qDebug()<<"Duration must be bigger then 0!";
		return;
	}
	m_keyframes.push_back(new Keyframe(duration, this));
	m_totalTrackSize += duration;

	emit totalFrames(m_totalTrackSize);
	emit totalKeyframes(m_keyframes.size());
	emit keyframesChanged();
	emit widthChanged();
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
	if(i >= m_keyframes.size())
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
		if(m_keyframes.empty()){
			m_tracker = 0;
			checkRender(0);
		}else{
			i = m_keyframes.size()-1;
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
    for(QObject* obj: m_keyframes)
        delete obj;
}


