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
    Q_PROPERTY(int steps READ getSteps WRITE setSteps NOTIFY stepsChanged)
    Q_PROPERTY(float timeScale READ getTimeScale WRITE setTimeScale NOTIFY timeScaleChanged)
	Q_PROPERTY(QObject* animation READ getAnimation NOTIFY onAnimationChanged)
    Q_PROPERTY(int width READ getWidth NOTIFY widthChanged)
    Q_PROPERTY(int size READ getSize NOTIFY sizeChanged)
    Q_PROPERTY(int tracker READ getTracker WRITE setTracker NOTIFY trackerChanged)

public:
    explicit Timeline(SpriteData* aniData = nullptr, QObject *parent = 0);

    int getSteps() const;
    float getTimeScale() const;
    int getSize() const;



    ~Timeline();
public slots:
	void clear();

	//player
	void nextFrame();
	void previousFrame();
	void nextKeyframe();
	void previousKeyframe();
	void pause();
	void play();

	//getters
	bool isPlaying() const;
	int getTracker() const;
	int getWidth() const;
	QObject* getAnimation() const;

	//setters
	void setTracker(int tracker);
	void setTimeScale(float scale);
	void setSteps(int steps);
	void setAnimation(SpriteAnimation* ani);



private slots:
	void loop();

signals:

    void widthChanged();
    void sizeChanged();
    void stepsChanged();
    void timeScaleChanged();
    void trackerChanged();
    void onAnimationChanged();

    void render();

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

    SpriteAnimation* m_currAnimation = nullptr;
};

#endif // TIMELINE_H
