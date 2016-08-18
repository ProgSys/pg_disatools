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
#include <files/SpriteData.h>
#include <QException>
#include <pg/files/PG_SH.h>
#include <QDebug>

Cutout::Cutout(QObject *parent): QObject(parent){

}
Cutout::Cutout(const PG::UTIL::IDImage& img,QObject *parent): QObject(parent),m_cutout(img)
{}

Cutout::Cutout(const PG::UTIL::IDImage& img, const PG::UTIL::ivec2& position, QObject *parent):
		QObject(parent), m_position(position), m_cutout(img)
{}

Cutout::Cutout(unsigned char externalSheetIDIn, QObject *parent): QObject(parent),m_externalSheetID(externalSheetIDIn)
{}

Cutout::Cutout(unsigned char externalSheetIDIn, unsigned short widthIn, unsigned short heightIn,QObject *parent): QObject(parent),
		m_externalSheetID(externalSheetIDIn),m_cutout((int)widthIn,(int)heightIn)
{}

Cutout::Cutout(const Cutout& cutout): QObject(cutout.parent()),
		m_externalSheetID(cutout.getExternalSheetID()),
		m_cutout(cutout.getCutout())
{}

Cutout::~Cutout(){

}

bool Cutout::isSame(int x,int y, int width, int height, int sheetID) const{
	return !isExternalSheet() && m_position.x == x && m_position.y == y && m_cutout.getWidth() == width && m_cutout.getHeight() == height && m_sheetID == sheetID;
}

//getters

const PG::UTIL::IDImage& Cutout::getCutout() const{
	return m_cutout;
}

bool Cutout::isExternalSheet() const{
	return m_externalSheetID;
}
unsigned short Cutout::getExternalSheetID() const{
	return m_externalSheetID;
}


unsigned short Cutout::getWidth() const{
	return m_cutout.getWidth();
}
unsigned short Cutout::getHeight() const{
	return m_cutout.getHeight();
}

const PG::UTIL::ivec2& Cutout::getPosition() const{
	return m_position;
}

int Cutout::getX() const{
	return m_position.x;
}

int Cutout::getY() const{
	return m_position.y;
}

int Cutout::getSheetID() const{
	return m_sheetID;
}

//setters
void Cutout::setExternalSheetID(unsigned short externalSheetIDIn){
	if(m_externalSheetID == externalSheetIDIn) return;
	m_externalSheetID = externalSheetIDIn;
	emit onExternalSheetIDChanged();
}

void Cutout::setCutout(const PG::UTIL::IDImage& img){
	m_cutout = img;
	emit onCutoutChanged();
	emit onWidthChanged();
	emit onHeightChanged();
}

void Cutout::setPosition(const PG::UTIL::ivec2& pos){
	if(m_position.x == pos.x && m_position.y == pos.y) return;
	m_position = pos;
	emit onPositionChanged();
}

void Cutout::setPosition(int x, int y){
	if(m_position.x == x && m_position.y == y) return;
	m_position.x = x;
	m_position.y = y;
	emit onPositionChanged();
}

void Cutout::setX(int x){
	if(m_position.x == x) return;
	m_position.x = x;
	emit onPositionChanged();
}

void Cutout::setY(int y){
	if(m_position.y == y) return;
	m_position.y = y;
	emit onPositionChanged();
}

void Cutout::setSheetID(int id){
	if(m_sheetID == id) return;
	m_sheetID = id;

}

///// KEYFRAME /////

Keyframe::Keyframe(QObject *parent):QObject(parent), QQuickImageProvider(QQmlImageProviderBase::Image){

}

Keyframe::Keyframe(Keyframe* previousIn, int startIn, int durationIn, unsigned int cutoutIDIn, unsigned char colortableIDIn,
		short anchorxIn, short anchoryIn,
		unsigned short scalexIn, unsigned short scaleyIn,
		short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn, unsigned char unknown, QObject *parent):
					QObject(parent),QQuickImageProvider(QQmlImageProviderBase::Image),
					m_start(startIn), m_duration(durationIn),
					m_cutoutID(cutoutIDIn), m_colortableID(colortableIDIn),
					m_anchorx(anchorxIn),m_anchory(anchoryIn) ,
					m_scalex(scalexIn) ,m_scaley(scaleyIn),
					m_offsetx(offsetxIn), m_offsety(offsetyIn),
					m_rotation(rotationIn), m_mirror(mirrorIn), m_unknown(unknown),
					m_previous(previousIn)
		{
			if(previousIn && m_start < previousIn->getEnd())
				m_start = previousIn->getEnd();
		}

Keyframe::Keyframe(int startIn, int durationIn, unsigned int cutoutIDIn, unsigned char colortableIDIn,
		short anchorxIn, short anchoryIn,
		unsigned short scalexIn, unsigned short scaleyIn,
		short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn, unsigned char unknown, QObject *parent):
			QObject(parent),QQuickImageProvider(QQmlImageProviderBase::Image),
			m_start(startIn), m_duration(durationIn),
			m_cutoutID(cutoutIDIn), m_colortableID(colortableIDIn),
			m_anchorx(anchorxIn),m_anchory(anchoryIn) ,
			m_scalex(scalexIn) ,m_scaley(scaleyIn),
			m_offsetx(offsetxIn), m_offsety(offsetyIn),
			m_rotation(rotationIn), m_mirror(mirrorIn), m_unknown(unknown)
		{}

Keyframe::Keyframe(const Keyframe& keyframe): QObject(keyframe.parent()), QQuickImageProvider(QQmlImageProviderBase::Image),
		m_start(keyframe.getStart()), m_duration(keyframe.getDuration()),
		m_cutoutID(keyframe.getCutoutID()), m_colortableID(keyframe.getColortableID()),
		m_anchorx(keyframe.getAnchorX()),m_anchory(keyframe.getAnchorY()) ,
		m_scalex(keyframe.getScaleX()) ,m_scaley(keyframe.getScaleY()),
		m_offsetx(keyframe.getOffsetX()), m_offsety(keyframe.getOffsetY()),
		m_rotation(keyframe.getRotation()), m_mirror(keyframe.getMirror()), m_unknown(keyframe.getUnknown()),
		m_previous(const_cast<Keyframe*>(keyframe.getPrevious())), m_next(const_cast<Keyframe*>(keyframe.getNext()))
		{}

void Keyframe::operator =(const Keyframe& keyframe){
	setStart(keyframe.getStart());
	setDuration(keyframe.getDuration());

	setCutoutID(keyframe.getCutoutID());
	setColortableID(keyframe.getColortableID());
	setAnchorX(keyframe.getAnchorX());
	setAnchorX(keyframe.getAnchorY());

	setScaleX(keyframe.getScaleX());
	setScaleY(keyframe.getScaleY());

	setOffsetX(keyframe.getOffsetX());
	setOffsetY(keyframe.getOffsetY());

	setUnknown(keyframe.getUnknown());
	setRotation(keyframe.getRotation());

	setMirror(keyframe.getMirror());
	setPrevious(const_cast<Keyframe*> (keyframe.getPrevious()));
	setNext(const_cast<Keyframe*> (keyframe.getNext()));
}

bool Keyframe::operator ==(const Keyframe* keyframe) const{
	return keyframe && m_cutoutID == keyframe->getCutoutID() && m_colortableID == keyframe->getColortableID()
			&& m_anchorx == keyframe->getAnchorX() && m_anchory == keyframe->getAnchorY()
			&& m_scalex == keyframe->getScaleX() && m_scaley == keyframe->getScaleY()
			&& m_offsetx == keyframe->getOffsetX() && m_offsety == keyframe->getOffsetY()
			&& m_rotation == keyframe->getRotation() && m_unknown == keyframe->getUnknown()
			&& m_mirror == keyframe->getMirror();
}

bool Keyframe::isSame(unsigned int cutoutIDIn, unsigned char colortableIDIn,
		short anchorxIn, short anchoryIn,
		unsigned short scalexIn, unsigned short scaleyIn,
		short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn, unsigned char unknown) const{
	return  m_cutoutID == cutoutIDIn && m_colortableID == colortableIDIn
			&& m_anchorx == anchorxIn && m_anchory == anchoryIn
			&& m_scalex == scalexIn && m_scaley == scaleyIn
			&& m_offsetx == offsetxIn && m_offsety == offsetyIn
			&& m_rotation == rotationIn
			&& m_unknown == unknown && m_mirror == mirrorIn;
}

//getters

int Keyframe::getStart() const{
	return m_start;
}

int Keyframe::getDuration() const{
	return m_duration;
}

int Keyframe::getEnd() const{
	return m_start+m_duration;
}

unsigned int Keyframe::getCutoutID() const{
	return m_cutoutID;
}
unsigned char Keyframe::getColortableID() const{
	return m_colortableID;
}

short Keyframe::getAnchorX() const{
	return m_anchorx;
}
short Keyframe::getAnchorY() const{
	return m_anchory;
}

unsigned short Keyframe::getScaleX() const{
	return m_scalex;
}
unsigned short Keyframe::getScaleY() const{
	return m_scaley;
}

short Keyframe::getOffsetX() const{
	return m_offsetx;
}
short Keyframe::getOffsetY() const{
	return m_offsety;
}

short Keyframe::getRotation() const{
	return m_rotation;
}
unsigned char Keyframe::getMirror() const{
	return m_mirror;
}

unsigned char Keyframe::getUnknown() const{
	return m_unknown;
}

Keyframe* Keyframe::getNext(){
	return m_next;
}

const Keyframe* Keyframe::getNext() const{
	return m_next;
}

bool Keyframe::hasNext() const{
	return m_next;
}

Keyframe* Keyframe::getPrevious(){
	return m_previous;
}

const Keyframe* Keyframe::getPrevious() const{
	return m_previous;
}

bool Keyframe::hasPrevious() const{
	return m_previous;
}

QImage Keyframe::getImage() const{
	return QImage("resources/test.jpg");
}

QImage Keyframe::requestImage(const QString &id, QSize *size, const QSize &requestedSize){
	return getImage();
}

//setters
void Keyframe::setStart(int startIn){
	if(startIn == m_start) return;

	if(m_previous && startIn <= m_previous->getEnd()){
		m_start = m_previous->getEnd()+1;
	}else if(m_next && startIn >= m_next->getStart()){
		m_start = m_next->getStart()-1;
	}else
		m_start = startIn;
	emit onStartChanged();
}

void Keyframe::setDuration(int durationIn){
	if(durationIn <= 0 || durationIn == m_duration) return;

	if(m_next && (m_start+durationIn) >= m_next->getStart()){
		m_duration = m_next->getStart() - m_start;
	}else
		m_duration = durationIn;
	emit onDurationChanged();
}

void Keyframe::setCutoutID(unsigned int cutoutIDIn){
	if(cutoutIDIn == m_cutoutID) return;
	m_cutoutID = cutoutIDIn;
	emit onCutoutIDChanged();
}
void Keyframe::setColortableID(unsigned char colortableIDIn){
	if(colortableIDIn == m_colortableID) return;
	m_colortableID = colortableIDIn;
	emit onColortableIDChanged();
}

void Keyframe::setAnchorX(short anchorxIn){
	if(anchorxIn == m_anchorx) return;
	m_anchorx = anchorxIn;
	emit onAnchorXChanged();
}
void Keyframe::setAnchorY(short anchoryIn){
	if(anchoryIn == m_anchory) return;
	m_anchory = anchoryIn;
	emit onAnchorYChanged();
}

void Keyframe::setScaleX(unsigned short scalexIn){
	if(scalexIn == m_scalex) return;
	m_scalex = scalexIn;
	emit onScaleXChanged();
}
void Keyframe::setScaleY(unsigned short scaleyIn){
	if(scaleyIn == m_scaley) return;
	m_scaley = scaleyIn;
	emit onScaleYChanged();
}

void Keyframe::setOffsetX(short offsetxIn){
	if(offsetxIn == m_offsetx) return;
	m_offsetx = offsetxIn;
	emit onOffsetXChanged();
}
void Keyframe::setOffsetY(short offsetyIn){
	if(offsetyIn == m_offsety) return;
	m_offsety = offsetyIn;
	emit onOffsetYChanged();
}

void Keyframe::setRotation(short rotationIn){
	if(rotationIn == m_rotation) return;
	m_rotation = rotationIn;
	emit onRotationChanged();
}
void Keyframe::setMirror(unsigned char mirrorIn){
	if(mirrorIn == m_mirror) return;
	m_mirror = mirrorIn;
	emit onMirrorChanged();
}

void Keyframe::setUnknown(unsigned char unknowIn){
	if(unknowIn == m_unknown) return;
	m_unknown = unknowIn;
	emit onUnknownChanged();
}

void Keyframe::setNext(Keyframe* nextIn){
	m_next = nextIn;
}

void Keyframe::setPrevious(Keyframe* previousIn){
	m_previous = previousIn;
}

Keyframe::~Keyframe(){

}

///// LAYER /////
Layer::Layer(QObject *parent)
: QAbstractListModel(parent)
{

}

Layer::Layer(const QString& nameIn, QObject *parent)
    : QAbstractListModel(parent)
{
    if(nameIn.size())
    	m_name = nameIn;
}

Layer::Layer(const Layer &layer)
 : QAbstractListModel(nullptr)
{
	m_name = layer.getName();
	m_keyframes = layer.getKeyframes();
}

void Layer::operator =(const Layer &layer)
{
	m_name = layer.getName();
	m_keyframes = layer.getKeyframes();
}

void Layer::push_backKeyframe(Keyframe* keyframe){
	if(!keyframe) return;
	if(m_keyframes.size()){
		m_keyframes.last()->setNext(keyframe);
		keyframe->setPrevious(m_keyframes.last());
	}
	m_keyframes.push_back(keyframe);
	emit onNumberOfKeyframesChanged();
}

void Layer::push_backKeyframe(int duration, unsigned int cutoutIDIn, unsigned char colortableIDIn,
		short anchorxIn, short anchoryIn,
		unsigned short scalexIn, unsigned short scaleyIn,
		short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn, unsigned char unknown, bool seperate){

	Keyframe* key = nullptr;
	if(m_keyframes.empty()){
		key = new Keyframe(0, duration, cutoutIDIn,colortableIDIn,anchorxIn,anchoryIn,scalexIn,scaleyIn,offsetxIn,offsetyIn,rotationIn,mirrorIn, unknown,this );
	}else{
		if(!seperate && m_keyframes.last()->isSame(cutoutIDIn,colortableIDIn,anchorxIn, anchoryIn, scalexIn, scaleyIn, offsetxIn, offsetyIn, rotationIn, mirrorIn, unknown)){
			m_keyframes.last()->setDuration(m_keyframes.last()->getDuration()+duration);
			return;
		}else
			key = new Keyframe(m_keyframes.last(), m_keyframes.last()->getEnd(), duration, cutoutIDIn,colortableIDIn,anchorxIn,anchoryIn,scalexIn,scaleyIn,offsetxIn,offsetyIn,rotationIn,mirrorIn, unknown,this );
	}

	push_backKeyframe(key);
}

void Layer::push_backKeyframe(int start, int duration, unsigned int cutoutIDIn, unsigned char colortableIDIn,
		short anchorxIn, short anchoryIn,
		unsigned short scalexIn, unsigned short scaleyIn,
		short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn, unsigned char unknown, bool seperate){

	Keyframe* key = nullptr;
	if(m_keyframes.empty()){
		key = new Keyframe(start, duration, cutoutIDIn,colortableIDIn,anchorxIn,anchoryIn,scalexIn,scaleyIn,offsetxIn,offsetyIn,rotationIn,mirrorIn, unknown,this );
	}else{
		if(!seperate && m_keyframes.last()->getEnd() == start
				&& m_keyframes.last()->isSame(cutoutIDIn,colortableIDIn,anchorxIn, anchoryIn, scalexIn, scaleyIn, offsetxIn, offsetyIn, rotationIn, mirrorIn, unknown)){
			m_keyframes.last()->setDuration(m_keyframes.last()->getDuration()+duration);
			return;
		}else
			key = new Keyframe(m_keyframes.last(), start, duration, cutoutIDIn,colortableIDIn,anchorxIn,anchoryIn,scalexIn,scaleyIn,offsetxIn,offsetyIn,rotationIn,mirrorIn, unknown,this );
	}

	push_backKeyframe(key);
}

const QString& Layer::getName() const{
	return m_name;
}

int Layer::getDuration() const
{
	if(m_keyframes.empty())
		return 0;
	else
		return m_keyframes.last()->getEnd();
}


int Layer::getNumberOfKeyframes() const{
	return m_keyframes.size();
}

const QList<Keyframe*>& Layer::getKeyframes() const{
	return m_keyframes;
}

void Layer::setName( const QString& name){
	if(!name.size() || name == m_name) return;
	m_name = name;
	emit onNameChanged();
}


// QAbstractListModel
QVariant Layer::data(const QModelIndex & index, int role) const{
	   if (!index.isValid())
	        return QVariant();

	    if (index.row() >= getNumberOfKeyframes())
	        return QVariant();

	    if (role == Qt::DisplayRole){
	    	return QVariant::fromValue(m_keyframes.at(index.row()));
	    }else
	        return QVariant();
}

int Layer::rowCount(const QModelIndex & parent) const{
	return getNumberOfKeyframes();
}

Layer::~Layer()
{
	for(Keyframe* key: m_keyframes)
		delete key;
}

////// SPRITEANIMATION //////

SpriteAnimation::SpriteAnimation(QObject *parent): QAbstractListModel(parent){

}

SpriteAnimation::SpriteAnimation(unsigned int IDin, const QString& nameIn, QObject *parent): QAbstractListModel(parent),
		m_ID(IDin), m_name(nameIn)
{}

SpriteAnimation::SpriteAnimation(const SpriteAnimation& ani): QAbstractListModel(ani.parent()),
		m_ID(ani.getID()),m_name(ani.getName()), m_Layers(ani.m_Layers){}

SpriteAnimation::~SpriteAnimation(){
	for(Layer* lay: m_Layers)
		delete lay;
}

void SpriteAnimation::operator =(const SpriteAnimation& ani){
	setID(ani.getID());
	setName(ani.getName());

	m_Layers = ani.m_Layers;
	emit onNumberOfLayersChanged();
}

void SpriteAnimation::push_backLayer(const QString& name){
	if(name.size() <= 0) return;
	m_Layers.push_back(new Layer(name, this));
	emit onNumberOfLayersChanged();
}

void SpriteAnimation::push_backLayer(Layer* layer){
	if(!layer) return;
	m_Layers.push_back(layer);
	emit onNumberOfLayersChanged();
}

//getters
unsigned int SpriteAnimation::getID() const{
	return m_ID;
}
const QString& SpriteAnimation::getName() const{
	return m_name;
}
int SpriteAnimation::getNumberOfLayers() const{
	return m_Layers.size();
}


QList<Layer*>& SpriteAnimation::getLayers(){
	return m_Layers;
}

const QList<Layer*>& SpriteAnimation::getLayers() const{
	return m_Layers;
}

unsigned int SpriteAnimation::getTotalFrames() const{
	unsigned int lenght = 0;
	for(const Layer* const layer: m_Layers)
		if(lenght< layer->getDuration())
			lenght = layer->getDuration();
	return lenght;
}

//setters
void SpriteAnimation::setID(unsigned int idIn){
	if(idIn == m_ID) return;
	m_ID = idIn;
	emit onIDChanged();
}

void SpriteAnimation::setName(const QString& nameIn){
	if(nameIn == m_name) return;
	m_name = nameIn;
	emit onNameChanged();
}

// QAbstractListModel
QVariant SpriteAnimation::data(const QModelIndex & index, int role) const{
	   if (!index.isValid())
	        return QVariant(QVariant::Invalid);

	    if (index.row() >= getNumberOfLayers())
	        return QVariant(QVariant::Invalid);

	    if (role == Qt::DisplayRole){
	    	return QVariant::fromValue(m_Layers.at(index.row()));
	    }else
	        return QVariant();
}
int SpriteAnimation::rowCount(const QModelIndex & parent) const{
	return getNumberOfLayers();
}

////// SPRITE DATA //////

SpriteData::SpriteData(QObject *parent): QAbstractListModel(parent) {
	// TODO Auto-generated constructor stub
	//m_aniamtions.push_back(SpriteAnimation(1,"AniTest"));
	//m_aniamtions.push_back(SpriteAnimation(10,"AniWalk"));
	//m_aniamtions.push_back(SpriteAnimation(12,"HalloWorld"));
}

SpriteData::~SpriteData() {
	close();
}

// functions
bool SpriteData::open(const QString& file){
	//TODO
	return false;
}
bool SpriteData::save(const QString& file){
	//TODO
	return false;
}

inline int findCutout(const QList<Cutout*> cutouts,const PG::FILE::shfileCutout& currCutout){
	int count = 0;
	for(const Cutout* cut: cutouts){
		if(cut->isSame(currCutout.x,currCutout.y, currCutout.width,currCutout.height, currCutout.sheet)){
			return count;
		}
		count++;
	}

	return -1;
}

bool SpriteData::importSH(const QString& file){
	if(file.size() == 0) return false;
	close();

	PG::FILE::SH sh;

	if(sh.open(file.toStdString())){
		qDebug() << "Couldn't read sh file!";
		return false;
	}

	unsigned int aniCount = 0;
	m_aniamtions.reserve(sh.getAnimations().size());
	for(const PG::FILE::shfileAnimation& ani: sh.getAnimations()){

		m_aniamtions.push_back(new SpriteAnimation(ani.id, "unknown"+QString::number(aniCount), this));

		const PG::FILE::shfileKeyframe* currKey = &sh.getKeyframes()[ani.start_keyframe];
		int startOffset = 0;
		int keyCount = 0;
		while(currKey->type != 2){

			const PG::FILE::shfileLayers& currlayer = sh.getLayers()[currKey->bundel_index];
			int layerCount = 0;

			for(unsigned int i = currlayer.start_cutout; i < currlayer.start_cutout+currlayer.number_of_cutouts; i++){
				const PG::FILE::shfileCutout& currCutout =  sh.getCutouts()[i];
				int cutoutID = -1;
				//cutout
				if(currCutout.external_sheet){
					cutoutID = m_cutouts.size();
					m_cutouts.push_back(new Cutout(currCutout.external_sheet,currCutout.width,currCutout.height, this));
				}else{
					cutoutID = findCutout(m_cutouts, currCutout);
					if(cutoutID < 0){
						Cutout* cutout = new Cutout(this);
						const PG::UTIL::IDImage& pgimg = sh.getSprtieSheets()[currCutout.sheet];
						PG::UTIL::IDImage window;
						PG::UTIL::uvec2 pos(currCutout.x, currCutout.y);
						PG::UTIL::uvec2 size(
								((pos.x+currCutout.width > pgimg.getWidth())? pgimg.getWidth()-pos.x : currCutout.width),
								((pos.y+currCutout.height > pgimg.getHeight())? pgimg.getHeight()-pos.y : currCutout.height));

						pgimg.getWindow(pos,size,window);
						cutout->setCutout(window);
						cutout->setPosition(currCutout.x,currCutout.y);
						cutout->setSheetID(currCutout.sheet);
						cutoutID = m_cutouts.size();
						m_cutouts.push_back(cutout);
					}
				}

				Layer* layer = nullptr;
				if(layerCount < m_aniamtions.back()->getLayers().size() ){
					layer = m_aniamtions.back()->getLayers()[layerCount];
				}else{
					m_aniamtions.back()->push_backLayer("layer"+QString::number(layerCount));
					layer = m_aniamtions.back()->getLayers().back();
				}

				/*
				if(aniCount == 1 && layerCount == 0 && !layer->getKeyframes().empty()){
					Keyframe* kT = layer->getKeyframes().last();
					PG_INFO_STREAM(" __________________ ");
					PG_INFO_STREAM(kT->getCutoutID() << " == "<<cutoutID);
					PG_INFO_STREAM(kT->getAnchorX() << " == "<<currCutout.anchorx);
					PG_INFO_STREAM(kT->getAnchorY() << " == "<<currCutout.anchory);
					PG_INFO_STREAM(kT->getScaleX() << " == "<<currCutout.scalex);
					PG_INFO_STREAM(kT->getScaleY() << " == "<<currCutout.scaley);
					PG_INFO_STREAM(kT->getOffsetX() << " == "<<currCutout.offsetx);
					PG_INFO_STREAM(kT->getOffsetY() << " == "<<currCutout.offsety);
					PG_INFO_STREAM(kT->getRotation() << " == "<<currCutout.rotation);
				}
				*/

				layer->push_backKeyframe(startOffset, currKey->duration, cutoutID,
						currCutout.colortable,
						currCutout.anchorx,currCutout.anchory,
						currCutout.scalex, currCutout.scaley,
						currCutout.offsetx, currCutout.offsety,
						currCutout.rotation, currCutout.mirror, currCutout.unkown0);

				layerCount++;
			}
			keyCount++;
			startOffset += currKey->duration;
			currKey++;
		}
		aniCount++;
	}

	//load colortable
	m_colortable.reserve(sh.getColortables().size());
	m_colortableGL = sh.getColortables();
	for(const PG::UTIL::rgba& color: sh.getColortables()){
		m_colortable.push_back(QColor(color.r,color.g,color.b,color.a));
	}
	if(m_aniamtions.empty()){
		m_currentAnimation = -1;
		emit onNumberOfAnimationsChanged();
		emit onCurrentAnimationChanged();
		emit onAnimationChanged(nullptr);
	}else{
		m_currentAnimation = 0;
		emit onNumberOfAnimationsChanged();
		emit onCurrentAnimationChanged();
		emit onAnimationChanged(m_aniamtions[m_currentAnimation]);
	}

	m_lastFile = file;
	emit onLastFileNameChanged();
	return true;
}
bool SpriteData::exportSH(const QString& file){
	//TODO
	return false;
}

void SpriteData::close(){
	for(Cutout* cut: m_cutouts)
		delete cut;
	m_cutouts.clear();
	m_colortable.clear();
	m_colortableGL.clear();
	for(SpriteAnimation* ani: m_aniamtions)
		delete ani;
	m_aniamtions.clear();

	emit onNumberOfAnimationsChanged();
	emit onCurrentAnimationChanged();
	emit onAnimationChanged(nullptr);
}

// getters
bool SpriteData::isOpen() const{
	//TODO
	return false;
}

int SpriteData::getNumberOfAnimations() const{
	return m_aniamtions.size();
}

int SpriteData::getNumberOfCutouts() const{
	return m_cutouts.size();
}

int SpriteData::getNumberOfColortables() const{
	return m_colortable.size()/16;
}

int SpriteData::getCurrentAnimationIndex() const{
	return m_currentAnimation;
}

QString SpriteData::getLastFileName() const{
	return m_lastFile;
}

const SpriteAnimation* SpriteData::getCurrentAnimation() const{
	if(m_currentAnimation < 0 || m_aniamtions.empty()) return nullptr;
	return m_aniamtions[m_currentAnimation];
}

const QList<Cutout*>& SpriteData::getCutouts() const{
	return m_cutouts;
}

const QList<QColor>& SpriteData::getColortable() const{
	return m_colortable;
}

const PG::FILE::ColorTable& SpriteData::getColortableGL() const{
	return m_colortableGL;
}

QImage SpriteData::getSprite(unsigned int CutoutID, unsigned int ColortableID) const{
	if(CutoutID > m_cutouts.size() || ColortableID > getNumberOfColortables()){
		qInfo() <<"Invalid CutoutID or ColortableID: "<<CutoutID<<", "<<ColortableID;
		return QImage();
	}

	Cutout* cutout = m_cutouts[CutoutID];
	if(cutout->isExternalSheet())
		return QImage("resources/external.png");


	QImage sprite(cutout->getWidth(), cutout->getHeight(), QImage::Format_RGB888);
	//uchar * data = new uchar[cutout->getWidth()*cutout->getHeight()*3];
	unsigned int i = 0;
	for(const unsigned char id: cutout->getCutout()){
		const QColor& color = m_colortable[ ColortableID*16 + int(id)];
		sprite.bits()[i] = color.red();
		sprite.bits()[i+1] = color.green();
		sprite.bits()[i+2] = color.blue();
		i += 3;
	}

	return sprite;
}

//setters
void SpriteData::setCurrentAnimationByIndex(int index){
	if(index < 0) index = 0;
	else if(index >= m_aniamtions.size()) index = m_aniamtions.size()-1;
	if(m_currentAnimation == index) return;
	m_currentAnimation = index;
	emit onCurrentAnimationChanged();
	emit onAnimationChanged(m_aniamtions[m_currentAnimation]);
}

// QAbstractListModel
QVariant SpriteData::data(const QModelIndex & index, int role) const{
	   if (!index.isValid())
	        return QVariant();

	    if (index.row() >= getNumberOfAnimations())
	        return QVariant();

	    if (role == Qt::DisplayRole){
	    	const SpriteAnimation* ani = m_aniamtions.at(index.row());
	        return QString::number(index.row())+": ID: "+QString::number(ani->getID())+" Name: "+ ani->getName() +" Duration: "+QString::number(ani->getTotalFrames());
	    }else
	        return QVariant();
}
int SpriteData::rowCount(const QModelIndex & parent) const{
	return getNumberOfAnimations();
}

