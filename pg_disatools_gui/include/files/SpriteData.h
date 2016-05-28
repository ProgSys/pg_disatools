/*
 * SpriteData.h
 *
 *  Created on: 28.05.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_FILES_SPRITEDATA_H_
#define INCLUDE_FILES_SPRITEDATA_H_

#include <QList>
#include <QObject>
#include <QImage>
#include <qvector2d.h>
#include <QAbstractListModel>

class Cutout: public QObject{
	Q_OBJECT
    Q_PROPERTY(bool isExternalSheet 			READ isExternalSheet NOTIFY onExternalSheetIDChanged)
    Q_PROPERTY(unsigned char externalSheetID 	READ getExternalSheetID WRITE setExternalSheetID NOTIFY onExternalSheetIDChanged)
	Q_PROPERTY(unsigned short cutoutWidth 		READ getWidth NOTIFY onWidthChanged)
	Q_PROPERTY(unsigned short cutoutHeight 		READ getHeight NOTIFY onHeightChanged)
	Q_PROPERTY(QImage cutout READ getCutout WRITE setCutout NOTIFY onCutoutChanged)
public:
	explicit Cutout(QObject *parent = 0);
	explicit Cutout(const QImage& img, QObject *parent = 0);
	explicit Cutout(unsigned char externalSheetIDIn, unsigned short widthIn, unsigned short heightIn
			,QObject *parent = 0);
	Cutout(const Cutout& cutout);
	virtual ~Cutout();

	//getters
	bool isExternalSheet() const;
	unsigned short getEexternalSheetID() const;

	const QImage& getCutout() const;
	unsigned short getWidth() const;
	unsigned short getHeight() const;

	//setters
	void setEexternalSheetID(unsigned short externalSheetIDIn);
	void setCutout(const QImage& img);
	void setWidth(unsigned short widthIn);
	void setHeight(unsigned short heightIn);

signals:
	void onExternalSheetIDChanged();

	void onCutoutChanged();
	void onWidthChanged();
	void onHeightChanged();
private:
	unsigned short  m_externalSheetID = 0; // get a sheet from different file by it's ID
	QImage m_cutout;
};

Q_DECLARE_METATYPE( Cutout );

class Layer: public QObject
{
    Q_OBJECT
    Q_PROPERTY(unsigned int cutoutID 		READ getCutoutID WRITE setCutoutID NOTIFY onCutoutIDChanged)
    Q_PROPERTY(unsigned char colortableID 	READ getColortableID WRITE setColortableID NOTIFY onColortableIDChanged)
	Q_PROPERTY(short anchorx 				READ getAnchorX WRITE setAnchorX NOTIFY onAnchorXChanged)
	Q_PROPERTY(short anchory 				READ getAnchorY WRITE setAnchorY NOTIFY onAnchorYChanged)
	Q_PROPERTY(unsigned short scalex 		READ getScaleX WRITE setScaleX NOTIFY onScaleXChanged)
	Q_PROPERTY(unsigned short scaley 		READ getScaleY WRITE setScaleY NOTIFY onScaleYChanged)
	Q_PROPERTY(short offsetx 				READ getOffsetX WRITE setOffsetX NOTIFY onOffsetXChanged)
	Q_PROPERTY(short offsety 				READ getOffsetY WRITE setOffsetY NOTIFY onOffsetYChanged)

	Q_PROPERTY(short rotation 				READ getRotation WRITE setRotation NOTIFY onRotationChanged)
	Q_PROPERTY(unsigned char mirror 		READ getMirror WRITE setMirror NOTIFY onMirrorChanged)

public:
    explicit Layer(QObject *parent = 0);
    explicit Layer(unsigned int cutoutIDIn, unsigned char colortableIDIn,
    		short anchorxIn, short anchoryIn,
			unsigned short scalexIn, unsigned short scaleyIn,
			short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn, QObject *parent = 0);
    Layer(const Layer& layer);
    virtual ~Layer();

    void operator =(const Layer& layer);

    //getters
    unsigned int getCutoutID() const;
    unsigned char getColortableID() const;

    short getAnchorX() const;
    short getAnchorY() const;

    unsigned short getScaleX() const;
    unsigned short getScaleY() const;

    short getOffsetX() const;
    short getOffsetY() const;

    short getRotation() const;
    unsigned char getMirror() const;

    //setters
    void setCutoutID(unsigned int cutoutIDIn);
    void setColortableID(unsigned char colortableIDIn);

    void setAnchorX(short anchorxIn);
    void setAnchorY(short anchoryIn);

    void setScaleX(unsigned short scalexIn);
    void setScaleY(unsigned short scaleyIn);

    void setOffsetX(short offsetxIn);
    void setOffsetY(short offsetyIn);

    void setRotation(short rotationIn);
    void setMirror(unsigned char mirrorIn);

signals:
    void onCutoutIDChanged();
    void onColortableIDChanged();

    void onAnchorXChanged();
    void onAnchorYChanged();

    void onScaleXChanged();
    void onScaleYChanged();

    void onOffsetXChanged();
    void onOffsetYChanged();

    void onRotationChanged();
    void onMirrorChanged();

private:
	unsigned int m_cutoutID = 0;
	unsigned char m_colortableID = 0; // the 16 rgba colortable which the sheet should use

	short m_anchorx = 0; // rotation and mirror point
	short m_anchory = 0; // rotation and mirror point
	unsigned short m_scalex = 100; // 0-100
	unsigned short m_scaley = 100; // 0-100
	short m_offsetx = 0; // offset from the anchor
	short m_offsety = 0; // offset from the anchor
	short m_rotation = 0; // is degree

	unsigned char m_mirror = 0;
};

Q_DECLARE_METATYPE( Layer );

class Keyframe : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int duration READ getDuration WRITE setDuration NOTIFY onDurationChanged)
    Q_PROPERTY(int layersSize READ getNumberOfLayers NOTIFY onNumberOfLayersChanged)
public:
    explicit Keyframe(QObject *parent = 0);
    explicit Keyframe(int duration, QObject *parent = 0);
    Keyframe(const Keyframe& keyframe);
    virtual ~Keyframe();

    void operator =(const Keyframe& keyframe);

    void pushLayer(const Layer& layer);
    void pushLayer(unsigned int cutoutIDIn, unsigned char colortableIDIn,
    		short anchorxIn, short anchoryIn,
			unsigned short scalexIn, unsigned short scaleyIn,
			short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn);

    //getters
    int getDuration() const;
    int getNumberOfLayers() const;

    //setters
    void setDuration(int duration);

signals:
    void onDurationChanged();
    void onNumberOfLayersChanged();

private:
    int m_duration = 1;
    QList<Layer> m_layers;
};

Q_DECLARE_METATYPE( Keyframe );

class SpriteAnimation: public QObject
{
	Q_OBJECT
    Q_PROPERTY(unsigned int id READ getID WRITE setID NOTIFY onIDChanged)
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY onNameChanged)
	Q_PROPERTY(int keyframesSize READ getNumberOfKeyframes NOTIFY onNumberOfKeyframesChanged)
public:
	explicit SpriteAnimation(QObject *parent = 0);
	explicit SpriteAnimation(unsigned int IDin, const QString& nameIn, QObject *parent = 0);
	SpriteAnimation(const SpriteAnimation& ani);
	virtual ~SpriteAnimation();

	void operator =(const SpriteAnimation& ani);

	//getters
	unsigned int getID() const;
	const QString& getName() const;
	int getNumberOfKeyframes() const;
	const QList<Keyframe>& getKeyframes() const;
	unsigned int getTotalFrames() const;

	//setters
	void setID(unsigned int idIn);
	void setName(const QString& nameIn);

signals:
	void onIDChanged();
	void onNameChanged();
	void onNumberOfKeyframesChanged();

private:
	unsigned int m_ID = 0;
	QString m_name;
	QList<Keyframe> m_keyframes;
};

Q_DECLARE_METATYPE( SpriteAnimation );


class SpriteData : public QAbstractListModel{
	 Q_OBJECT
	 Q_PROPERTY(int animationsSize READ getNumberOfAnimations NOTIFY onNumberOfAnimationsChanged)
	 Q_PROPERTY(int currentAnimationIndex READ getCurrentAnimationIndex  WRITE setCurrentAnimationByIndex NOTIFY onCurrentAnimationChanged)
public:
	SpriteData(QObject *parent = 0);
	virtual ~SpriteData();

	// functions
	bool open(const QString& file);
	bool save(const QString& file);

	bool importSH(const QString& file);
	bool exportSH(const QString& file);

	void close();

	// getters
	bool isOpen() const;
	int getNumberOfAnimations() const;
	int getCurrentAnimationIndex() const;
	const SpriteAnimation& getCurrentAnimation() const;

	//setters
	void setCurrentAnimationByIndex(int index);

	// QAbstractListModel
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const final;
	virtual int rowCount(const QModelIndex & parent = QModelIndex()) const final;

signals:
	void onNumberOfAnimationsChanged();
	void onCurrentAnimationChanged();
	void onAnimationChanged(SpriteAnimation* ani);

private:
	QString m_lastFile;

	int m_currentAnimation = -1;
	QList<SpriteAnimation> m_aniamtions;
	QList<Cutout> m_cutouts;
};

#endif /* INCLUDE_FILES_SPRITEDATA_H_ */
