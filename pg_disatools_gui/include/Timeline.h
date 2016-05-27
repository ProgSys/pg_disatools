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

#ifndef TIMELINE_H
#define TIMELINE_H

#include <QList>
#include <QObject>
#include <QTimer>

#define ONEFRAME_ANIMATION_SPEED 3

class Keyframe : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int duration READ getDuration WRITE setDuration NOTIFY durationChanged)
public:
    explicit Keyframe(QObject *parent = 0);
    explicit Keyframe(int duration, QObject *parent = 0);
    Keyframe(const Keyframe& keyframe);

    void operator =(const Keyframe& keyframe);

    //getters
    int getDuration() const;

    //setters
    void setDuration(int duration);

    ~Keyframe();

signals:
    void durationChanged();



private:
    int m_duration = 1;
};


class Timeline : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int steps READ getSteps NOTIFY stepsChanged)
    Q_PROPERTY(float timeScale READ getTimeScale NOTIFY timeScaleChanged)
    Q_PROPERTY(QList<QObject*> keyframes READ getKeyframes NOTIFY keyframesChanged)
    Q_PROPERTY(int width READ getWidth NOTIFY widthChanged)
    Q_PROPERTY(int size READ getSize NOTIFY sizeChanged)
    Q_PROPERTY(int tracker READ getTracker WRITE setTracker NOTIFY trackerChanged)

public:
    explicit Timeline(QObject *parent = 0);

    int getSteps() const;
    float getTimeScale() const;
    int getWidth() const;
    int getSize() const;
    Q_INVOKABLE int getOffset(int index) const;

    const QList<QObject*>& getKeyframes() const;

    /*!
     * @brief Get the track witch is under the tracker.
     * @returns -1 when there is no track under the tracker, like when there are no trackes.
     */
    int getTrackIndex() const;

    ~Timeline();
public slots:
	void clear();
	void addKeyframe(Keyframe* key);
	void addKeyframe(int duration);

	//player
	int getTracker() const;
	void setTracker(int tracker);

	void nextFrame();
	void previousFrame();
	void nextKeyframe();
	void previousKeyframe();
	void pause();
	void play();
	bool isPlaying() const;

private slots:
	void loop();

signals:
    void widthChanged();
    void keyframesChanged();
    void sizeChanged();
    void stepsChanged();
    void timeScaleChanged();
    void trackerChanged();

    void render();
    void renderKeyframe(int keyframe);

    void onPause();
    void onPlay();

private:
    void checkRender();
    void checkRender(int keyframe);
    //time
	QTimer m_time;
	bool m_playing = true;

	//scale
    int m_steps = 10;
    float m_timeScale = 1.0f;

    //tracker
    int m_tracker = 0;
    int m_totalTrackSize = 0;
    int m_currentKeyframe = 0;

    QList<QObject*> m_keyframes;
};

#endif // TIMELINE_H
