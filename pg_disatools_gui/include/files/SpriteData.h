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
#ifndef INCLUDE_FILES_SPRITEDATA_H_
#define INCLUDE_FILES_SPRITEDATA_H_

#include <QList>
#include <QObject>
#include <QImage>
#include <qvector2d.h>
#include <QAbstractListModel>
#include <QColor>
#include <pg/util/PG_Image.h>
#include <vector>
#include <QQuickImageProvider>

class Cutout: public QObject{
	Q_OBJECT
    Q_PROPERTY(bool isExternalSheet 			READ isExternalSheet NOTIFY onExternalSheetIDChanged)
    Q_PROPERTY(unsigned char externalSheetID 	READ getExternalSheetID WRITE setExternalSheetID NOTIFY onExternalSheetIDChanged)
	Q_PROPERTY(unsigned short cutoutWidth 		READ getWidth NOTIFY onWidthChanged)
	Q_PROPERTY(unsigned short cutoutHeight 		READ getHeight NOTIFY onHeightChanged)
	//Q_PROPERTY(QImage cutout READ getCutout WRITE setCutout NOTIFY onCutoutChanged)
public:
	explicit Cutout(QObject *parent = 0);
	explicit Cutout(const PG::UTIL::IDImage& img, QObject *parent = 0);
	explicit Cutout(unsigned char externalSheetIDIn, QObject *parent = 0);
	explicit Cutout(unsigned char externalSheetIDIn, unsigned short widthIn, unsigned short heightIn,QObject *parent = 0);
	Cutout(const Cutout& cutout);
	virtual ~Cutout();

	//getters
	bool isExternalSheet() const;
	unsigned short getExternalSheetID() const;

	const PG::UTIL::IDImage& getCutout() const;
	unsigned short getWidth() const;
	unsigned short getHeight() const;

	//setters
	void setExternalSheetID(unsigned short externalSheetIDIn);
	void setCutout(const PG::UTIL::IDImage& img);

signals:
	void onExternalSheetIDChanged();

	void onCutoutChanged();
	void onWidthChanged();
	void onHeightChanged();
private:
	unsigned short  m_externalSheetID = 0; // get a sheet from different file by it's ID
	PG::UTIL::IDImage m_cutout;
};

Q_DECLARE_METATYPE( Cutout );

class Layer: public QObject, public QQuickImageProvider
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

	Q_PROPERTY(QImage image		READ getImage NOTIFY onImageChanged)

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

    QImage getImage() const;

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

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

    void onImageChanged();

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
Q_DECLARE_METATYPE( Layer* );


class Keyframe : public QAbstractListModel
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

    void push_backLayer(Layer* layer);
    void push_backLayer(unsigned int cutoutIDIn, unsigned char colortableIDIn,
    		short anchorxIn, short anchoryIn,
			unsigned short scalexIn, unsigned short scaleyIn,
			short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn);

    //getters
    int getDuration() const;
    int getNumberOfLayers() const;
    const QList<Layer*>& getLayers() const;

    //setters
    void setDuration(int duration);

	//QAbstractListModel
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const final;
	virtual int rowCount(const QModelIndex & parent = QModelIndex()) const final;

signals:
    void onDurationChanged();
    void onNumberOfLayersChanged();

private:
    int m_duration = 1;
    QList<Layer*> m_layers;
};

Q_DECLARE_METATYPE( Keyframe );
Q_DECLARE_METATYPE( Keyframe* );

class SpriteAnimation: public QAbstractListModel
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

	void push_backKeyframe(int duration);
	void push_backKeyframe(Keyframe* key);

	//getters
	unsigned int getID() const;
	const QString& getName() const;
	int getNumberOfKeyframes() const;
	const QList<Keyframe*>& getKeyframes() const;
	unsigned int getTotalFrames() const;

	//setters
	void setID(unsigned int idIn);
	void setName(const QString& nameIn);

	//QAbstractListModel
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const final;
	virtual int rowCount(const QModelIndex & parent = QModelIndex()) const final;
signals:
	void onIDChanged();
	void onNameChanged();
	void onNumberOfKeyframesChanged();

private:
	unsigned int m_ID = 0;
	QString m_name;
	QList<Keyframe*> m_keyframes;
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
	int getNumberOfCutouts() const;
	int getNumberOfColortables() const;
	int getCurrentAnimationIndex() const;

	const SpriteAnimation* getCurrentAnimation() const;
	const QList<Cutout*>& getCutouts() const;
	const QList<QColor>& getColortable() const;
	const std::vector<PG::UTIL::rgba>& getColortableGL() const;

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
	QList<SpriteAnimation*> m_aniamtions;
	QList<Cutout*> m_cutouts;
	QList<QColor> m_colortable;
	std::vector<PG::UTIL::rgba> m_colortableGL;
};

#endif /* INCLUDE_FILES_SPRITEDATA_H_ */
