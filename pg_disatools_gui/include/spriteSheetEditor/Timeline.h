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
#include <files/SpriteData.h>

#define ONEFRAME_ANIMATION_SPEED 3


class Timeline : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int steps READ getSteps NOTIFY stepsChanged)
    Q_PROPERTY(float timeScale READ getTimeScale NOTIFY timeScaleChanged)
    //Q_PROPERTY(QList<QObject*> keyframes READ getKeyframes NOTIFY keyframesChanged)
    Q_PROPERTY(int width READ getWidth NOTIFY widthChanged)
    Q_PROPERTY(int size READ getSize NOTIFY sizeChanged)
    Q_PROPERTY(int tracker READ getTracker WRITE setTracker NOTIFY trackerChanged)

public:
    explicit Timeline(SpriteData* aniData = nullptr, QObject *parent = 0);

    int getSteps() const;
    float getTimeScale() const;
    int getSize() const;
    Q_INVOKABLE int getOffset(int index) const;


    /*!
     * @brief Get the track witch is under the tracker.
     * @returns -1 when there is no track under the tracker, like when there are no trackes.
     */
    int getTrackIndex() const;



    ~Timeline();
public slots:
	void clear();

	//player
	int getTracker() const;
	void setTracker(int tracker);
	int getWidth() const;

	//setters
	void setAnimation(SpriteAnimation* ani);

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

    void currentKeyframe(int keyframe);
    void totalKeyframes(int totalkeyframes);

    void currentFrame(int frame);
    void totalFrames(int totalframes);

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

    SpriteAnimation* m_currAnimation = nullptr;
};

#endif // TIMELINE_H
