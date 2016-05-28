/*
 * SpriteData.cpp
 *
 *  Created on: 28.05.2016
 *      Author: ProgSys
 */

#include <files/SpriteData.h>
#include <QException>

Cutout::Cutout(QObject *parent): QObject(parent){

}
Cutout::Cutout(unsigned char externalSheetIDIn, unsigned short sheetIDIn,
		unsigned short xIn, unsigned short yIn, unsigned short widthIn, unsigned short heightIn
		,QObject *parent): QObject(parent),
				m_externalSheetID(externalSheetIDIn),m_sheetID(sheetIDIn),
				m_pos(QVector2D(xIn,yIn)),
				m_width(widthIn),m_height(heightIn)
{}

Cutout::Cutout(unsigned char externalSheetIDIn, unsigned short sheetIDIn,
		const QVector2D& posIn, unsigned short widthIn, unsigned short heightIn
		,QObject *parent): QObject(parent),
				m_externalSheetID(externalSheetIDIn),m_sheetID(sheetIDIn),
				m_pos(posIn),
				m_width(widthIn),m_height(heightIn)
{}

Cutout::Cutout(const Cutout& cutout): QObject(cutout.parent()),
		m_externalSheetID(cutout.getEexternalSheetID()),
		m_sheetID(cutout.getSheetID()),
		m_pos(cutout.getPos()),
		m_width(cutout.getWidth()),
		m_height(cutout.getHeight())
{}

Cutout::~Cutout(){

}

//getters
bool Cutout::isExternalSheet() const{
	return m_externalSheetID;
}
unsigned char Cutout::getEexternalSheetID() const{
	return m_externalSheetID;
}
unsigned short Cutout::getSheetID() const{
	return m_sheetID;
}

float Cutout::getX() const{
	return m_pos.x();
}
float Cutout::getY() const{
	return m_pos.y();
}

const QVector2D& Cutout::getPos() const{
	return m_pos;
}

unsigned short Cutout::getWidth() const{
	return m_width;
}
unsigned short Cutout::getHeight() const{
	return m_height;
}

//setters
void Cutout::setEexternalSheetID(unsigned char externalSheetIDIn){
	if(m_externalSheetID == externalSheetIDIn) return;
	m_externalSheetID = externalSheetIDIn;
	emit onEexternalSheetIDChanged();
}
void Cutout::setSheetID(unsigned short sheetIDIn){
	if(m_sheetID == sheetIDIn) return;
	m_sheetID = sheetIDIn;
	emit onSheetIDChanged();
}

void Cutout::setX(float xIn){
	if(m_pos.x() == xIn) return;
	m_pos.setX(xIn);
	emit onXChanged();
	emit onPosChanged();
}
void Cutout::setY(float yIn){
	if(m_pos.y() == yIn) return;
	m_pos.setY(yIn);
	emit onYChanged();
	emit onPosChanged();
}

void Cutout::setPos(const QVector2D& posIn){
	if(m_pos.x() != posIn.x()){
		m_pos.setX(posIn.x());
		emit onXChanged();
		emit onPosChanged();
	}
	if(m_pos.y() != posIn.y()){
		m_pos.setY(posIn.y());
		emit onYChanged();
		emit onPosChanged();
	}
}

void Cutout::setWidth(unsigned short widthIn){
	if(m_width == widthIn) return;
	m_width = widthIn;
	emit onWidthChanged();
}
void Cutout::setHeight(unsigned short heightIn){
	if(m_height == heightIn) return;
	m_height = heightIn;
	emit onHeightChanged();
}

///// LAYER /////

Layer::Layer(QObject *parent):QObject(parent){

}
Layer::Layer(unsigned int cutoutIDIn, unsigned char colortableIDIn,
		short anchorxIn, short anchoryIn,
		unsigned short scalexIn, unsigned short scaleyIn,
		short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn, QObject *parent):
			QObject(parent),
			m_cutoutID(cutoutIDIn), m_colortableID(colortableIDIn),
			m_anchorx(anchorxIn),m_anchory(anchoryIn) ,
			m_scalex(scalexIn) ,m_scaley(scaleyIn),
			m_offsetx(offsetxIn), m_offsety(offsetyIn),
			m_rotation(rotationIn), m_mirror(mirrorIn){

}

Layer::Layer(const Layer& layer): QObject(layer.parent()),
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
     m_layers = keyframe.m_layers;
}

void Keyframe::pushLayer(const Layer& layer){
	m_layers.push_back(layer);
	emit onNumberOfLayersChanged();
}
void Keyframe::pushLayer(unsigned int cutoutIDIn, unsigned char colortableIDIn,
		short anchorxIn, short anchoryIn,
		unsigned short scalexIn, unsigned short scaleyIn,
		short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn){
	pushLayer(Layer(cutoutIDIn,colortableIDIn,anchorxIn,anchoryIn,scalexIn,scaleyIn,offsetyIn,offsetxIn,rotationIn,mirrorIn,this ));
}


int Keyframe::getDuration() const
{
    return m_duration;
}

int Keyframe::getNumberOfLayers() const{
	return m_layers.size();
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

Keyframe::~Keyframe()
{

}

////// SPRITEANIMATION //////

SpriteAnimation::SpriteAnimation(QObject *parent): QObject(parent){

}

SpriteAnimation::SpriteAnimation(unsigned int IDin, const QString& nameIn, QObject *parent): QObject(parent),
		m_ID(IDin), m_name(nameIn)
{}

SpriteAnimation::SpriteAnimation(const SpriteAnimation& ani): QObject(ani.parent()),
		m_ID(ani.getID()),m_name(ani.getName()), m_keyframes(ani.m_keyframes){}

SpriteAnimation::~SpriteAnimation(){

}

void SpriteAnimation::operator =(const SpriteAnimation& ani){
	setID(ani.getID());
	setName(ani.getName());

	m_keyframes = ani.m_keyframes;
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
const QList<Keyframe>& SpriteAnimation::getKeyframes() const{
	return m_keyframes;
}

unsigned int SpriteAnimation::getTotalFrames() const{
	unsigned int lenght = 0;
	for(const Keyframe& key: m_keyframes)
		lenght += key.getDuration();
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


////// SPRITE DATA //////

SpriteData::SpriteData(QObject *parent): QAbstractListModel(parent) {
	// TODO Auto-generated constructor stub
	m_aniamtions.push_back(SpriteAnimation(1,"AniTest"));
	m_aniamtions.push_back(SpriteAnimation(10,"AniWalk"));
	m_aniamtions.push_back(SpriteAnimation(12,"HalloWorld"));
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
	//TODO
	return false;
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

int SpriteData::getCurrentAnimationIndex() const{
	return m_currentAnimation;
}

const SpriteAnimation& SpriteData::getCurrentAnimation() const{
	if(m_currentAnimation < 0 || m_aniamtions.empty()) throw "There are no animations!";
	return m_aniamtions[m_currentAnimation];
}

//setters
void SpriteData::setCurrentAnimationByIndex(int index){
	if(index < 0) index = 0;
	else if(index >= m_aniamtions.size()) index = m_aniamtions.size()-1;
	if(m_currentAnimation == index) return;
	m_currentAnimation = index;
	emit onCurrentAnimationChanged();
	emit onAnimationChanged(&m_aniamtions[m_currentAnimation]);
}

// QAbstractListModel
QVariant SpriteData::data(const QModelIndex & index, int role) const{
	   if (!index.isValid())
	        return QVariant();

	    if (index.row() >= getNumberOfAnimations())
	        return QVariant();

	    if (role == Qt::DisplayRole){
	    	const SpriteAnimation& ani = m_aniamtions.at(index.row());
	        return QString::number(index.row())+": ID: "+QString::number(ani.getID())+" Name: "+ ani.getName() +" Keyframes: "+QString::number(ani.getNumberOfKeyframes());
	    }else
	        return QVariant();
}
int SpriteData::rowCount(const QModelIndex & parent) const{
	return getNumberOfAnimations();
}

