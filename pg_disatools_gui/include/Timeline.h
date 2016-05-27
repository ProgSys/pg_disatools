#ifndef TIMELINE_H
#define TIMELINE_H

#include <QList>
#include <QObject>

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
    int getTracker() const;
    Q_INVOKABLE int getOffset(int index) const;

    const QList<QObject*>& getKeyframes() const;

    void setTracker(int tracker);



    ~Timeline();
public slots:
	void clear();
	void addKeyframe(Keyframe* key);
	void addKeyframe(int duration);

signals:
    void widthChanged();
    void keyframesChanged();
    void sizeChanged();
    void stepsChanged();
    void timeScaleChanged();
    void trackerChanged();

private:
    int m_steps = 10;
    float m_timeScale = 1.0f;

    int m_tracker = 0;

    QList<QObject*> m_keyframes;
};

#endif // TIMELINE_H
