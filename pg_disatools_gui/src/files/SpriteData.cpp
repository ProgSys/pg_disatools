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

///// LAYER /////

Layer::Layer(QObject *parent):QObject(parent), QQuickImageProvider(QQmlImageProviderBase::Image){

}
Layer::Layer(unsigned int cutoutIDIn, unsigned char colortableIDIn,
		short anchorxIn, short anchoryIn,
		unsigned short scalexIn, unsigned short scaleyIn,
		short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn, QObject *parent):
			QObject(parent),QQuickImageProvider(QQmlImageProviderBase::Image),
			m_cutoutID(cutoutIDIn), m_colortableID(colortableIDIn),
			m_anchorx(anchorxIn),m_anchory(anchoryIn) ,
			m_scalex(scalexIn) ,m_scaley(scaleyIn),
			m_offsetx(offsetxIn), m_offsety(offsetyIn),
			m_rotation(rotationIn), m_mirror(mirrorIn){

}

Layer::Layer(const Layer& layer): QObject(layer.parent()), QQuickImageProvider(QQmlImageProviderBase::Image),
		m_cutoutID(layer.getCutoutID()), m_colortableID(layer.getColortableID()),
		m_anchorx(layer.getAnchorX()),m_anchory(layer.getAnchorY()) ,
		m_scalex(layer.getScaleX()) ,m_scaley(layer.getScaleY()),
		m_offsetx(layer.getOffsetX()), m_offsety(layer.getOffsetY()),
		m_rotation(layer.getRotation()), m_mirror(layer.getMirror()){

}

void Layer::operator =(const Layer& layer){
	setCutoutID(layer.getCutoutID());
	setColortableID(layer.getColortableID());
	setAnchorX(layer.getAnchorX());
	setAnchorX(layer.getAnchorY());

	setScaleX(layer.getScaleX());
	setScaleY(layer.getScaleY());

	setOffsetX(layer.getOffsetX());
	setOffsetY(layer.getOffsetY());

	setRotation(layer.getRotation());

	setMirror(layer.getMirror());
}

//getters
unsigned int Layer::getCutoutID() const{
	return m_cutoutID;
}
unsigned char Layer::getColortableID() const{
	return m_colortableID;
}

short Layer::getAnchorX() const{
	return m_anchorx;
}
short Layer::getAnchorY() const{
	return m_anchory;
}

unsigned short Layer::getScaleX() const{
	return m_scalex;
}
unsigned short Layer::getScaleY() const{
	return m_scaley;
}

short Layer::getOffsetX() const{
	return m_offsetx;
}
short Layer::getOffsetY() const{
	return m_offsety;
}

short Layer::getRotation() const{
	return m_rotation;
}
unsigned char Layer::getMirror() const{
	return m_mirror;
}

QImage Layer::getImage() const{
	return QImage("resources/test.jpg");
}

QImage Layer::requestImage(const QString &id, QSize *size, const QSize &requestedSize){
	return getImage();
}

//setters
void Layer::setCutoutID(unsigned int cutoutIDIn){
	if(cutoutIDIn == m_cutoutID) return;
	m_cutoutID = cutoutIDIn;
	emit onCutoutIDChanged();
}
void Layer::setColortableID(unsigned char colortableIDIn){
	if(colortableIDIn == m_colortableID) return;
	m_colortableID = colortableIDIn;
	emit onColortableIDChanged();
}

void Layer::setAnchorX(short anchorxIn){
	if(anchorxIn == m_anchorx) return;
	m_anchorx = anchorxIn;
	emit onAnchorXChanged();
}
void Layer::setAnchorY(short anchoryIn){
	if(anchoryIn == m_anchory) return;
	m_anchory = anchoryIn;
	emit onAnchorYChanged();
}

void Layer::setScaleX(unsigned short scalexIn){
	if(scalexIn == m_scalex) return;
	m_scalex = scalexIn;
	emit onScaleXChanged();
}
void Layer::setScaleY(unsigned short scaleyIn){
	if(scaleyIn == m_scaley) return;
	m_scaley = scaleyIn;
	emit onScaleYChanged();
}

void Layer::setOffsetX(short offsetxIn){
	if(offsetxIn == m_offsetx) return;
	m_offsetx = offsetxIn;
	emit onOffsetXChanged();
}
void Layer::setOffsetY(short offsetyIn){
	if(offsetyIn == m_offsety) return;
	m_offsety = offsetyIn;
	emit onOffsetYChanged();
}

void Layer::setRotation(short rotationIn){
	if(rotationIn == m_rotation) return;
	m_rotation = rotationIn;
	emit onRotationChanged();
}
void Layer::setMirror(unsigned char mirrorIn){
	if(mirrorIn == m_mirror) return;
	m_mirror = mirrorIn;
	emit onMirrorChanged();
}


Layer::~Layer(){

}

///// KEYFRAME /////
Keyframe::Keyframe(QObject *parent)
: QAbstractListModel(parent)
{

}

Keyframe::Keyframe(int duration, QObject *parent)
    : QAbstractListModel(parent)
{
    if(duration <= 0)
        m_duration = 1;
    else
        m_duration = duration;
}

Keyframe::Keyframe(const Keyframe &keyframe)
 : QAbstractListModel(nullptr)
{
    m_duration = keyframe.getDuration();
}

void Keyframe::operator =(const Keyframe &keyframe)
{
     m_duration = keyframe.getDuration();
     m_layers = keyframe.m_layers;
}

void Keyframe::push_backLayer(Layer* layer){
	if(!layer) return;
	m_layers.push_back(layer);
	emit onNumberOfLayersChanged();
}
void Keyframe::push_backLayer(unsigned int cutoutIDIn, unsigned char colortableIDIn,
		short anchorxIn, short anchoryIn,
		unsigned short scalexIn, unsigned short scaleyIn,
		short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn){
	push_backLayer(new Layer(cutoutIDIn,colortableIDIn,anchorxIn,anchoryIn,scalexIn,scaleyIn,offsetxIn,offsetyIn,rotationIn,mirrorIn,this ));
}


int Keyframe::getDuration() const
{
    return m_duration;
}

int Keyframe::getNumberOfLayers() const{
	return m_layers.size();
}

const QList<Layer*>& Keyframe::getLayers() const{
	return m_layers;
}

void Keyframe::setDuration(int duration)
{
    if(duration <= 0)
        duration = 1;
    if(duration != m_duration){
         m_duration = duration;
         emit onDurationChanged();
    }
}

// QAbstractListModel
QVariant Keyframe::data(const QModelIndex & index, int role) const{
	   if (!index.isValid())
	        return QVariant();

	    if (index.row() >= getNumberOfLayers())
	        return QVariant();

	    if (role == Qt::DisplayRole){
	    	return QVariant::fromValue(m_layers.at(index.row()));
	    }else
	        return QVariant();
}

int Keyframe::rowCount(const QModelIndex & parent) const{
	return getNumberOfLayers();
}

Keyframe::~Keyframe()
{

}

////// SPRITEANIMATION //////

SpriteAnimation::SpriteAnimation(QObject *parent): QAbstractListModel(parent){

}

SpriteAnimation::SpriteAnimation(unsigned int IDin, const QString& nameIn, QObject *parent): QAbstractListModel(parent),
		m_ID(IDin), m_name(nameIn)
{}

SpriteAnimation::SpriteAnimation(const SpriteAnimation& ani): QAbstractListModel(ani.parent()),
		m_ID(ani.getID()),m_name(ani.getName()), m_keyframes(ani.m_keyframes){}

SpriteAnimation::~SpriteAnimation(){

}

void SpriteAnimation::operator =(const SpriteAnimation& ani){
	setID(ani.getID());
	setName(ani.getName());

	m_keyframes = ani.m_keyframes;
	emit onNumberOfKeyframesChanged();
}

void SpriteAnimation::push_backKeyframe(int duration){
	if(duration <= 0) return;
	m_keyframes.push_back(new Keyframe(duration, this));
	emit onNumberOfKeyframesChanged();
}

void SpriteAnimation::push_backKeyframe(Keyframe* key){
	if(!key) return;
	m_keyframes.push_back(key);
	emit onNumberOfKeyframesChanged();
}

//getters
unsigned int SpriteAnimation::getID() const{
	return m_ID;
}
const QString& SpriteAnimation::getName() const{
	return m_name;
}
int SpriteAnimation::getNumberOfKeyframes() const{
	return m_keyframes.size();
}


const QList<Keyframe*>& SpriteAnimation::getKeyframes() const{
	return m_keyframes;
}

unsigned int SpriteAnimation::getTotalFrames() const{
	unsigned int lenght = 0;
	for(const Keyframe* const key: m_keyframes)
		lenght += key->getDuration();
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

	    if (index.row() >= getNumberOfKeyframes())
	        return QVariant(QVariant::Invalid);

	    if (role == Qt::DisplayRole){
	    	return QVariant::fromValue(m_keyframes.at(index.row()));
	    }else
	        return QVariant();
}
int SpriteAnimation::rowCount(const QModelIndex & parent) const{
	return getNumberOfKeyframes();
}

////// SPRITE DATA //////

SpriteData::SpriteData(QObject *parent): QAbstractListModel(parent) {
	// TODO Auto-generated constructor stub
	//m_aniamtions.push_back(SpriteAnimation(1,"AniTest"));
	//m_aniamtions.push_back(SpriteAnimation(10,"AniWalk"));
	//m_aniamtions.push_back(SpriteAnimation(12,"HalloWorld"));
}

SpriteData::~SpriteData() {
	// TODO Auto-generated destructor stub
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

bool SpriteData::importSH(const QString& file){
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
		while(currKey->type != 2){
			Keyframe* key = new Keyframe(currKey->duration,m_aniamtions.back());
			m_aniamtions.back()->push_backKeyframe(key);

			const PG::FILE::shfileLayers& currlayer = sh.getLayers()[currKey->bundel_index];
			for(unsigned int i = currlayer.start_cutout; i < currlayer.start_cutout+currlayer.number_of_cutouts; i++){
				const PG::FILE::shfileCutout& currCutout =  sh.getCutouts()[i];

				if(currCutout.external_sheet){
					m_cutouts.push_back(new Cutout(currCutout.external_sheet,currCutout.width,currCutout.height, this));
				}else{
					//TODO find a way to reduce the number of cutouts, a way to find the same cutouts
					Cutout* cutout = new Cutout(this);
					const PG::UTIL::IDImage& pgimg = sh.getSprtieSheets()[currCutout.sheet];
					PG::UTIL::IDImage window;
					PG::UTIL::uvec2 pos(currCutout.x, currCutout.y);
					PG::UTIL::uvec2 size(
							((pos.x+currCutout.width > pgimg.getWidth())? pgimg.getWidth()-pos.x : currCutout.width),
							((pos.y+currCutout.height > pgimg.getHeight())? pgimg.getHeight()-pos.y : currCutout.height));

					pgimg.getWindow(pos,size,window);
					cutout->setCutout(window);

					m_cutouts.push_back(cutout);
				}

				key->push_backLayer(m_cutouts.size()-1,currCutout.colortable,
						currCutout.anchorx,currCutout.anchory,
						currCutout.scalex, currCutout.scaley,
						currCutout.offsetx, currCutout.offsety,
						currCutout.rotation, currCutout.mirror);

			}

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
	return true;
}
bool SpriteData::exportSH(const QString& file){
	//TODO
	return false;
}

void SpriteData::close(){
	m_cutouts.clear();
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
	        return QString::number(index.row())+": ID: "+QString::number(ani->getID())+" Name: "+ ani->getName() +" Keyframes: "+QString::number(ani->getNumberOfKeyframes());
	    }else
	        return QVariant();
}
int SpriteData::rowCount(const QModelIndex & parent) const{
	return getNumberOfAnimations();
}

