#include "Timeline.h"
#include <QDebug>

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


Timeline::Timeline(QObject *parent)
    : QObject(parent)
{

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
    int width = 0;
    for(const QObject* const obj: m_keyframes)
        width += static_cast<const Keyframe* const>(obj)->getDuration();
    return width;
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

void Timeline::setTracker(int tracker)
{
    if(tracker < 0) tracker = 0;
    qDebug()<<"tracker: "<<QString::number(tracker);
    if(m_tracker != tracker){
        m_tracker = tracker;
        emit trackerChanged();
    }
}

void Timeline::clear(){
	m_keyframes.clear();
	m_tracker = 0;
	emit trackerChanged();
	emit keyframesChanged();
}

void Timeline::addKeyframe(Keyframe* key){
	m_keyframes.push_back(key);
	emit keyframesChanged();
}

void Timeline::addKeyframe(int duration){
	m_keyframes.push_back(new Keyframe(duration, this));
	emit keyframesChanged();
}

Timeline::~Timeline()
{
    for(QObject* obj: m_keyframes)
        delete obj;
}


