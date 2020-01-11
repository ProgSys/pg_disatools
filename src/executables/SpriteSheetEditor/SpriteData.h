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
#pragma once

#include <Marker.h>
#include <QList>
#include <QObject>
#include <QImage>
#include <QDataStream>
#include <qvector2d.h>
#include <QAbstractListModel>
#include <QColor>
#include <Util/PG_Image.h>
#include <vector>
#include <QKeyEvent>
#include <QVector>

typedef QVector<QColor> QColorTable;

class Cutout: public QObject{
	Q_OBJECT
    Q_PROPERTY(bool isExternalSheet 			READ isExternalSheet NOTIFY onExternalSheetIDChanged)
    Q_PROPERTY(unsigned char externalSheetID 	READ getExternalSheetID WRITE setExternalSheetID NOTIFY onExternalSheetIDChanged)
	Q_PROPERTY(int id READ getSheetID  NOTIFY onSheetIDChanged)

	Q_PROPERTY(int x READ getX WRITE setX NOTIFY onXChanged)
	Q_PROPERTY(int y READ getY WRITE setY NOTIFY onYChanged)
	Q_PROPERTY(unsigned short width READ getWidth WRITE setWidth NOTIFY onWidthChanged)
	Q_PROPERTY(unsigned short height READ getHeight WRITE setHeight NOTIFY onHeightChanged)

	Q_PROPERTY(unsigned int colortable READ getDefaultColorTable WRITE setDefaultColorTable NOTIFY onDefaultColorTableChanged)
	Q_PROPERTY(bool hidden READ isHidden WRITE setHidden NOTIFY onHiddenChanged)

	//Q_PROPERTY(QImage cutout READ getCutout WRITE setCutout NOTIFY onCutoutChanged)
public:
	explicit Cutout(QObject *parent = 0);
	explicit Cutout(int sheetID, const PG::UTIL::ivec2& position, const PG::UTIL::ivec2& size, QObject *parent = 0);
	explicit Cutout(int sheetID, const PG::UTIL::ivec2& position, const PG::UTIL::ivec2& size, unsigned int defaultColorTable, QObject *parent = 0);
	explicit Cutout(int sheetID, unsigned short externalSheetIDIn, const PG::UTIL::ivec2& position, const PG::UTIL::ivec2& size,  QObject *parent = 0);
	explicit Cutout(int sheetID, unsigned short externalSheetIDIn, const PG::UTIL::ivec2& position, const PG::UTIL::ivec2& size, unsigned int defaultColorTable, QObject *parent = 0);
	Cutout(const Cutout& cutout);
	virtual ~Cutout();

	bool isSame(int x,int y, int width, int height, int sheetID) const;

	//getters
	int getSheetID() const;
	bool isExternalSheet() const;
	unsigned short getExternalSheetID() const;

	const PG::UTIL::ivec2& getPosition() const;
	int getX() const;
	int getY() const;
	const PG::UTIL::ivec2& getSize() const;
	unsigned short getWidth() const;
	unsigned short getHeight() const;

	unsigned int getDefaultColorTable() const;
	bool isHidden() const;

	//setters
	void setSheetID(int id);
	void setExternalSheetID(unsigned short externalSheetIDIn);

	void setPosition(const PG::UTIL::ivec2& pos);
	void setPosition(int x, int y);
	void setX(int x);
	void setY(int y);

	void setSize(const PG::UTIL::ivec2& size);
	void setSize(int width, int height);
	void setWidth(int x);
	void setHeight(int y);

	void setDefaultColorTable(unsigned int tableID);

	void setHidden(bool hide);

signals:
	void onExternalSheetIDChanged();
	void onSheetIDChanged();

	void onCutoutChanged();
	void onPositionChanged();
	void onXChanged();
	void onYChanged();
	void onWidthChanged();
	void onHeightChanged();

	void onDefaultColorTableChanged();
	void onHiddenChanged();

private:
	int m_sheetID = -1;
	unsigned short  m_externalSheetID = 0; // get a sheet from different file by it's ID
	unsigned int m_defaultColorTable = 0;
	PG::UTIL::ivec2 m_position = PG::UTIL::ivec2(0,0);
	PG::UTIL::ivec2 m_size = PG::UTIL::ivec2(0,0);
	bool m_hide = false;
};

Q_DECLARE_METATYPE( Cutout );
Q_DECLARE_METATYPE( Cutout* );

//Forward deceleration
class Layer;

class Keyframe: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int start 		READ getStart WRITE setStart NOTIFY onStartChanged)
    Q_PROPERTY(int duration 	READ getDuration WRITE setDuration NOTIFY onDurationChanged)

    Q_PROPERTY(unsigned int cutoutID 		READ getCutoutID WRITE setCutoutID NOTIFY onCutoutIDChanged)
    Q_PROPERTY(unsigned int colortableID 	READ getColortableID WRITE setColortableID NOTIFY onColortableIDChanged)
	Q_PROPERTY(short anchorx 				READ getAnchorX WRITE setAnchorX NOTIFY onAnchorXChanged)
	Q_PROPERTY(short anchory 				READ getAnchorY WRITE setAnchorY NOTIFY onAnchorYChanged)
	Q_PROPERTY(unsigned short scalex 		READ getScaleX WRITE setScaleX NOTIFY onScaleXChanged)
	Q_PROPERTY(unsigned short scaley 		READ getScaleY WRITE setScaleY NOTIFY onScaleYChanged)
	Q_PROPERTY(short offsetx 				READ getOffsetX WRITE setOffsetX NOTIFY onOffsetXChanged)
	Q_PROPERTY(short offsety 				READ getOffsetY WRITE setOffsetY NOTIFY onOffsetYChanged)

	Q_PROPERTY(short rotation 				READ getRotation WRITE setRotation NOTIFY onRotationChanged)
	Q_PROPERTY(int transparency		READ getTransparency WRITE setTransparency NOTIFY onTransparencyChanged)
	Q_PROPERTY(int mic 		READ getMic WRITE setMic NOTIFY onMicChanged)
	Q_PROPERTY(bool mirroredHorizontally 	READ isMirroredHorizontally WRITE setMirroredHorizontally NOTIFY horizontalMirrorChanged)
	Q_PROPERTY(bool mirroredVertically 		READ isMirroredVertically WRITE setMirroredVertically NOTIFY verticalMirrorChanged)
	Q_PROPERTY(bool adaptive 		READ isAdaptive WRITE setAdaptive NOTIFY adaptiveChanged)

	Q_PROPERTY(bool selected 		READ isSelected WRITE setSelected NOTIFY onSelectionChanged)

	Q_PROPERTY(bool hasNext READ hasNext NOTIFY onStartChanged)
	Q_PROPERTY(bool hasPrevious READ hasPrevious NOTIFY onStartChanged)

	friend class Layer;
public:
    explicit Keyframe(QObject *parent = 0);
    explicit Keyframe(Keyframe* previousIn, int startIn, int durationIn, unsigned int cutoutIDIn, unsigned char colortableIDIn,
    		short anchorxIn, short anchoryIn,
			unsigned short scalexIn, unsigned short scaleyIn,
			short offsetxIn, short offsetyIn, short rotationIn, unsigned char transparencyIn, unsigned char micIn, QObject *parent = 0);

    explicit Keyframe(int startIn, int durationIn, unsigned int cutoutIDIn, unsigned char colortableIDIn,
    		short anchorxIn, short anchoryIn,
			unsigned short scalexIn, unsigned short scaleyIn,
			short offsetxIn, short offsetyIn, short rotationIn, unsigned char transparencyIn, unsigned char micIn, QObject *parent = 0);
    Keyframe(const Keyframe& keyframe);
    virtual ~Keyframe();

    void operator =(const Keyframe& keyframe);
    bool operator ==(const Keyframe* keyframe) const;
    bool isSame(unsigned int cutoutIDIn, unsigned char colortableIDIn,
    		short anchorxIn, short anchoryIn,
			unsigned short scalexIn, unsigned short scaleyIn,
			short offsetxIn, short offsetyIn, short rotationIn, unsigned char transparencyIn, unsigned char micIn) const;


    //getters
    int getStart() const;
    int getDuration() const;
    int getEnd() const;

    unsigned int getCutoutID() const;
    unsigned char getColortableID() const;

    short getAnchorX() const;
    short getAnchorY() const;

    unsigned short getScaleX() const;
    unsigned short getScaleY() const;

    short getOffsetX() const;
    short getOffsetY() const;

    short getRotation() const;
    unsigned char getTransparency() const;

    //mirror and render mods
    unsigned char getMic() const;
    bool isMirroredHorizontally() const;
    bool isMirroredVertically() const;
    bool isAdaptive() const;

    Keyframe* getNext();
    const Keyframe* getNext() const;
    bool hasNext() const;
    Keyframe* getPrevious();
    const Keyframe* getPrevious() const;
    bool hasPrevious() const;

    bool isSelected() const;


    //setters
    Q_INVOKABLE void setStart(int startIn, bool keepDocked = false);
    Q_INVOKABLE void setDuration(int durationIn, bool keepDocked = false);
    Q_INVOKABLE void moveTo(int frame);

    void setCutoutID(unsigned int cutoutIDIn);
    void setColortableID(unsigned char colortableIDIn);

    void setAnchorX(short anchorxIn);
    void setAnchorY(short anchoryIn);

    void setScaleX(unsigned short scalexIn);
    void setScaleY(unsigned short scaleyIn);

    void setOffsetX(short offsetxIn);
    void setOffsetY(short offsetyIn);
    void moveOffset(short offsetxIn, short offsetyIn);

    void setRotation(short rotationIn);
    void setTransparency(unsigned char transparencyIn);

    void setMic(unsigned char micIn);
    void setMirroredHorizontally(bool mirrored);
    void setMirroredVertically(bool mirrored);
    void setAdaptive(bool addaptive);

    void setSelected(bool select);

    Q_INVOKABLE bool swapNext();
    Q_INVOKABLE bool swapPrevious();

signals:
	void onStartChanged();
	void onDurationChanged();

    void onCutoutIDChanged();
    void onColortableIDChanged();

    void onAnchorXChanged();
    void onAnchorYChanged();

    void onScaleXChanged();
    void onScaleYChanged();

    void onOffsetXChanged();
    void onOffsetYChanged();

    void onRotationChanged();

    void onTransparencyChanged();
    void onMicChanged();

    void horizontalMirrorChanged();
    void verticalMirrorChanged();
    void adaptiveChanged();

    void onSelectionChanged();

protected:
    void setNext(Keyframe* nextIn);
    void setPrevious(Keyframe* previousIn);
    void setDurationDirrect(int durr);
    void setStartDirrect(int frame);
private:
    int m_start = 0;
    int m_duration = 0;

	unsigned int m_cutoutID = 0;
	unsigned char m_colortableID = 0; // the 16 rgba colortable which the sheet should use

	short m_anchorx = 0; // rotation and mirror point
	short m_anchory = 0; // rotation and mirror point
	unsigned short m_scalex = 100; // 0-100
	unsigned short m_scaley = 100; // 0-100
	short m_offsetx = 0; // offset from the anchor
	short m_offsety = 0; // offset from the anchor
	short m_rotation = 0; // is degree

	unsigned char m_transparency = 128;
	unsigned char m_mic = 0;

	bool m_selected = false;

	Keyframe* m_next = nullptr;
	Keyframe* m_previous = nullptr;
};

Q_DECLARE_METATYPE( Keyframe );
Q_DECLARE_METATYPE( Keyframe* );


class Layer : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY onNameChanged)
    Q_PROPERTY(int duration READ getDuration NOTIFY onDurationChanged)
    Q_PROPERTY(int keyframesSize READ getNumberOfKeyframes NOTIFY onNumberOfKeyframesChanged)
	Q_PROPERTY(bool hidden READ isHidden WRITE setHidden NOTIFY hiddenChanged)
public:
    explicit Layer(QObject *parent = 0);
    explicit Layer(const QString& name, QObject *parent = 0);
    explicit Layer(const QString& name, bool hidden, QObject *parent = 0);
    Layer(const Layer& layer);
    virtual ~Layer();

    void operator =(const Layer& layer);

    void push_backKeyframe(Keyframe* keyframe);

    void push_backKeyframe(int duration, unsigned int cutoutIDIn, unsigned char colortableIDIn,
    		short anchorxIn, short anchoryIn,
			unsigned short scalexIn, unsigned short scaleyIn,
			short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn, unsigned char unknown, bool seperate = false);

    void push_backKeyframe(int start, int duration, unsigned int cutoutIDIn, unsigned char colortableIDIn,
    		short anchorxIn, short anchoryIn,
			unsigned short scalexIn, unsigned short scaleyIn,
			short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn, unsigned char unknown, bool seperate = false);

    Q_INVOKABLE bool insertKeyframe(int frame);
    //Q_INVOKABLE bool pushBackKeyframe();
    Q_INVOKABLE bool splitKeyframe(int frame);
    Q_INVOKABLE bool removeKeyframe(Keyframe* keyframe);


    //getters
    const QString& getName() const;
    int getDuration() const;
    int getNumberOfKeyframes() const;
    const QList<Keyframe*>& getKeyframes() const;
    bool isHidden() const;

    //setters
    void setName( const QString& name);
    void setHidden(bool hidden);

	//QAbstractListModel
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const final;
	virtual int rowCount(const QModelIndex & parent = QModelIndex()) const final;

signals:
    void onNameChanged();
    void onDurationChanged();
    void onNumberOfKeyframesChanged();
    void hiddenChanged();

private:
    QString m_name;
    QList<Keyframe*> m_keyframes;
    bool m_hidden = false;
};

Q_DECLARE_METATYPE( Layer );
Q_DECLARE_METATYPE( Layer* );


class SpriteAnimation: public QAbstractListModel
{
	Q_OBJECT
    Q_PROPERTY(unsigned int id READ getID WRITE setID NOTIFY onIDChanged)
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY onNameChanged)
	Q_PROPERTY(int layerSize READ getNumberOfLayers NOTIFY onNumberOfLayersChanged)
	Q_PROPERTY(int length READ getTotalFrames NOTIFY onLengthChanged)
	Q_PROPERTY(int workspacelength READ getTotalWorkspace NOTIFY onLengthChanged)
public:
	explicit SpriteAnimation(QObject *parent = 0);
	explicit SpriteAnimation(unsigned int IDin, const QString& nameIn, QObject *parent = 0);
	SpriteAnimation(const SpriteAnimation& ani);
	virtual ~SpriteAnimation();

	void operator =(const SpriteAnimation& ani);

	void push_backLayer(const QString& name);
	void push_backLayer(Layer* layer);

	//getters
	unsigned int getID() const;
	const QString& getName() const;
	unsigned int getTotalFrames() const;
	unsigned int getTotalWorkspace() const;

	QList<Layer*>& getLayers();
	const QList<Layer*>& getLayers() const;
	int getNumberOfLayers() const;
	int getNumberOfMarkers() const;

	Q_INVOKABLE MarkersList* getMarkers();
	const MarkersList* getMarkers() const;
	//setters

	void setID(unsigned int idIn);
	void setName(const QString& nameIn);

	void refresh();

	//markers
	Q_INVOKABLE bool moveMarker(Marker* mark, int frame );
	Q_INVOKABLE bool addMarker(int frame, int type = 1, short a = 0, short b = 0 );
	Q_INVOKABLE bool removeMarker(Marker* mark);

	//layers
	Q_INVOKABLE  bool moveUp(Layer* lay);
	Q_INVOKABLE  bool moveDown(Layer* lay);
	Q_INVOKABLE  void move(Layer* lay, int newPosition);

	Q_INVOKABLE  bool addLayer();
	Q_INVOKABLE  bool addLayer(Layer* lay);
	Q_INVOKABLE  bool remove(Layer* lay);

	//QAbstractListModel
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const final;
	virtual int rowCount(const QModelIndex & parent = QModelIndex()) const final;
signals:
	void onIDChanged();
	void onNameChanged();
	void onNumberOfLayersChanged();
	void onLengthChanged();
private:
	unsigned int m_ID = 0;
	QString m_name;
	QList<Layer*> m_Layers;
	MarkersList* m_Markers;
};

Q_DECLARE_METATYPE( SpriteAnimation );


class SpriteSheet: public QAbstractListModel{
	Q_OBJECT
	Q_PROPERTY(int width READ getWidth NOTIFY onWidthChanged)
	Q_PROPERTY(int height READ getHeight NOTIFY onHeightChanged)
	Q_PROPERTY(int colors READ getSizeOfColorTable NOTIFY numberOfColorsChanged)
	Q_PROPERTY(int size READ getNumberOfCutouts NOTIFY onNumberOfCutoutsChanged)
	Q_PROPERTY(bool isReference READ isReference NOTIFY referenceIDChanged)
	Q_PROPERTY(int referenceId READ getReferenceID WRITE setReferenceID NOTIFY referenceIDChanged)

public:
	SpriteSheet(QObject* parent = nullptr);
	SpriteSheet(int referenceID, QObject* parent = nullptr);
	SpriteSheet(int width, int height, int powerColorTable = 4, QObject *parent = nullptr);
	SpriteSheet(const PG::UTIL::IDImage& img, int powerColorTable = 4 , QObject *parent = nullptr);
	SpriteSheet(const SpriteSheet& sheet);
	virtual ~SpriteSheet();

	void operator= (const SpriteSheet& sheet);

	//getters
	int getWidth() const;
	int getHeight() const;
	int getNumberOfCutouts() const;
	int getPowerOfColorTable() const;
	int getSizeOfColorTable() const;
	inline bool isReference() const { return m_referenceID >= 0; }
	inline int getReferenceID() const { return m_referenceID; }

	PG::UTIL::IDImage& getSpriteSheet();
	const PG::UTIL::IDImage& getSpriteSheet() const;

	PG::UTIL::RGBAImage getSpritePG(unsigned int ColortableID, const QVector<QColor>& colortable) const;
	PG::UTIL::RGBAImage getSpritePG(const Cutout* cut, unsigned int ColortableID, const QVector<QColor>& colortable) const;
	QImage getSprite(unsigned int ColortableID, const QVector<QColor>& colortable, bool alpha = false) const;
	QImage getSprite(const Cutout* cut, unsigned int ColortableID, const QVector<QColor>& colortable, bool alpha = false) const;

	PG::UTIL::IDImage getSpritePGIDs(const Cutout* cut) const;
	QImage getSpriteIDs(const Cutout* cut) const;


	QList<int>& getCutoutIDs();
	const QList<int>& getCutoutIDs() const;
	bool removeCutoutID(int ID);
	void refresh();

	//setters
	void push_backCutoutID(int id);
	void set(int width, int height, int powerColorTable = 4, bool resizeSprites = false);
	void setReferenceID(int reference);

	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const final override;
	int rowCount(const QModelIndex & parent = QModelIndex()) const final override;

signals:
	void onWidthChanged();
	void onHeightChanged();
	void numberOfColorsChanged();
	void onNumberOfCutoutsChanged();
	void referenceIDChanged();
private:
	PG::UTIL::IDImage m_img;
	int m_powerOfColoTable = -1;
	QList<int> m_cutoutsIDs;
	int m_referenceID = -1;
};

Q_DECLARE_METATYPE( SpriteSheet );
Q_DECLARE_METATYPE( SpriteSheet* );

class SpriteData : public QAbstractListModel{
	 Q_OBJECT
	 Q_PROPERTY(int animationsSize READ getNumberOfAnimations NOTIFY onNumberOfAnimationsChanged)
	 Q_PROPERTY(int currentAnimationIndex READ getCurrentAnimationIndex WRITE setCurrentAnimationByIndex NOTIFY onCurrentAnimationChanged)
	 Q_PROPERTY(int cutoutSize READ getNumberOfCutouts NOTIFY onNumberOfCutoutsChanged)
	 Q_PROPERTY(int colortableSize READ getNumberOfColortableSets NOTIFY numberOfColorTablesChanged)
	 Q_PROPERTY(int colorTablesNumber READ getNumberOfColortables NOTIFY numberOfColorTablesChanged)
	 Q_PROPERTY(int colorsSize READ getNumberOfColors NOTIFY numberOfColorTablesChanged)
	 Q_PROPERTY(int sheetsSize READ getNumberOfSpriteSheets NOTIFY numberOfSheetsChanged)

	 Q_PROPERTY(int colorTable READ getCurrentColorTable WRITE setCurrentColorTable NOTIFY currentColorTableChanged)
	 Q_PROPERTY(bool isolateSelection READ getIsolateSelection WRITE setIsolateSelection NOTIFY isolateSelectionChanged)

	 Q_PROPERTY(Cutout* selected READ getSelected WRITE setSelected NOTIFY selectedChanged)
	 Q_PROPERTY(QString fileName READ getLastFileName NOTIFY onLastFileNameChanged)
	 Q_PROPERTY(SpriteAnimation* animation READ getCurrentAnimation NOTIFY onCurrentAnimationChanged)
	 Q_PROPERTY(Keyframe* selectedKey READ getSelectedKey WRITE setSelectedKey NOTIFY selectedKeyChanged)
	 Q_PROPERTY(QColorTable colortable READ getColorTable NOTIFY currentColorTableChanged)

signals:
	void onNumberOfAnimationsChanged();
	void onCurrentAnimationChanged();
	void onAnimationChanged(SpriteAnimation* ani);
	void onLastFileNameChanged();
	void onSelectionChanged();
	void onNumberOfCutoutsChanged();

	void selectedChanged();
	void selectedKeyChanged();

	void numberOfColorTablesChanged();
	void currentColorTableChanged();
	void isolateSelectionChanged();
	void allColorTablesChanged();
	void colorTableChanged(int colorTableIndex);
	void colorTableAdded(int spritesheetID);
	void colorTableRemoved(int colorTableIndex);

	void numberOfSheetsChanged();
	void allSpriteSheetsChanged();
	void spriteSheetChanged(int spritesheetID);
	void spriteSheetAdded();
	void spriteSheetRemoved(int spritesheetID);

	void refresh();

public:
	SpriteData(QObject *parent = 0);
	virtual ~SpriteData();


	// getters
	bool isOpen() const;
	int getNumberOfAnimations() const;
	int getNumberOfCutouts() const;
	int getNumberOfColortables() const;
	int getNumberOfColortableSets() const;
	int getNumberOfColors() const;
	int getMaxUsedColortable() const;
	int getNumberOfSpriteSheets() const;
	int getCurrentAnimationIndex() const;
	QString getLastFileName() const;
	bool getIsolateSelection() const { return m_isolateSelection; }

	const QList<SpriteAnimation*>& getAnimations() const;
	bool push_backAnimation(const QString& name, int ID);
	bool removeAnimation(int index);
	SpriteAnimation* getCurrentAnimation();
	const SpriteAnimation* getCurrentAnimation() const;
	const QList<Cutout*>& getCutouts() const;
	const QList<SpriteSheet*>& getSpriteSheets() const;

	std::vector<PG::UTIL::rgba> getColortableGL(int index) const;
	QImage getSprite(unsigned int CutoutID, unsigned int ColortableID) const;
	const SpriteSheet* getSpriteSheet(unsigned int spriteID) const;
	inline Cutout* getSelected() const { return m_selected; }
	inline Keyframe* getSelectedKey() const { return m_selectedKeyframe; };

	//setters
	void setCurrentAnimationByIndex(int index);
	void setSelected(Cutout* cutout);
	void setSelectedKey(Keyframe* key);
	void setIsolateSelection(bool value);

	void keyPressEvent(QKeyEvent * event);

	// QAbstractListModel
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const final;
	virtual int rowCount(const QModelIndex & parent = QModelIndex()) const final;

public slots:
	// functions
	bool open(const QString& file);
	bool save(const QString& file);

	bool importSH(const QString& file);
	bool exportSH(const QString& file);

	void close();

	Q_INVOKABLE int findReferenceSpriteSheetIndex(int referenceId) const;

	///if png is false then tga is used
	Q_INVOKABLE int exportSprites(const QString& folder, const QString& type);
	Q_INVOKABLE int exportSpritesIDs(const QString& folder, const QString& type);

	Q_INVOKABLE bool exportSprite(int cutoutID);
	Q_INVOKABLE bool exportSprite(int cutoutID, const QString& file);

	Q_INVOKABLE bool exportSpriteIDs(int cutoutID);
	Q_INVOKABLE bool exportSpriteIDs(int cutoutID, const QString& file);

	Q_INVOKABLE bool exportSpriteSheet(Cutout* cutout);
	/**
	* @brief Exports the entiere sprite sheet to a color image file
	* @param cutout Used to determine the colortable
	*/
	Q_INVOKABLE bool exportSpriteSheet(Cutout* cutout, const QString& file);

	Q_INVOKABLE bool importSpriteAsIDs(int cutoutID, const QString& file = "");
	Q_INVOKABLE bool importSpriteAsIDs(int sheetID, int x, int y, int width, int height, const QString& file = "");
	Q_INVOKABLE bool importSpriteAsIDs(Cutout& cut, const QString& file = "");

	Q_INVOKABLE bool importSpriteAsColor(int cutoutID, const QString& file = "");
	Q_INVOKABLE bool importSpriteAsColorForSheet(int sheetID, const QString& file = "");
	Q_INVOKABLE bool importSpriteAsColor(int sheetID, int x, int y, int width, int height, int colorTableOffset, QImage& image);
	Q_INVOKABLE bool importSpriteAsColor(int sheetID, int x, int y, int width, int height, int colorTableOffset = -1, const QString& file = "");
	Q_INVOKABLE bool importSpriteAsColor(Cutout& cut, QImage& image);

	Q_INVOKABLE bool importSpriteSheetAsColor(Cutout* cutout);
	Q_INVOKABLE bool importSpriteSheetAsColor(Cutout* cutout, const QString& file);

	bool dump(const QString& filepath);

	Q_INVOKABLE Cutout* getCutout(int cutoutIndex) const;
	Q_INVOKABLE int getCoutoutIndex(Cutout* cutoutIndex) const;
	Q_INVOKABLE SpriteSheet* getSpriteSheet(int spriteSheetIndex) const;
	Q_INVOKABLE bool addCutout(int sheetID);
	Q_INVOKABLE bool addCutout(int sheetID,unsigned int x,unsigned int y = 0,unsigned int width = 100,unsigned int height = 100,unsigned char colorID = 0);
	bool removeCutout(Cutout* cut);
	Q_INVOKABLE bool removeCutoutID(int id, bool warning = true);
	Q_INVOKABLE void cropCutout(int cutoutIndex);
	Q_INVOKABLE void autoFindCutouts(int sheetID);

	Q_INVOKABLE bool addNewSpriteSheet();
	Q_INVOKABLE bool addNewSpriteSheet(int width, int height, int powerOfColorTable = 4);
	Q_INVOKABLE bool removeSpriteSheet(unsigned int index);
	Q_INVOKABLE bool editSpriteSheet(unsigned int index);

	Q_INVOKABLE void clearSelectedKey();
	Q_INVOKABLE void unhideAllCutouts();
	Q_INVOKABLE void update();

	//tools
	Q_INVOKABLE void batchDoubleResize();

	//color table
	Q_INVOKABLE int getCurrentColorTable() const;
	Q_INVOKABLE void setCurrentColorTable(int index = 0);
	Q_INVOKABLE QList<QColorTable>& getColorTables();
	Q_INVOKABLE const QList<QColorTable>& getColorTables() const;
	Q_INVOKABLE QColorTable& getColorTable();
	Q_INVOKABLE const QColorTable& getColorTable() const;

	Q_INVOKABLE void insertColorTable(int index = -1, bool copy = false);
	Q_INVOKABLE void removeColorTable(int index = -1);

	Q_INVOKABLE bool exportColortable(int index = 0);
	Q_INVOKABLE bool exportColortable(const QString& file, int index = 0);
	Q_INVOKABLE bool importColortable(int index = 0);
	Q_INVOKABLE bool importColortable(const QString& file, int index = 0);

	Q_INVOKABLE QColor getColor(int index) const;
	Q_INVOKABLE void setColor(int index,const QColor& color);
	Q_INVOKABLE void addColors(int index, int number = 16);
	Q_INVOKABLE void removeColors(int index, int number = 16);

	Q_INVOKABLE void renameCurrentAnimation();
	Q_INVOKABLE void renameCurrentAnimation(const QString& newName);
	Q_INVOKABLE void renameAnimation(const QString& newName, unsigned int index);


private:
	bool openPGSHv1(QDataStream& in);
	bool openPGSHv2(QDataStream& in);
	void resizeSpritesOnSheet(SpriteSheet* sheet, int oldWidth, int oldHeight, int targetWidth, int targetHeight);

	QString m_lastFile;

	int m_currentAnimation = -1;
	QList<SpriteAnimation*> m_aniamtions;
	QList<Cutout*> m_cutouts;

	int m_currentColorTable = -1;
	QList<QColorTable> m_colortables;

	QList<SpriteSheet*> m_spriteSheets;

	Cutout* m_selected = nullptr;
	Keyframe* m_selectedKeyframe = nullptr;


	bool m_isolateSelection = false;
};

QDebug operator<< (QDebug d, const PG::UTIL::ivec2 &m);
QDebug operator<< (QDebug d, const PG::UTIL::ivec3 &m);

