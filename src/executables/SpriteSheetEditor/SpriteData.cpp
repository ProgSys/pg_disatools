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
#include <SpriteData.h>
#include <QException>
#include <Files/PG_SH.h>
#include <Files/PG_ImageFiles.h>
#include <QDebug>
#include <QFileInfo>
#include <Util/PG_VectorUtil.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <CreateEmptySpriteSheet.h>
#include <Util/PG_Exception.h>
#include <QInputDialog>
#include <QQueue>
#include <QVector>
#include <QMutableListIterator>
#include <ImageLib.h>
#include <Util/Misc/ResourcePath.h>
#include <QClipboard>

static PG::UTIL::IDImage g_defaultExternalSpriteSheet;
static std::map<int, PG::UTIL::IDImage> g_externalSpriteSheet;
static std::map<int, QVector<QColorTable>> g_externalColorTable;

PG::UTIL::IDImage& getPredifinedExternalSpriteSheet(int externalId) {
	auto findIt = g_externalSpriteSheet.find(externalId);
	if (findIt == g_externalSpriteSheet.end())
		return g_defaultExternalSpriteSheet;
	else return findIt->second;
}

const QVector<QColorTable>& getPredifinedExternalColorTable(int externalId, const QVector<QColorTable>& defaultTable) {
	auto findIt = g_externalColorTable.find(externalId);
	if (findIt == g_externalColorTable.end())
		return defaultTable;
	else return findIt->second;
}

Cutout::Cutout(QObject* parent) : QObject(parent) {

}
Cutout::Cutout(int sheetID, const PG::UTIL::ivec2& position, const PG::UTIL::ivec2& size, QObject* parent) :
	QObject(parent), m_sheetID(sheetID), m_position(position), m_size(size)
{}

Cutout::Cutout(int sheetID, const PG::UTIL::ivec2& position, const PG::UTIL::ivec2& size, unsigned int defaultColorTable, QObject* parent) :
	QObject(parent), m_sheetID(sheetID), m_position(position), m_size(size), m_defaultColorTable(defaultColorTable)
{}

Cutout::Cutout(const Cutout& cutout) : QObject(cutout.parent()),
m_sheetID(cutout.getSheetID()),
m_position(cutout.getPosition()),
m_size(cutout.getSize())
{}

Cutout::~Cutout() {

}

bool Cutout::isSame(int x, int y, int width, int height, unsigned int colortable) const {
	return m_position.x == x && m_position.y == y && m_size.x == width && m_size.y == height && m_defaultColorTable == colortable;
}

//getters
int Cutout::getSheetID() const {
	return m_sheetID;
}

const PG::UTIL::ivec2& Cutout::getPosition() const {
	return m_position;
}

int Cutout::getX() const {
	return m_position.x;
}

int Cutout::getY() const {
	return m_position.y;
}

const PG::UTIL::ivec2& Cutout::getSize() const {
	return m_size;
}

unsigned short Cutout::getWidth() const {
	return m_size.x;
}
unsigned short Cutout::getHeight() const {
	return m_size.y;
}


unsigned int Cutout::getDefaultColorTable() const {
	return m_defaultColorTable;
}

bool Cutout::isHidden() const {
	return m_hide;
}

//setters
void Cutout::setSheetID(int id) {
	if (m_sheetID == id) return;
	m_sheetID = id;
	emit onSheetIDChanged();
}

void Cutout::setPosition(const PG::UTIL::ivec2& pos) {
	if (m_position.x == pos.x && m_position.y == pos.y) return;
	m_position = pos;
	emit onXChanged();
	emit onYChanged();
	emit onPositionChanged();
}

void Cutout::setPosition(int x, int y) {
	if (m_position.x == x && m_position.y == y) return;
	m_position.x = x;
	m_position.y = y;
	emit onXChanged();
	emit onYChanged();
	emit onPositionChanged();
}

void Cutout::setX(int x) {
	if (x < 0) x = 0;
	if (m_position.x == x) return;
	m_position.x = x;
	emit onXChanged();
	emit onPositionChanged();
}

void Cutout::setY(int y) {
	if (y < 0) y = 0;
	if (m_position.y == y) return;
	m_position.y = y;
	emit onYChanged();
	emit onPositionChanged();
}

void Cutout::setSize(const PG::UTIL::ivec2& size) {
	setWidth(size.x);
	setHeight(size.y);
}

void Cutout::setSize(int width, int height) {
	setWidth(width);
	setHeight(height);
}

void Cutout::setWidth(int x) {
	if (m_size.x == x) return;
	m_size.x = x;
	emit onWidthChanged();
}
void Cutout::setHeight(int y) {
	if (m_size.y == y) return;
	m_size.y = y;
	emit onHeightChanged();
}

void Cutout::setDefaultColorTable(unsigned int tableID) {
	if (m_defaultColorTable == tableID) return;
	m_defaultColorTable = tableID;
	emit onDefaultColorTableChanged();
}

void Cutout::setHidden(bool hide) {
	if (m_hide == hide) return;
	m_hide = hide;
	emit onHiddenChanged();
}

///// KEYFRAME /////

Keyframe::Keyframe(QObject* parent) :QObject(parent) {

}

Keyframe::Keyframe(Keyframe* previousIn, int startIn, int durationIn, unsigned int cutoutIDIn, unsigned char colortableIDIn,
	short anchorxIn, short anchoryIn,
	unsigned short scalexIn, unsigned short scaleyIn,
	short offsetxIn, short offsetyIn, short rotationIn, unsigned char transparencyIn, unsigned char micIn, QObject* parent) :
	QObject(parent),
	m_start(startIn), m_duration(durationIn),
	m_cutoutID(cutoutIDIn), m_colortableID(colortableIDIn),
	m_anchorx(anchorxIn), m_anchory(anchoryIn),
	m_scalex(scalexIn), m_scaley(scaleyIn),
	m_offsetx(offsetxIn), m_offsety(offsetyIn),
	m_rotation(rotationIn), m_transparency(transparencyIn), m_mic(micIn),
	m_previous(previousIn)
{
	if (previousIn && m_start < previousIn->getEnd())
		m_start = previousIn->getEnd();
}

Keyframe::Keyframe(int startIn, int durationIn, unsigned int cutoutIDIn, unsigned char colortableIDIn,
	short anchorxIn, short anchoryIn,
	unsigned short scalexIn, unsigned short scaleyIn,
	short offsetxIn, short offsetyIn, short rotationIn, unsigned char transparencyIn, unsigned char micIn, QObject* parent) :
	QObject(parent),
	m_start(startIn), m_duration(durationIn),
	m_cutoutID(cutoutIDIn), m_colortableID(colortableIDIn),
	m_anchorx(anchorxIn), m_anchory(anchoryIn),
	m_scalex(scalexIn), m_scaley(scaleyIn),
	m_offsetx(offsetxIn), m_offsety(offsetyIn),
	m_rotation(rotationIn), m_transparency(transparencyIn), m_mic(micIn)
{}

Keyframe::Keyframe(const Keyframe& keyframe) : QObject(keyframe.parent()),
m_start(keyframe.getStart()), m_duration(keyframe.getDuration()),
m_cutoutID(keyframe.getCutoutID()), m_colortableID(keyframe.getColortableID()),
m_anchorx(keyframe.getAnchorX()), m_anchory(keyframe.getAnchorY()),
m_scalex(keyframe.getScaleX()), m_scaley(keyframe.getScaleY()),
m_offsetx(keyframe.getOffsetX()), m_offsety(keyframe.getOffsetY()),
m_rotation(keyframe.getRotation()), m_transparency(keyframe.getTransparency()), m_mic(keyframe.getMic()),
m_previous(const_cast<Keyframe*>(keyframe.getPrevious())), m_next(const_cast<Keyframe*>(keyframe.getNext()))
{}

void Keyframe::operator =(const Keyframe& keyframe) {
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

	setRotation(keyframe.getRotation());
	setTransparency(keyframe.getTransparency());
	setMic(keyframe.getMic());

	setPrevious(const_cast<Keyframe*> (keyframe.getPrevious()));
	setNext(const_cast<Keyframe*> (keyframe.getNext()));
}

bool Keyframe::operator ==(const Keyframe* keyframe) const {
	return keyframe && m_cutoutID == keyframe->getCutoutID() && m_colortableID == keyframe->getColortableID()
		&& m_anchorx == keyframe->getAnchorX() && m_anchory == keyframe->getAnchorY()
		&& m_scalex == keyframe->getScaleX() && m_scaley == keyframe->getScaleY()
		&& m_offsetx == keyframe->getOffsetX() && m_offsety == keyframe->getOffsetY()
		&& m_rotation == keyframe->getRotation() && m_transparency == keyframe->getTransparency()
		&& m_mic == keyframe->getMic();
}

bool Keyframe::isSame(unsigned int cutoutIDIn, unsigned char colortableIDIn,
	short anchorxIn, short anchoryIn,
	unsigned short scalexIn, unsigned short scaleyIn,
	short offsetxIn, short offsetyIn, short rotationIn, unsigned char transparencyIn, unsigned char micIn) const {
	return  m_cutoutID == cutoutIDIn && m_colortableID == colortableIDIn
		&& m_anchorx == anchorxIn && m_anchory == anchoryIn
		&& m_scalex == scalexIn && m_scaley == scaleyIn
		&& m_offsetx == offsetxIn && m_offsety == offsetyIn
		&& m_rotation == rotationIn
		&& m_transparency == transparencyIn && m_mic == micIn;
}

//getters

int Keyframe::getStart() const {
	return m_start;
}

int Keyframe::getDuration() const {
	return m_duration;
}

int Keyframe::getEnd() const {
	return m_start + m_duration;
}

unsigned int Keyframe::getCutoutID() const {
	return m_cutoutID;
}
unsigned char Keyframe::getColortableID() const {
	return m_colortableID;
}

short Keyframe::getAnchorX() const {
	return m_anchorx;
}
short Keyframe::getAnchorY() const {
	return m_anchory;
}

unsigned short Keyframe::getScaleX() const {
	return m_scalex;
}
unsigned short Keyframe::getScaleY() const {
	return m_scaley;
}

short Keyframe::getOffsetX() const {
	return m_offsetx;
}
short Keyframe::getOffsetY() const {
	return m_offsety;
}

short Keyframe::getRotation() const {
	return m_rotation;
}
unsigned char Keyframe::getTransparency() const {
	return m_transparency;
}

unsigned char Keyframe::getMic() const {
	return m_mic;
}

bool Keyframe::isMirroredHorizontally() const {
	return m_mic & 0x10;
}

bool Keyframe::isMirroredVertically() const {
	return m_mic & 0x08;
}

bool Keyframe::isAdaptive() const {
	return m_mic & 0x04;
}

Keyframe* Keyframe::getNext() {
	return m_next;
}

const Keyframe* Keyframe::getNext() const {
	return m_next;
}

bool Keyframe::hasNext() const {
	return m_next;
}

Keyframe* Keyframe::getPrevious() {
	return m_previous;
}

const Keyframe* Keyframe::getPrevious() const {
	return m_previous;
}

bool Keyframe::hasPrevious() const {
	return m_previous;
}

bool Keyframe::isSelected() const {
	return m_selected;
}

//setters
void Keyframe::setStart(int startIn, bool keepDocked) {
	if (startIn < 0) startIn = 0;
	if (startIn == m_start) return;
	int end = m_start + m_duration;

	if (keepDocked && m_previous
		&& (startIn < m_previous->getEnd() || m_previous->getEnd() == getStart())) {

		if (startIn - m_previous->getStart() < 1)
			startIn = m_previous->getStart() + 1;
		else if (startIn >= end) {
			startIn = end - 1;
		}
		m_previous->setDurationDirrect(startIn - m_previous->getStart());

	}

	if (m_previous && startIn <= m_previous->getEnd()) {
		m_start = m_previous->getEnd();
	}
	else if (m_next && startIn >= m_next->getStart()) {
		m_start = m_next->getStart() - 1;
	}
	else
		m_start = startIn;
	if (m_start < 0)
		m_start = 0;
	else if (m_start >= end)
		m_start = end - 1;

	emit onStartChanged();

	m_duration = end - m_start;
	emit onDurationChanged();


}

void Keyframe::setDuration(int durationIn, bool keepDocked) {
	if (durationIn <= 0 || durationIn == m_duration) return;

	if (keepDocked && m_next && (getStart() + durationIn > m_next->getStart() || getEnd() == m_next->getStart())) {
		const int diff = getStart() + durationIn - m_next->getStart();
		if ((m_next->getEnd() - m_next->getStart() - diff) > 1) {
			m_next->setStartDirrect(m_next->getStart() + diff);
			m_next->setDuration(m_next->getDuration() - diff);
		}
		else {
			m_next->setStartDirrect(m_next->getEnd() - 1);
			m_next->setDuration(1);
			durationIn = m_next->getEnd() - getStart();
			/*
			Keyframe* key = m_next;
			while(key){
				key->setStartDirrect(key->getStart()+diff);
				key = key->getNext();
			}
			*/
		}
	}

	if (m_next && (m_start + durationIn) >= m_next->getStart()) {
		m_duration = m_next->getStart() - m_start;
	}
	else
		m_duration = durationIn;
	emit onDurationChanged();
}

void Keyframe::moveTo(int frame) {
	if (m_previous && m_previous->getEnd() < frame)
		frame = m_previous->getEnd();
	else if (frame < 0) frame = 0;

	const int diff = frame - m_start;
	m_start = frame;
	Keyframe* key = m_next;
	while (key) {
		key->setStartDirrect(key->getStart() + diff);
		key = key->getNext();
	}
	emit onStartChanged();
}


void Keyframe::setCutoutID(unsigned int cutoutIDIn) {
	if (cutoutIDIn == m_cutoutID) return;
	m_cutoutID = cutoutIDIn;
	emit onCutoutIDChanged();
}
void Keyframe::setColortableID(unsigned char colortableIDIn) {
	if (colortableIDIn == m_colortableID) return;
	m_colortableID = colortableIDIn;
	emit onColortableIDChanged();
}

void Keyframe::setAnchorX(short anchorxIn) {
	if (anchorxIn == m_anchorx) return;
	m_anchorx = anchorxIn;
	emit onAnchorXChanged();
}
void Keyframe::setAnchorY(short anchoryIn) {
	if (anchoryIn == m_anchory) return;
	m_anchory = anchoryIn;
	emit onAnchorYChanged();
}

void Keyframe::setScaleX(unsigned short scalexIn) {
	if (scalexIn == m_scalex) return;
	m_scalex = scalexIn;
	emit onScaleXChanged();
}
void Keyframe::setScaleY(unsigned short scaleyIn) {
	if (scaleyIn == m_scaley) return;
	m_scaley = scaleyIn;
	emit onScaleYChanged();
}

void Keyframe::setOffsetX(short offsetxIn) {
	if (offsetxIn == m_offsetx) return;
	m_offsetx = offsetxIn;
	emit onOffsetXChanged();
}
void Keyframe::setOffsetY(short offsetyIn) {
	if (offsetyIn == m_offsety) return;
	m_offsety = offsetyIn;
	emit onOffsetYChanged();
}

void Keyframe::moveOffset(short offsetxIn, short offsetyIn) {
	setOffsetX(m_offsetx + offsetxIn);
	setOffsetY(m_offsety + offsetyIn);
}

void Keyframe::setRotation(short rotationIn) {
	rotationIn = rotationIn % 360;
	if (rotationIn < 0) rotationIn = 360 + rotationIn;
	if (rotationIn == m_rotation) return;
	m_rotation = rotationIn;
	emit onRotationChanged();
}
void Keyframe::setTransparency(unsigned char transparencyIn) {
	if (transparencyIn == m_transparency) return;
	m_transparency = transparencyIn;
	emit onTransparencyChanged();
}

void Keyframe::setMic(unsigned char micIn) {
	if (micIn == m_mic) return;
	m_mic = micIn;
	emit onMicChanged();
	emit horizontalMirrorChanged();
	emit verticalMirrorChanged();
	emit adaptiveChanged();
}

void Keyframe::setMirroredHorizontally(bool mirrored) {
	if (isMirroredHorizontally() == mirrored) return;
	m_mic ^= 0x10;
	emit onMicChanged();
	emit horizontalMirrorChanged();
}

void Keyframe::setMirroredVertically(bool mirrored) {
	if (isMirroredVertically() == mirrored) return;
	m_mic ^= 0x08;
	emit onMicChanged();
	emit verticalMirrorChanged();
}

void Keyframe::setAdaptive(bool addaptive) {
	if (isAdaptive() == addaptive) return;
	m_mic ^= 0x04;
	emit onMicChanged();
	emit adaptiveChanged();
}

void Keyframe::setSelected(bool select) {
	if (select == m_selected) return;
	m_selected = select;
	emit onSelectionChanged();
}

bool Keyframe::swapNext() {
	if (!hasNext()) return false;

	//buffvalues
	const unsigned int cutoutID = m_next->getCutoutID();
	const unsigned char colortableID = m_next->getColortableID();

	const short anchorx = m_next->getAnchorX();
	const short anchory = m_next->getAnchorY();
	const unsigned short scalex = m_next->getScaleX();
	const unsigned short scaley = m_next->getScaleY();
	const short offsetx = m_next->getOffsetX();
	const short offsety = m_next->getOffsetY();
	const short rotation = m_next->getRotation();

	const unsigned char trans = m_next->getTransparency();
	const unsigned char mic = m_next->getMic();

	//copy them over
	m_next->setCutoutID(m_cutoutID);
	m_next->setColortableID(m_colortableID);

	m_next->setAnchorX(m_anchorx);
	m_next->setAnchorY(m_anchory);
	m_next->setScaleX(m_scalex);
	m_next->setScaleY(m_scaley);
	m_next->setOffsetX(m_offsetx);
	m_next->setOffsetY(m_offsety);
	m_next->setRotation(m_rotation);

	m_next->setTransparency(m_transparency);
	m_next->setMic(m_mic);

	//copy buffer values
	setCutoutID(cutoutID);
	setColortableID(colortableID);

	setAnchorX(anchorx);
	setAnchorY(anchory);
	setScaleX(scalex);
	setScaleY(scaley);
	setOffsetX(offsetx);
	setOffsetY(offsety);
	setRotation(m_rotation);

	setTransparency(trans);
	setMic(mic);

	return true;
}

bool Keyframe::swapPrevious() {
	if (!hasPrevious()) return false;

	//buffvalues
	const unsigned int cutoutID = m_previous->getCutoutID();
	const unsigned char colortableID = m_previous->getColortableID();

	const short anchorx = m_previous->getAnchorX();
	const short anchory = m_previous->getAnchorY();
	const unsigned short scalex = m_previous->getScaleX();
	const unsigned short scaley = m_previous->getScaleY();
	const short offsetx = m_previous->getOffsetX();
	const short offsety = m_previous->getOffsetY();
	const short rotation = m_previous->getRotation();

	const unsigned char trans = m_previous->getTransparency();
	const unsigned char mic = m_previous->getMic();

	//copy them over
	m_previous->setCutoutID(m_cutoutID);
	m_previous->setColortableID(m_colortableID);

	m_previous->setAnchorX(m_anchorx);
	m_previous->setAnchorY(m_anchory);
	m_previous->setScaleX(m_scalex);
	m_previous->setScaleY(m_scaley);
	m_previous->setOffsetX(m_offsetx);
	m_previous->setOffsetY(m_offsety);
	m_previous->setRotation(m_rotation);

	m_previous->setTransparency(m_transparency);
	m_previous->setMic(m_mic);

	//copy buffer values
	setCutoutID(cutoutID);
	setColortableID(colortableID);

	setAnchorX(anchorx);
	setAnchorY(anchory);
	setScaleX(scalex);
	setScaleY(scaley);
	setOffsetX(offsetx);
	setOffsetY(offsety);
	setRotation(m_rotation);

	setTransparency(trans);
	setMic(mic);

	return true;
}

void Keyframe::setNext(Keyframe* nextIn) {
	m_next = nextIn;
}

void Keyframe::setPrevious(Keyframe* previousIn) {
	m_previous = previousIn;
}

void Keyframe::setDurationDirrect(int durr) {
	if (durr < 1) durr = 1;
	if (durr == m_duration) return;
	m_duration = durr;
	emit onDurationChanged();
}

void Keyframe::setStartDirrect(int frame) {
	if (frame < 0) frame = 0;
	if (m_start == frame) return;
	m_start = frame;
	emit onStartChanged();
}

Keyframe::~Keyframe() {

}

///// LAYER /////
Layer::Layer(QObject* parent)
	: QAbstractListModel(parent)
{

}

Layer::Layer(const QString& nameIn, QObject* parent)
	: QAbstractListModel(parent)
{
	if (nameIn.size())
		m_name = nameIn;
}

Layer::Layer(const QString& nameIn, bool hidden, QObject* parent)
	: QAbstractListModel(parent), m_hidden(hidden) {

	if (nameIn.size())
		m_name = nameIn;
}

Layer::Layer(const Layer& layer)
	: QAbstractListModel(nullptr)
{
	m_name = layer.getName();
	m_keyframes = layer.getKeyframes();
}

void Layer::operator =(const Layer& layer)
{
	m_name = layer.getName();
	m_keyframes = layer.getKeyframes();
}

void Layer::push_backKeyframe(Keyframe* keyframe) {
	if (!keyframe) return;
	if (m_keyframes.size()) {
		m_keyframes.last()->setNext(keyframe);
		keyframe->setPrevious(m_keyframes.last());
	}
	m_keyframes.push_back(keyframe);
	emit onNumberOfKeyframesChanged();
}

void Layer::push_backKeyframe(int duration, unsigned int cutoutIDIn, unsigned char colortableIDIn,
	short anchorxIn, short anchoryIn,
	unsigned short scalexIn, unsigned short scaleyIn,
	short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn, unsigned char unknown, bool seperate) {

	Keyframe* key = nullptr;
	if (m_keyframes.empty()) {
		key = new Keyframe(0, duration, cutoutIDIn, colortableIDIn, anchorxIn, anchoryIn, scalexIn, scaleyIn, offsetxIn, offsetyIn, rotationIn, mirrorIn, unknown, this);
	}
	else {
		if (!seperate && m_keyframes.last()->isSame(cutoutIDIn, colortableIDIn, anchorxIn, anchoryIn, scalexIn, scaleyIn, offsetxIn, offsetyIn, rotationIn, mirrorIn, unknown)) {
			m_keyframes.last()->setDuration(m_keyframes.last()->getDuration() + duration);
			return;
		}
		else
			key = new Keyframe(m_keyframes.last(), m_keyframes.last()->getEnd(), duration, cutoutIDIn, colortableIDIn, anchorxIn, anchoryIn, scalexIn, scaleyIn, offsetxIn, offsetyIn, rotationIn, mirrorIn, unknown, this);
	}

	push_backKeyframe(key);
}

void Layer::push_backKeyframe(int start, int duration, unsigned int cutoutIDIn, unsigned char colortableIDIn,
	short anchorxIn, short anchoryIn,
	unsigned short scalexIn, unsigned short scaleyIn,
	short offsetxIn, short offsetyIn, short rotationIn, unsigned char mirrorIn, unsigned char unknown, bool seperate) {

	Keyframe* key = nullptr;
	if (m_keyframes.empty()) {
		key = new Keyframe(start, duration, cutoutIDIn, colortableIDIn, anchorxIn, anchoryIn, scalexIn, scaleyIn, offsetxIn, offsetyIn, rotationIn, mirrorIn, unknown, this);
	}
	else {
		if (!seperate && m_keyframes.last()->getEnd() == start
			&& m_keyframes.last()->isSame(cutoutIDIn, colortableIDIn, anchorxIn, anchoryIn, scalexIn, scaleyIn, offsetxIn, offsetyIn, rotationIn, mirrorIn, unknown)) {
			m_keyframes.last()->setDuration(m_keyframes.last()->getDuration() + duration);
			return;
		}
		else
			key = new Keyframe(m_keyframes.last(), start, duration, cutoutIDIn, colortableIDIn, anchorxIn, anchoryIn, scalexIn, scaleyIn, offsetxIn, offsetyIn, rotationIn, mirrorIn, unknown, this);
	}

	push_backKeyframe(key);
}

bool Layer::insertKeyframe(int frame) {
	if (m_keyframes.empty()) {
		beginInsertRows(QModelIndex(), m_keyframes.size(), m_keyframes.size());
		m_keyframes.push_back(new Keyframe(frame, 10, 0, 0, 0, 0, 100, 100, 0, 0, 0, 128, 0, this));
		endInsertRows();
		emit onDurationChanged();
		emit onNumberOfKeyframesChanged();
		return true;
	}
	else if (frame < 0) {
		beginInsertRows(QModelIndex(), m_keyframes.size(), m_keyframes.size());
		Keyframe* currentBack = m_keyframes.back();
		m_keyframes.push_back(new Keyframe(currentBack, currentBack->getEnd(), 10,
			currentBack->getCutoutID(), currentBack->getColortableID(),
			currentBack->getAnchorX(), currentBack->getAnchorY(),
			currentBack->getScaleX(), currentBack->getScaleY(),
			currentBack->getOffsetX(), currentBack->getOffsetY(),
			currentBack->getRotation(), 128, 0, this));
		endInsertRows();
		emit onDurationChanged();
		emit onNumberOfKeyframesChanged();
		return true;
	}
	else if (frame >= m_keyframes.last()->getEnd()) {
		beginInsertRows(QModelIndex(), m_keyframes.size(), m_keyframes.size());
		Keyframe* lastKey = m_keyframes.last();
		m_keyframes.push_back(new Keyframe(frame, 10, 0, 0, 0, 0, 100, 100, 0, 0, 0, 128, 0, this));
		m_keyframes.last()->setPrevious(lastKey);
		lastKey->setNext(m_keyframes.last());
		endInsertRows();

		emit onDurationChanged();
		emit onNumberOfKeyframesChanged();
		return true;
	}
	else if (frame < m_keyframes.first()->getStart()) {
		beginInsertRows(QModelIndex(), 0, 0);
		Keyframe* currentFront = m_keyframes.first();
		m_keyframes.push_front(new Keyframe(frame, ((frame + 10) > currentFront->getStart()) ? currentFront->getStart() - frame : 10, 0, 0, 0, 0, 100, 100, 0, 0, 0, 128, 0, this));
		m_keyframes.first()->setNext(currentFront);
		currentFront->setPrevious(m_keyframes.first());
		endInsertRows();

		emit onDurationChanged();
		emit onNumberOfKeyframesChanged();
		return true;
	}

	Keyframe* key = m_keyframes.first();
	while (key) {
		Keyframe* next = key->getNext();
		if (!next) break;

		if (key->getEnd() != next->getStart() && frame >= key->getEnd() && frame <= next->getStart()) {
			const int indexOf = m_keyframes.indexOf(next);
			assert_Test("Index of is invalid!", indexOf == -1);

			beginInsertRows(QModelIndex(), indexOf, indexOf);
			Keyframe* newKey = new Keyframe(frame, next->getStart() - frame, 0, 0, 0, 0, 100, 100, 0, 0, 0, 128, 0, this);
			newKey->setPrevious(key);
			newKey->setNext(next);
			key->setNext(newKey);
			next->setPrevious(newKey);
			m_keyframes.insert(indexOf, newKey);
			endInsertRows();
			emit onNumberOfKeyframesChanged();
			return true;
		}
		key = next;
	}

	return false;
}


bool Layer::splitKeyframe(int frame) {
	if (frame <= 0) return false;
	for (Keyframe* key : m_keyframes) {
		if (key->getStart() < frame && frame < key->getEnd()) {
			const int indexOf = m_keyframes.indexOf(key);
			assert_Test("Index of is invalid!", indexOf == -1);

			beginInsertRows(QModelIndex(), indexOf + 1, indexOf + 1);
			const int oldKeyDuration = frame - key->getStart();
			const int newKeyDuration = key->getEnd() - frame;
			key->setDuration(oldKeyDuration);

			Keyframe* newKey = new Keyframe(frame, newKeyDuration,
				key->getCutoutID(), key->getColortableID(),
				key->getAnchorX(), key->getAnchorY(),
				key->getScaleX(), key->getScaleY(),
				key->getOffsetX(), key->getOffsetY(),
				key->getRotation(), key->getTransparency(), key->getMic());


			Keyframe* after = key->getNext();

			newKey->setPrevious(key);
			newKey->setNext(after);

			if (after) after->setPrevious(newKey);
			key->setNext(newKey);

			m_keyframes.insert(indexOf + 1, newKey);
			endInsertRows();
			emit onNumberOfKeyframesChanged();
			return true;
		}
	}
	return false;
}

bool Layer::removeKeyframe(Keyframe* keyframe) {
	if (!keyframe) return false;
	int indexof = m_keyframes.indexOf(keyframe);

	if (indexof != -1) {
		beginRemoveRows(QModelIndex(), indexof, indexof);
		Keyframe* before = keyframe->getPrevious();
		Keyframe* after = keyframe->getNext();

		if (after) after->setPrevious(before);
		if (before) before->setNext(after);

		m_keyframes.removeAt(indexof);
		endRemoveRows();

		emit onDurationChanged();
		emit onNumberOfKeyframesChanged();

		return true;
	}

	return false;

}

const QString& Layer::getName() const {
	return m_name;
}

int Layer::getDuration() const
{
	if (m_keyframes.empty())
		return 0;
	else
		return m_keyframes.last()->getEnd();
}


int Layer::getNumberOfKeyframes() const {
	return m_keyframes.size();
}

const QList<Keyframe*>& Layer::getKeyframes() const {
	return m_keyframes;
}

bool Layer::isHidden() const {
	return m_hidden;
}

void Layer::setName(const QString& name) {
	if (!name.size() || name == m_name) return;
	m_name = name;
	emit onNameChanged();
}

void Layer::setHidden(bool hidden) {
	if (m_hidden == hidden) return;
	m_hidden = hidden;
	emit hiddenChanged();
}

// QAbstractListModel
QVariant Layer::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (index.row() >= getNumberOfKeyframes())
		return QVariant();

	if (role == Qt::DisplayRole) {
		return QVariant::fromValue(m_keyframes.at(index.row()));
	}
	else
		return QVariant();
}

int Layer::rowCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return 0;
	return getNumberOfKeyframes();
}

Layer::~Layer()
{
	for (Keyframe* key : m_keyframes)
		delete key;
}


////// SPRITEANIMATION //////

SpriteAnimation::SpriteAnimation(QObject* parent) : QAbstractListModel(parent), m_Markers(new MarkersList(this)) {

}

SpriteAnimation::SpriteAnimation(unsigned int IDin, const QString& nameIn, QObject* parent) : QAbstractListModel(parent),
m_ID(IDin), m_name(nameIn), m_Markers(new MarkersList(this))
{}

SpriteAnimation::SpriteAnimation(const SpriteAnimation& ani) : QAbstractListModel(ani.parent()),
m_ID(ani.getID()), m_name(ani.getName()), m_Layers(ani.m_Layers), m_Markers(new MarkersList(this)) {}

SpriteAnimation::~SpriteAnimation() {
	for (Layer* lay : m_Layers)
		delete lay;
	delete m_Markers;
}

void SpriteAnimation::operator =(const SpriteAnimation& ani) {
	setID(ani.getID());
	setName(ani.getName());

	m_Layers = ani.m_Layers;
	emit onNumberOfLayersChanged();
}

void SpriteAnimation::push_backLayer(const QString& name) {
	if (name.size() <= 0) return;
	m_Layers.push_back(new Layer(name, this));
	emit onNumberOfLayersChanged();
	emit onLengthChanged();
}

void SpriteAnimation::push_backLayer(Layer* layer) {
	if (!layer) return;
	m_Layers.push_back(layer);
	emit onNumberOfLayersChanged();
	emit onLengthChanged();
}

//getters
unsigned int SpriteAnimation::getID() const {
	return m_ID;
}
const QString& SpriteAnimation::getName() const {
	return m_name;
}

unsigned int SpriteAnimation::getTotalFrames() const {
	unsigned int lenght = 0;
	for (const Layer* const layer : m_Layers)
		if (lenght < layer->getDuration())
			lenght = layer->getDuration();
	return lenght;
}

unsigned int SpriteAnimation::getTotalWorkspace() const {
	unsigned int lenght = getTotalFrames();
	for (const Marker* const mark : m_Markers->getList())
		if (lenght < mark->getStart() + mark->getDuration())
			lenght = mark->getStart() + mark->getDuration();
	return lenght;

}

QList<Layer*>& SpriteAnimation::getLayers() {
	return m_Layers;
}

const QList<Layer*>& SpriteAnimation::getLayers() const {
	return m_Layers;
}

int SpriteAnimation::getNumberOfLayers() const {
	return m_Layers.size();
}

int SpriteAnimation::getNumberOfMarkers() const {
	return m_Markers->size();
}

MarkersList* SpriteAnimation::getMarkers() {
	return m_Markers;
}

const MarkersList* SpriteAnimation::getMarkers() const {
	return m_Markers;
}



//setters
void SpriteAnimation::setID(unsigned int idIn) {
	if (idIn == m_ID) return;
	m_ID = idIn;
	emit onIDChanged();
}

void SpriteAnimation::setName(const QString& nameIn) {
	if (nameIn == m_name) return;
	m_name = nameIn;
	emit onNameChanged();
}

void SpriteAnimation::refresh() {
	emit onNumberOfLayersChanged();
	emit onLengthChanged();
}

bool SpriteAnimation::moveMarker(Marker* mark, int frame) {
	assert_Test("You are trying to move a marker but the animation dosn't even have any!", m_Markers->empty());
	assert_Test("The marker is not inside this animation!", !m_Markers->contains(mark));

	int newframe = m_Markers->findNextFreeframe(frame);
	if (frame < newframe && newframe > mark->getStart()) frame = mark->getStart();
	else frame = newframe;
	if (frame >= 0) {
		PG_INFO_STREAM("moving mark to frame: " << frame)
			mark->setStart(frame);
		emit onLengthChanged();
		return true;
	}
	return false;
}

bool SpriteAnimation::addMarker(int frame, int type, short a, short b) {
	if (frame < 0 || type == 0) return false;
	frame = m_Markers->findNextFreeframe(frame);
	if (frame >= 0) m_Markers->add(new Marker(frame, 1, type, a, b));
	return true;
}

bool SpriteAnimation::removeMarker(Marker* mark) {
	return m_Markers->remove(mark);
}


bool SpriteAnimation::moveUp(Layer* lay) {
	if (!lay) return false;
	const int indexOf = m_Layers.indexOf(lay);
	if (indexOf <= 0) return false;

	m_Layers[indexOf] = m_Layers[indexOf - 1];
	m_Layers[indexOf - 1] = lay;
	emit dataChanged(index(indexOf - 1), index(indexOf));
	return true;
}

bool SpriteAnimation::moveDown(Layer* lay) {
	if (!lay) return false;
	const int indexOf = m_Layers.indexOf(lay);
	if (indexOf == -1 || indexOf >= m_Layers.size() - 1) return false;

	m_Layers[indexOf] = m_Layers[indexOf + 1];
	m_Layers[indexOf + 1] = lay;
	emit dataChanged(index(indexOf), index(indexOf + 1));
	return true;
}

void SpriteAnimation::move(Layer* lay, int newPosition) {
	if (newPosition < 0 || newPosition > m_Layers.size()) return;
	int indexOf = m_Layers.indexOf(lay);
	if (indexOf == -1 || newPosition == indexOf) return;

	m_Layers.takeAt(indexOf);
	m_Layers.insert(newPosition, lay);
	emit dataChanged(index(0), index(m_Layers.size()));

}

bool SpriteAnimation::addLayer() {
	beginInsertRows(QModelIndex(), m_Layers.size(), m_Layers.size());
	m_Layers.push_back(new Layer("layer" + QString::number(m_Layers.size())));
	endInsertRows();
	emit onNumberOfLayersChanged();
	return true;
}

bool SpriteAnimation::addLayer(Layer* lay) {
	const int indexOf = m_Layers.indexOf(lay);
	if (indexOf != -1) {
		beginInsertRows(QModelIndex(), indexOf, indexOf);
		m_Layers.insert(indexOf, new Layer("layer" + QString::number(m_Layers.size())));
		endInsertRows();
		emit onNumberOfLayersChanged();
		return true;
	}

	return false;
}

bool SpriteAnimation::remove(Layer* lay) {
	if (!lay) return false;
	const int indexOf = m_Layers.indexOf(lay);
	if (indexOf != -1) {
		beginRemoveRows(QModelIndex(), indexOf, indexOf);
		m_Layers.removeAt(indexOf);
		delete lay;
		endRemoveRows();
		emit onNumberOfLayersChanged();
		return true;
	}
	return false;
}

// QAbstractListModel
QVariant SpriteAnimation::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QVariant(QVariant::Invalid);

	if (index.row() >= getNumberOfLayers())
		return QVariant(QVariant::Invalid);

	if (role == Qt::DisplayRole) {
		return QVariant::fromValue(m_Layers.at(index.row()));
	}
	else
		return QVariant();
}
int SpriteAnimation::rowCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return 0;
	return getNumberOfLayers();
}

////// SpriteSheet //////
SpriteSheet::SpriteSheet(QObject* parent): QAbstractListModel(parent) {}

SpriteSheet::SpriteSheet(int externalID, QObject* parent) : QAbstractListModel(parent), m_externalID(externalID) {}

SpriteSheet::SpriteSheet(int externalID, int width, int height, int powerColorTable, QObject* parent) :
	QAbstractListModel(parent), m_img(width, height), m_powerOfColorTable(powerColorTable), m_externalID(externalID) {
	m_img.fill(0);
}


SpriteSheet::SpriteSheet(const PG::UTIL::IDImage& img, int powerColorTable, QObject* parent) :
	QAbstractListModel(parent), m_img(img), m_powerOfColorTable(powerColorTable) {}

SpriteSheet::SpriteSheet(const SpriteSheet& sheet) :
	m_img(sheet.getSpriteSheet())
{}

SpriteSheet::~SpriteSheet() {}


bool SpriteSheet::removeCutoutID(int ID) {
	if (ID <= 0) return false; // zero sheet cant be removed

	const int indexOf = m_cutoutsIDs.indexOf(ID);
	if (indexOf != -1) {
		beginRemoveRows(QModelIndex(), indexOf, indexOf);
		m_cutoutsIDs.removeAt(indexOf);
		endRemoveRows();

		return true;
	}
	return false;
}


void SpriteSheet::refresh() {
	layoutAboutToBeChanged();
	layoutChanged();
	emit dataChanged(index(0), index(m_cutoutsIDs.size()));
}

//setters
void SpriteSheet::push_backCutoutID(int id) {
	if (!m_cutoutsIDs.contains(id)) {
		beginInsertRows(QModelIndex(), m_cutoutsIDs.size(), m_cutoutsIDs.size());
		//layoutAboutToBeChanged();
		m_cutoutsIDs.push_back(id);
		//layoutChanged();
		endInsertRows();
		//endInsertColumns();
		emit onNumberOfCutoutsChanged();
	}
#ifdef DEBUG
	else assert_Test("Sprite sheet already has a cutout with the same ID!", true);
#endif

}

void SpriteSheet::set(int width, int height, int powerColorTable, bool resizeSprites) {
	if (m_powerOfColorTable != powerColorTable) {
		m_powerOfColorTable = powerColorTable;
		emit numberOfColorsChanged();
	}

	if (getWidth() != width || getHeight() != height) {
		if (resizeSprites)
			m_img.resize(width, height, PG::UTIL::IDImage::NearestNeighbor);
		else
			m_img.resize(width, height, PG::UTIL::IDImage::KeepOld);
		emit onHeightChanged();
		emit onWidthChanged();
	}
}

void SpriteSheet::setExternalID(int External) {
	if (m_externalID == External) return;
	m_externalID = External;
	emit externalIDChanged();
}

QVariant SpriteSheet::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QVariant(QVariant::Invalid);

	if (index.row() >= m_cutoutsIDs.size())
		return QVariant(QVariant::Invalid);

	if (role == Qt::DisplayRole) {
		return QVariant::fromValue(m_cutoutsIDs.at(index.row()));
	}
	else
		return QVariant();
}
int SpriteSheet::rowCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return 0;
	return m_cutoutsIDs.size();
}

//getters
int SpriteSheet::getWidth() const {
	return getSpriteSheet().getWidth();
}
int SpriteSheet::getHeight() const {
	return getSpriteSheet().getHeight();
}
int SpriteSheet::getNumberOfCutouts() const {
	return m_cutoutsIDs.size();
}

int SpriteSheet::getPowerOfColorTable() const {
	return m_powerOfColorTable;
}

int SpriteSheet::getSizeOfColorTable() const {
	return pow(2.0, m_powerOfColorTable);
}

bool SpriteSheet::isExternalOpened() const {
	if (!m_externalColortables.empty()) {
		return true;
	}
	else {
		return !getPredifinedExternalColorTable(m_externalID, m_externalColortables).empty();
	}
}
const QVector<QColorTable>& SpriteSheet::getExternalColortables() const {
	if (m_externalColortables.empty())
		return getPredifinedExternalColorTable(m_externalID, m_externalColortables);
	else return m_externalColortables;
}

PG::UTIL::IDImage& SpriteSheet::getSpriteSheet() {
	return isExternal() && m_img.empty() ? getPredifinedExternalSpriteSheet(m_externalID) : m_img;
}
const PG::UTIL::IDImage& SpriteSheet::getSpriteSheet() const {
	return isExternal() && m_img.empty() ? getPredifinedExternalSpriteSheet(m_externalID) : m_img;
}

void SpriteSheet::openExternal(const PG::UTIL::IDImage& idImage, int powerOfColorTable, const QVector<QColorTable>& colortables){
	if (idImage.empty() || colortables.empty())
		return;
	m_img = idImage;
	m_powerOfColorTable = powerOfColorTable;
	m_externalColortables = colortables;
	emit onWidthChanged();
	emit onHeightChanged();
	emit numberOfColorsChanged();
}

void SpriteSheet::makeNotExternal(int width, int height, int powerColorTable) {
	if (m_externalID < 0) return;
	m_externalID = -1;
	m_externalColortables.clear();
	m_powerOfColorTable = powerColorTable;
	m_img.resize(width, height);
	m_img.fill(0);
}

void SpriteSheet::makeExternal(int externalID) {
	if (externalID < 0 || m_externalID == externalID) return;
	m_externalID = externalID;
	m_img.clear();
	m_externalColortables.clear();
	m_powerOfColorTable = -1;
}

inline bool getCutoutImageHelper(PG::UTIL::IDImage& outCutoutImage, const PG::UTIL::IDImage& inIDImage, const Cutout* inCut) {
	if (!inCut) {
		outCutoutImage = PG::UTIL::IDImage();
		return false;
	}


	const int trueWidth = (inCut->getX() + inCut->getWidth() >= inIDImage.getWidth()) ? inIDImage.getWidth() - inCut->getX() : inCut->getWidth();
	const int trueHeight = (inCut->getY() + inCut->getHeight() >= inIDImage.getHeight()) ? inIDImage.getHeight() - inCut->getY() : inCut->getHeight();

	if (trueWidth <= 0 || trueHeight <= 0) {
		PG_ERROR_STREAM("Cutout width or height is invalid! (" << trueWidth << ", " << trueHeight << ")");
		outCutoutImage = PG::UTIL::IDImage();
		return false;
	}

	inIDImage.getWindow(PG::UTIL::touvec2(inCut->getPosition()), PG::UTIL::uvec2(trueWidth, trueHeight), outCutoutImage);
	return true;
}

PG::UTIL::RGBAImage idImageToColorPG(const PG::UTIL::IDImage& idImage, unsigned int colortableID, const QVector<QColor>& colortable) {
	assert_Test("Color table ID out of bound!", colortableID * 16 + 16 > colortable.size());

	PG::UTIL::RGBAImage sprite(idImage.getWidth(), idImage.getHeight());
	unsigned int i = 0;
	for (const unsigned char id : idImage) {
		int colorIndex = colortableID * 16 + int(id);
		if (colorIndex >= colortable.size()) {
			sprite[i].r = 255;
			sprite[i].g = 0;
			sprite[i].b = 255;
			sprite[i].a = 255;
		}
		else {
			const QColor& color = colortable[colorIndex];
			sprite[i].r = color.red();
			sprite[i].g = color.green();
			sprite[i].b = color.blue();
			sprite[i].a = color.alpha();
		}
		i++;
	}

	return sprite;
}

PG::UTIL::RGBAImage SpriteSheet::getSpritePG(unsigned int colortableID, const QVector<QColor>& colortable) const {
	return idImageToColorPG(getSpriteSheet(), colortableID, colortable);
}

PG::UTIL::RGBAImage SpriteSheet::getSpritePG(const Cutout* cut, unsigned int colortableID, const QVector<QColor>& colortable) const {
	if (!cut) return PG::UTIL::RGBAImage();
	assert_Test("Colortable size is invalid!", getSizeOfColorTable() <= 0);
	

	PG::UTIL::IDImage spriteIDs;
	if (!getCutoutImageHelper(spriteIDs, getSpriteSheet(), cut))
		return PG::UTIL::RGBAImage();

	return idImageToColorPG(spriteIDs, colortableID, colortable);
}

QImage idImageToColor(const PG::UTIL::IDImage& idImage, unsigned int colortableID, const QVector<QColor>& colortable, bool alpha = false) {
	assert_Test("Color table ID out of bound!", colortableID * 16 + 16 > colortable.size());

	if (alpha) {
		QImage sprite(idImage.getWidth(), idImage.getHeight(), QImage::Format_RGBA8888);
		unsigned int i = 0;
		for (const unsigned char id : idImage) {
			int colorIndex = colortableID * 16 + int(id);
			if (colorIndex >= colortable.size()) {
				sprite.bits()[i] = 255;
				sprite.bits()[i + 1] = 0;
				sprite.bits()[i + 2] = 255;
				sprite.bits()[i + 3] = 255;
			}
			else {
				const QColor& color = colortable[colorIndex];
				sprite.bits()[i] = color.red();
				sprite.bits()[i + 1] = color.green();
				sprite.bits()[i + 2] = color.blue();
				sprite.bits()[i + 3] = color.alpha();
			}
			i += 4;
		}

		return sprite;

	}

	QImage sprite(idImage.getWidth(), idImage.getHeight(), QImage::Format_RGB888);
#ifdef DEBUG
	if (spriteIDs.getWidth() != cut->getWidth()) PG_INFO_STREAM("Original Width: " << cut->getWidth() << " new: " << spriteIDs.getWidth());
	if (spriteIDs.getHeight() != cut->getHeight()) PG_INFO_STREAM("Original Height: " << cut->getHeight() << " new: " << spriteIDs.getHeight());
#endif

	//uchar * data = new uchar[cutout->getWidth()*cutout->getHeight()*3];
	unsigned int i = 0;
	for (const unsigned char id : idImage) {
		const unsigned int colorIndex = colortableID * 16 + int(id);
		QColor color(255, 0, 255);
		if (colorIndex < colortable.size())
			color = colortable[colorIndex];
		const unsigned int y = (i / idImage.getWidth());
		sprite.setPixel(i - y * idImage.getWidth(), y, qRgb(color.red(), color.green(), color.blue()));
		i++;
	}

	return sprite;
}

QImage SpriteSheet::getSprite(unsigned int colortableID, const QVector<QColor>& colortable, bool alpha) const {
	assert_Test("Colortable size is invalid!", getSizeOfColorTable() <= 0);

	return idImageToColor(getSpriteSheet(), colortableID, colortable, alpha);
}

QImage SpriteSheet::getSprite(const Cutout* cut, unsigned int colortableID, const QVector<QColor>& colortable, bool alpha) const {
	if (!cut) return QImage();

	assert_Test("Colortable size is invalid!", getSizeOfColorTable() <= 0);
	

	PG::UTIL::IDImage spriteIDs;
	if (!getCutoutImageHelper(spriteIDs, getSpriteSheet(), cut))
		return QImage();

	return idImageToColor(spriteIDs, colortableID, colortable, alpha);
}


PG::UTIL::IDImage SpriteSheet::getSpritePGIDs(const Cutout* cut) const {
	PG::UTIL::IDImage spriteIDs;
	if (!getCutoutImageHelper(spriteIDs, getSpriteSheet(), cut))
		return PG::UTIL::IDImage();
	return spriteIDs;
}

QImage SpriteSheet::getSpriteIDs(const Cutout* cut) const {
	if (!cut) return QImage();

	PG::UTIL::IDImage spriteIDs;
	if (!getCutoutImageHelper(spriteIDs, getSpriteSheet(), cut))
		return QImage();

	QImage sprite(spriteIDs.getWidth(), spriteIDs.getHeight(), QImage::Format_RGB888);
	for (unsigned int i = 0; i < spriteIDs.getWidth() * spriteIDs.getHeight() * 3; i += 3) {
		const unsigned char id = spriteIDs[i / 3];
		sprite.bits()[i] = id;
		sprite.bits()[i + 1] = id;
		sprite.bits()[i + 2] = id;
	}

	return sprite;
}

////// SPRITE DATA //////

SpriteData::SpriteData(QObject* parent) : QAbstractListModel(parent), m_selectedKeyframe(nullptr) {
	if (!parent) return;

	if (g_defaultExternalSpriteSheet.empty()) {
		PG::FILE::loadTGA(getResourcePath().toStdString() + "/materials/external_sprite_sheet.tga", g_defaultExternalSpriteSheet);
	}

	g_externalSpriteSheet.clear();
	g_externalColorTable.clear();
	QDir dir(getResourcePath() + "/materials/external");
	if (dir.exists()) {
		QStringList externals = dir.entryList(QStringList() << "external_*.tga");
		if (externals.size() > 100) return;
		const std::string dir = (getResourcePath() + "/materials/external/").toStdString();
		for (const auto& external : externals) {
			const QStringList split = QFileInfo(external).baseName().split('_');
			if (split.size() != 2) continue;
			const int id = split.last().toInt();
			if (id <= 0 || id > 99) continue;
			qInfo() <<"Opening default external sprite sheet: " << QFileInfo(external).baseName();
			PG::UTIL::IDImage img;
			if (PG::FILE::loadTGA(dir + external.toStdString(), img) && !img.empty() && img.getWidth() <= 1024 && img.getHeight() <= 1024) {
				g_externalSpriteSheet[id] = std::move(img);
			}

			PG::UTIL::RGBAImage colortable;
			if (PG::FILE::loadTGA(dir + "external_"+std::to_string(id)+"_colortable.tga", colortable) && !colortable.empty() && colortable.size() <= 96) {
				QColorTable qTable;
				qTable.reserve(colortable.size());
				for (const PG::UTIL::rgba& color : colortable)
					qTable.push_back(QColor(color.r, color.g, color.b, color.a));
				g_externalColorTable[id].push_back(std::move(qTable));
			}
		}
	}

}

SpriteData::~SpriteData() {
	close();
}


inline void writeText(QDataStream& out, const QString& text) {
	out << (qint32)text.size();
	out << text;
}

bool SpriteData::save(const QString& file) {
	if (file.size() == 0) return false;

	if (m_spriteSheets.empty() || m_colortables.empty()) return false;

	QFile qfile(file);
	if (!qfile.open(QIODevice::WriteOnly)) return false;
	QDataStream out(&qfile);
	out.writeRawData("PGSH", 4);
	out << (quint16)3; //version

	out << "More info at 'https://github.com/ProgSys/pg_disatools'";
	//writeText(out, "More info at 'https://github.com/ProgSys/pg_disatools'");

	//ColorTable
	out << "ColorTable";
	//writeText(out, "ColorTable");

	out << (quint16)m_colortables.size();

	for (const QColorTable& colortable : m_colortables) {
		out << (quint16)colortable.size();
		for (const QColor& color : colortable) {
			out << (quint8)color.red();
			out << (quint8)color.green();
			out << (quint8)color.blue();
			out << (quint8)color.alpha();
		}
	}

	//SpriteSheets
	out << "SpriteSheets";
	//writeText(out, "SpriteSheets");

	out << (quint16)getNumberOfSpriteSheets();
	for (const SpriteSheet* sheet : m_spriteSheets) {
		out << (qint16)sheet->getExternalID();
		out << (qint16)sheet->getPowerOfColorTable();
		if (sheet->isEmpty()) {
			out << (quint16)0 << (quint16)0 << (quint32) 0;
		}
		else {
			out << (quint16)sheet->getWidth();
			out << (quint16)sheet->getHeight();
			out.writeBytes((char*) sheet->getSpriteSheet().data(), sheet->getSpriteSheet().size());
		}
	}

	//Cutouts
	out << "Cutouts";
	//writeText(out, "Cutouts");

	out << (quint32)m_cutouts.size();
	for (const Cutout* cut : m_cutouts) {
		out << (quint16)m_spriteSheets[cut->getSheetID()]->getExternalID();
		out << (quint16)cut->getSheetID();
		out << (quint16)cut->getX();
		out << (quint16)cut->getY();
		out << (quint16)cut->getWidth();
		out << (quint16)cut->getHeight();
		out << (quint16)cut->getDefaultColorTable();
	}

	//Cutouts
	out << "Animations";
	//writeText(out, "Animations");
	out << (quint32)m_animations.size();
	for (const SpriteAnimation* ani : m_animations) {
		out << (quint32)ani->getID();
		out << ani->getName();
		//writeText(out, ani->getName());

		out << (quint32)ani->getNumberOfMarkers();
		for (const Marker* mark : ani->getMarkers()->getList()) {
			out << (quint32)mark->getStart();
			out << (quint32)mark->getDuration();
			out << (quint32)mark->getType();
			out << (qint16)mark->getX();
			out << (quint16)mark->getY();
		}

		out << (quint32)ani->getNumberOfLayers();
		for (const Layer* lay : ani->getLayers()) {
			//writeText(out, lay->getName());
			out << lay->getName();
			out << (quint8)lay->isHidden();

			out << (quint32)lay->getNumberOfKeyframes();
			for (const Keyframe* key : lay->getKeyframes()) {
				out << (quint16)key->getCutoutID();
				out << (quint16)key->getColortableID();
				out << (qint32)key->getStart();
				out << (qint32)key->getDuration();

				out << (qint16)key->getAnchorX();
				out << (qint16)key->getAnchorY();

				out << (qint16)key->getOffsetX();
				out << (qint16)key->getOffsetY();

				out << (qint16)key->getScaleX();
				out << (qint16)key->getScaleY();

				out << (qint16)key->getRotation();
				out << (qint16)key->getTransparency();
				out << (qint16)key->getMic();
			}
		}

	}

	qfile.close();

	return true;
}

inline int findCutout(const QVector<SpriteSheet*>& spriteSheets, const QVector<Cutout*>& cutouts, const PG::FILE::shfileCutout& currCutout) {
	auto findIt = cutouts.end();
	if (currCutout.external_sheet) {
		findIt = std::find_if(cutouts.begin(), cutouts.end(), [&spriteSheets, &currCutout](const Cutout* cut) {
			SpriteSheet* sheet = spriteSheets[cut->getSheetID()];
			if (!sheet->isExternal()) return false;
			return sheet->getExternalID() == currCutout.external_sheet && cut->isSame(currCutout.x, currCutout.y, currCutout.width, currCutout.height, currCutout.colortable);
		});
	}
	else {
		findIt = std::find_if(cutouts.begin(), cutouts.end(), [&spriteSheets, &currCutout](const Cutout* cut) {
			SpriteSheet* sheet = spriteSheets[cut->getSheetID()];
			if (sheet->isExternal()) return false;
			return cut->getSheetID() == currCutout.sheet && cut->isSame(currCutout.x, currCutout.y, currCutout.width, currCutout.height, currCutout.colortable);
			});
	}

	if (findIt == cutouts.end()) return -1;
	else return std::distance(cutouts.begin(), findIt);
}

bool SpriteData::importSH(const QString& file) {
	if (file.size() == 0) return false;
	close();

	PG::FILE::SH sh;

	if (sh.open(file.toStdString())) {
		qInfo() << "Couldn't read sh file!";
		return false;
	}

	if (sh.getSpriteSheets().empty() || sh.getSheetsInfos().size() != sh.getSpriteSheets().size()) {
		qInfo() << "There are no sprite sheets inside the SH file! File may be corrupt!";
		return false;
	}

	if (sh.getAnimations().empty()) {
		qInfo() << "Animation data is missing!";
	}

	if (sh.getKeyframes().empty()) {
		qInfo() << "Keyframes data is missing!";
	}

	if (sh.getCutouts().empty()) {
		qInfo() << "Cutouts data is missing!";
	}

	if (sh.getColortables().empty()) {
		qInfo() << "There are no color tables! File may be corrupt!";
		return false;
	}

	if (sh.getColortables().front().size() < 15) {
		qInfo() << "Colortable is too small! File may be corrupt! (" << sh.getColortables().front().size() << ")";
		return false;
	}

	//save sheets
	for (unsigned int i = 0; i < sh.getSheetsInfos().size(); i++) {
		const PG::FILE::shfileSheetInfo& info = sh.getSheetsInfos()[i];
		const PG::UTIL::IDImage& sheet = sh.getSpriteSheets()[i];
		m_spriteSheets.push_back(new SpriteSheet(sheet, info.power_colortablesize, this));
	}

	//load colortable
	for (const PG::FILE::ColorTable& table : sh.getColortables()) {
		QColorTable qTable;
		qTable.reserve(table.size());
		for (const PG::UTIL::rgba& color : table)
			qTable.push_back(QColor(color.r, color.g, color.b, color.a));
		m_colortables.push_back(std::move(qTable));
	}


	beginInsertRows(QModelIndex(), 0, sh.getAnimations().size());
	unsigned int aniCount = 0;
	m_animations.reserve(sh.getAnimations().size());

	//some cutouts my not be used in any animation
	std::vector<bool> cutoutUseMap(sh.getCutouts().size(), false);

	// lambda member function to add a cutout
	auto addCoutout = [this](const PG::FILE::shfileCutout& shCutout) {
		int cutoutID = findCutout(m_spriteSheets, m_cutouts, shCutout);

		if (cutoutID < 0) {
			if (shCutout.external_sheet) {
				cutoutID = m_cutouts.size();
				//find or create external sprite sheet
				int spriteSheetIndex = findExternalSpriteSheetIndex(shCutout.external_sheet);
				SpriteSheet* spriteSheet;
				if (spriteSheetIndex < 0) {
					spriteSheet = new SpriteSheet(shCutout.external_sheet, this);
					spriteSheetIndex = m_spriteSheets.size();
					m_spriteSheets.push_back(spriteSheet);
				}
				else {
					spriteSheet = m_spriteSheets[spriteSheetIndex];
				}
				m_cutouts.push_back(new Cutout(spriteSheetIndex, PG::UTIL::ivec2(shCutout.x, shCutout.y), PG::UTIL::ivec2(shCutout.width, shCutout.height), shCutout.colortable, this));
				spriteSheet->push_backCutoutID(m_cutouts.size()-1);
			}
			else {
				assert_Test("Invalid sprite sheet ID!", currCutout.sheet >= m_spriteSheets.size() || currCutout.sheet < 0);
				assert_Test("Invalid color table ID!", currCutout.colortable * 16 + 16 > getNumberOfColors());
				Cutout* cutout = new Cutout(shCutout.sheet, PG::UTIL::ivec2(shCutout.x, shCutout.y), PG::UTIL::ivec2(shCutout.width, shCutout.height), shCutout.colortable, this);
				cutoutID = m_cutouts.size();
				m_spriteSheets[shCutout.sheet]->push_backCutoutID(cutoutID);
				m_cutouts.push_back(cutout);
			}
		}
		return cutoutID;
	};

	auto currAni = sh.getAnimations().begin();
	while (currAni != sh.getAnimations().end()) {

		m_animations.push_back(new SpriteAnimation(currAni->id, "unknown" + QString::number(aniCount), this));

		auto currKey = sh.getKeyframes().begin() + currAni->start_keyframe;
		assert(currKey != sh.getKeyframes().end());
		int startOffset = 0;
		int keyCount = 0;
		while (
			currKey != sh.getKeyframes().end()
			&& ((currAni == sh.getAnimations().end() || (currAni + 1) == sh.getAnimations().end()) ? true : (currKey < (sh.getKeyframes().begin() + (currAni + 1)->start_keyframe)))
			) {
			assert_Test("Current key can't be the end key", currKey == sh.getKeyframes().end());

			//add marker
			if (currKey->global_x || currKey->global_y || (currKey->type != 0 && currKey->type != 3 && currKey->type != 2))
				m_animations.back()->getMarkers()->push_back(new Marker(startOffset, 1, currKey->type, currKey->global_x, currKey->global_y));

			if (currKey->duration == 0 || currKey->type == 3 || currKey->type == 2) {
				currKey++;
				continue;
			}

			const PG::FILE::shfileLayers& currlayer = sh.getLayers()[currKey->bundel_index];
			int layerCount = 0;

			for (unsigned int i = currlayer.start_cutout; i < currlayer.start_cutout + currlayer.number_of_cutouts; i++) {
				if (i >= sh.getCutouts().size())
					continue;
				const PG::FILE::shfileCutout& currCutout = sh.getCutouts()[i];
				cutoutUseMap[i] = true; //mark cutout as used
				const int cutoutID = addCoutout(currCutout);

				Layer* layer = nullptr;
				if (layerCount < m_animations.back()->getLayers().size()) {
					layer = m_animations.back()->getLayers()[layerCount];
				}
				else {
					m_animations.back()->push_backLayer("layer" + QString::number(layerCount));
					layer = m_animations.back()->getLayers().back();
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
					currCutout.anchorx, currCutout.anchory,
					currCutout.scalex, currCutout.scaley,
					currCutout.offsetx, currCutout.offsety,
					currCutout.rotation, currCutout.transparency, currCutout.mic);

				layerCount++;
			}

			keyCount++;
			startOffset += currKey->duration;

			currKey++;
			if (currKey == sh.getKeyframes().end()) break;
		}
		aniCount++;
		currAni++;
	}

	//add unused cutouts 
	for (auto it = cutoutUseMap.begin(); it != cutoutUseMap.end(); ++it) {
		if (!*it) addCoutout(sh.getCutouts()[std::distance(cutoutUseMap.begin(), it)]);
	}


	m_currentColorTable = 0;

	if (m_animations.empty()) {
		m_currentAnimation = -1;
		emit onNumberOfAnimationsChanged();
		emit onCurrentAnimationChanged();
		emit onAnimationChanged(nullptr);
	}
	else {
		m_currentAnimation = 0;
		emit onNumberOfAnimationsChanged();
		emit onCurrentAnimationChanged();
		emit onAnimationChanged(m_animations[m_currentAnimation]);
	}

	m_lastFile = file;

	endInsertRows();

	emit onLastFileNameChanged();
	emit onNumberOfCutoutsChanged();
	emit numberOfSheetsChanged();
	emit allSpriteSheetsChanged();
	emit numberOfColorTablesChanged();
	emit allColorTablesChanged();


	return true;
}

bool SpriteData::exportSH(const QString& file) {
	if (file.size() == 0) return false;

	//make sure data is valid
	reorganizeData();

	PG::FILE::SH sh;

	for (const SpriteSheet* sheet : m_spriteSheets) {
		if (sheet->isExternal()) continue;
		sh.getSpriteSheets().push_back(sheet->getSpriteSheet());
		//TODO set the last values
		const unsigned char p = sheet->getPowerOfColorTable();
		sh.getSheetsInfos().push_back({ 0, unsigned short(sheet->getWidth()), unsigned short(sheet->getHeight()),p,0,0,0 });
	}

	for (unsigned int i = 0; i < m_colortables.size(); i++)
		sh.getColortables().push_back(getColortableGL(i));

	//some cutouts may be unused by any animation
	//but they shall also be saved
	std::vector<bool> cutoutUseMap(m_cutouts.size(), false);

	const int keyframeSizeBefore = sh.getKeyframes().size();
	for (const SpriteAnimation* ani : m_animations) {
		assert_Test("Value is too big!", ani->getID() > 65534);
		sh.getAnimations().push_back({ (unsigned short)sh.getKeyframes().size(), (unsigned short)ani->getID() });

		unsigned int startFrame = 0;
		unsigned int keyCount = 0;
		const unsigned int totalTrackSize = ani->getTotalFrames();

		//write keys
		while (startFrame < totalTrackSize) {
			unsigned int nextFrame = totalTrackSize;

			//find the end of the next frame
			for (const Layer* lay : ani->getLayers()) {
				if (lay->isHidden()) continue;
				for (const Keyframe* key : lay->getKeyframes()) {
					if (startFrame >= key->getStart() && startFrame < key->getEnd() && nextFrame > key->getEnd()) {
						nextFrame = key->getEnd();
					}
				}
			}

			//insert a keyframe with the propertys of the marker
			const Marker* foundMarker = nullptr;
			for (const Marker* mark : ani->getMarkers()->getList()) {
				if (mark->getStart() == startFrame) {
					foundMarker = mark;
				}
				else if (startFrame < mark->getStart() && mark->getStart() < nextFrame) {
					nextFrame = mark->getStart();
				}
			}


			assert_Test("Start frame and end frame are the same!", startFrame == nextFrame);

			PG::FILE::shfileKeyframe shKey;
			shKey.duration = ((nextFrame - startFrame) > 255) ? 255 : (nextFrame - startFrame);
			shKey.type = 0;
			shKey.global_x = 0;
			shKey.global_y = 0;
			shKey.bundel_index = sh.getLayers().size();

			if (foundMarker) {
				shKey.global_x = foundMarker->getX();
				shKey.global_y = foundMarker->getY();

				if (foundMarker->getType() != 0 && foundMarker->getType() != 2 && foundMarker->getType() != 3)
					shKey.type = foundMarker->getType();
			}

			//bundels
			QList<const Keyframe*> keys;
			for (const Layer* lay : ani->getLayers()) {
				if (lay->isHidden()) continue;
				for (const Keyframe* key : lay->getKeyframes()) {
					if (key->getStart() <= startFrame && key->getEnd() > startFrame)
						keys.push_back(key);
				}
			}

			//TODO
			if (keys.isEmpty()) {
				PG_INFO_STREAM("There can't be an empty area!")
					return false;
			}

			PG::FILE::shfileLayers bundle;
			bundle.start_cutout = sh.getCutouts().size();
			bundle.number_of_cutouts = keys.size();
			sh.getLayers().push_back(bundle);

			//writeCutouts
			for (const Keyframe* key : keys) {
				const Cutout* cut = m_cutouts[key->getCutoutID()];
				const SpriteSheet* sheet = m_spriteSheets[cut->getSheetID()];

				cutoutUseMap[key->getCutoutID()] = true;
				assert_Test("Cutout is nullptr!", !cut);

				PG::FILE::shfileCutout shCut;
				if (sheet->isExternal()) {
					shCut.external_sheet = sheet->getExternalID(); // get a sheet from different file by it's ID
					shCut.sheet = 0;
					shCut.colortable = 0;
				}
				else {
					shCut.external_sheet = 0;
					shCut.sheet = cut->getSheetID(); // there is not one big sprite sheet but multiple smaller one
					shCut.colortable = cut->getDefaultColorTable(); // the 16 rgba colortable which the sheet should use
				}
				shCut.anchorx = key->getAnchorX();  // rotation and mirror point
				shCut.anchory = key->getAnchorY(); // rotation and mirror point

				//sprite info
				shCut.x = cut->getX();
				shCut.y = cut->getY();
				shCut.width = cut->getWidth();
				shCut.height = cut->getHeight();
				shCut.scalex = key->getScaleX();
				shCut.scaley = key->getScaleY();

				shCut.offsetx = key->getOffsetX();
				shCut.offsety = key->getOffsetY();
				shCut.rotation = key->getRotation();

				shCut.transparency = key->getTransparency();
				shCut.mic = key->getMic();

				sh.getCutouts().push_back(shCut);
			}

			startFrame += shKey.duration;

			sh.getKeyframes().push_back(shKey);

			if (shKey.type == 1) {
				if (keyCount != 0 && nextFrame != totalTrackSize) {
					PG::FILE::shfileKeyframe skKeyType3 = shKey;
					skKeyType3.duration = 0;
					skKeyType3.type = 3;
					skKeyType3.global_x = shKey.global_x;
					skKeyType3.global_y = shKey.global_y;
					sh.getKeyframes().push_back(skKeyType3);
				}
			}
			keyCount++;
		}

		//add end keyframe?
		if (sh.getKeyframes().size() > keyframeSizeBefore) {
			PG::FILE::shfileKeyframe shKey = sh.getKeyframes().back();
			shKey.duration = 0;
			shKey.type = 2;
			sh.getKeyframes().push_back(shKey);
		}

	}

	//save unused cutouts
	for (auto it = cutoutUseMap.begin(); it != cutoutUseMap.end(); ++it) {
		if (*it) continue;
		const Cutout* cut = m_cutouts[std::distance(cutoutUseMap.begin(), it)];
		const SpriteSheet* sheet = m_spriteSheets[cut->getSheetID()];
		assert_Test("Cutout is nullptr!", !cut);

		PG::FILE::shfileCutout shCut;
		if (sheet->isExternal()) {
			shCut.external_sheet = sheet->getExternalID(); // get a sheet from different file by it's ID
			shCut.sheet = 0;
			shCut.colortable = 0;
		}
		else {
			shCut.external_sheet = 0;
			shCut.sheet = cut->getSheetID(); // there is not one big sprite sheet but multiple smaller one
			shCut.colortable = cut->getDefaultColorTable(); // the 16 rgba colortable which the sheet should use
		}
		shCut.anchorx = 0.0f;  // rotation and mirror point
		shCut.anchory = 0.0f; // rotation and mirror point

		//sprite info
		shCut.x = cut->getX();
		shCut.y = cut->getY();
		shCut.width = cut->getWidth();
		shCut.height = cut->getHeight();
		shCut.scalex = 100.0f;
		shCut.scaley = 100.0f;

		shCut.offsetx = 0.0f;
		shCut.offsety = 0.0f;
		shCut.rotation = 0.0f;

		shCut.transparency = 0;
		shCut.mic = 0;

		sh.getCutouts().push_back(shCut);

		PG::FILE::shfileLayers layer;
		layer.start_cutout = sh.getCutouts().size() - 1;
		layer.number_of_cutouts = 1;
		sh.getLayers().push_back(layer);
	}

	PG_INFO_STREAM("START SAVING!")
		return !sh.save(file.toStdString());
}

int SpriteData::findExternalSpriteSheetIndex(int externalID) const {
	auto findIt = std::find_if(m_spriteSheets.begin(), m_spriteSheets.end(), [externalID](SpriteSheet* s) {
		return s->getExternalID() == externalID;
		});
	return findIt == m_spriteSheets.end() ? -1 : std::distance(m_spriteSheets.begin(), findIt);
}

bool SpriteData::openExternalSpriteSheet(int sheetID) {
	if (sheetID < 0 || sheetID >= m_spriteSheets.size()) {
		qInfo() << "sheetID out of bounds! ("<< sheetID<< ", "<< m_spriteSheets.size()<<")";
		return false;
	}
	SpriteSheet* spriteSheet = m_spriteSheets[sheetID];
	if (!spriteSheet || !spriteSheet->isExternal()) {
		qInfo() << "Sprite sheets is not external!";
		return false;
	}

	//select file
	QFileDialog openDialog(nullptr);
	openDialog.setNameFilter(tr("SH (*.sh);;SPG (*.spg)"));


	QString file;
	QStringList fileNames;
	if (openDialog.exec()) {
		fileNames = openDialog.selectedFiles();
		if (fileNames.size() > 0) 
			file = fileNames[0];
		else
			return false;
	}
	const QFileInfo info(file);
	const QString suffix = info.completeSuffix().toLower();
	//prosess file
	if (suffix == "sh") {
		PG::FILE::SH sh;

		if (sh.open(file.toStdString())) {
			qInfo() << "Couldn't read .sh file!";
			return false;
		}

		if (sh.getSheetsInfos().empty() || sh.getSpriteSheets().empty() || sh.getColortables().empty()) {
			qInfo() << "No sprite sheets or color tables!";
			return false;
		}

		//load colortable
		QVector<QColorTable> colortables;
		for (const PG::FILE::ColorTable& table : sh.getColortables()) {
			QColorTable qTable;
			for (const PG::UTIL::rgba& color : table)
				qTable.push_back(QColor(color.r, color.g, color.b, color.a));
			colortables.push_back(qTable);
		}
		
		spriteSheet->openExternal(sh.getSpriteSheets().front(), sh.getSheetsInfos().front().power_colortablesize, colortables);
	}
	else if (suffix == "spg") {
		SpriteData spg;
		if (!spg.open(file)) {
			qInfo() << "Couldn't read .spg file!";
			return false;
		}

		if(spg.getSpriteSheets().empty() || spg.getColorTables().empty()) {
			qInfo() << "No sprite sheets or color tables!";
			return false;
		}

		const auto& sheet = spg.getSpriteSheets().front();
		spriteSheet->openExternal(sheet->getSpriteSheet(), sheet->getPowerOfColorTable(), spg.getColorTables() );
	}
	else {
		qInfo() << "Unknown file: "<< file <<" ("<< suffix <<")";
		return false;
	}
	
	emit externalSpriteSheetOpened(spriteSheet);
	emit spriteSheetChanged(sheetID);

	return true;
}

///if png is false then tga is used
int SpriteData::exportSprites(const QString& folder, const QString& type) {
	if (folder.isEmpty() || type.isEmpty() || m_cutouts.empty()) return 0;


	bool png = false;
	if (type == "PNG") {
		png = true;
	}
	else if (type != "TGA")
		return 0;

	//convertToRGBA
	std::vector< PG::UTIL::RGBAImage > images;
	images.reserve(m_spriteSheets.size());
	for (const SpriteSheet* sheet : m_spriteSheets)
		images.push_back(PG::UTIL::RGBAImage(sheet->getWidth(), sheet->getHeight()));

	for (const Cutout* cut : m_cutouts) {
		if (cut->getSheetID() < 0 || cut->getSheetID() >= m_spriteSheets.size()) continue;
		if (cut->getDefaultColorTable() >= getNumberOfColors() / 16) continue;


		const PG::UTIL::RGBAImage sprite = m_spriteSheets[cut->getSheetID()]->getSpritePG(cut, cut->getDefaultColorTable(), getColorTable());
		images[cut->getSheetID()].setWindow(PG::UTIL::touvec2(cut->getPosition()), sprite);
	}



	int imgCount = 0;
	if (png) {
		for (const PG::UTIL::RGBAImage& image : images) {
			QImage qimg(&(image[0].r), image.getWidth(), image.getHeight(), QImage::Format_RGBA8888);
			qimg.save(folder + "/" + QFileInfo(m_lastFile).baseName() + "_" + QString::number(imgCount) + ".png", 0, 100);
			imgCount++;
		}

	}
	else {
		for (const PG::UTIL::RGBAImage& image : images) {
			std::stringstream o;
			o << folder.toStdString() << "/" << QFileInfo(m_lastFile).baseName().toStdString() << "_" << imgCount << ".tga";
			PG::FILE::saveTGA(o.str(), image);
			imgCount++;
		}
	}

	return imgCount;

}

int SpriteData::exportSpritesIDs(const QString& folder, const QString& type) {
	if (folder.isEmpty() || type.isEmpty() || m_cutouts.empty()) return 0;

	bool png = false;
	if (type == "PNG") {
		png = true;
	}
	else if (type != "TGA")
		return 0;


	int imgCount = 0;
	if (png) {
		for (const SpriteSheet* sprite : m_spriteSheets) {

			QImage qimg(&(sprite->getSpriteSheet()[0]), sprite->getSpriteSheet().getWidth(), sprite->getSpriteSheet().getHeight(), QImage::Format_Indexed8);
			qimg.save(folder + "/" + QFileInfo(m_lastFile).baseName() + "_ID_" + QString::number(imgCount) + ".png", 0, 100);
			imgCount++;
		}

	}
	else {
		for (const SpriteSheet* sprite : m_spriteSheets) {
			std::stringstream o;
			o << folder.toStdString() << "/" << QFileInfo(m_lastFile).baseName().toStdString() << "_ID_" << imgCount << ".tga";
			PG::FILE::saveTGA(o.str(), sprite->getSpriteSheet());
			imgCount++;
		}
	}

	return imgCount;
}

bool SpriteData::exportSprite(int cutoutID) {
	QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Export color sprite"),
		QFileInfo(m_lastFile).baseName() + "_Sprite" + QString::number(cutoutID),
		tr("PNG (*.png);;TGA (*.tga)"));
	return exportSprite(cutoutID, fileName);

}

bool SpriteData::exportSprite(int cutoutID, const QString& file) {
	if (file.isEmpty() || m_cutouts.empty() || cutoutID < 0 || cutoutID >= m_cutouts.size()) return false;

	const Cutout* cut = m_cutouts[cutoutID];
	assert_Test("Cutout is nullptr", !cut);

	const SpriteSheet* sheet = m_spriteSheets[cut->getSheetID()];
	const QString ext = QFileInfo(file).completeSuffix();

	if (ext == "tga") {
		if (PG::FILE::saveTGA(file.toStdString(), sheet->getSpritePG(cut, cut->getDefaultColorTable(), getColorTable())))
			return true;
	}
	else if (!ext.isEmpty()) {
		if (sheet->getSprite(cut, cut->getDefaultColorTable(), getColorTable(), true).save(file, 0, 100))
			return true;
	}
	return false;
}

bool SpriteData::exportSpriteIDs(int cutoutID) {
	QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Export IDs sprite"),
		QFileInfo(m_lastFile).baseName() + "_SpriteIds" + QString::number(cutoutID),
		tr("PNG (*.png);;TGA (*.tga)"));
	return exportSpriteIDs(cutoutID, fileName);
}

bool SpriteData::exportSpriteIDs(int cutoutID, const QString& file) {
	if (file.isEmpty() || m_cutouts.empty() || cutoutID < 0 || cutoutID >= m_cutouts.size()) return false;

	const Cutout* cut = m_cutouts[cutoutID];
	assert_Test("Cutout is nullptr", !cut);

	const SpriteSheet* sheet = m_spriteSheets[cut->getSheetID()];
	const QString ext = QFileInfo(file).completeSuffix();

	if (ext == "tga") {
		if (PG::FILE::saveTGA(file.toStdString(), sheet->getSpritePGIDs(cut)))
			return true;
	}
	else if (!ext.isEmpty()) {
		if (sheet->getSpriteIDs(cut).save(file, 0, 100))
			return true;
	}
	return false;
}

bool SpriteData::exportSpriteSheet(Cutout* cut) {
	if (!cut)
		return false;
	QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Export IDs sprite"),
		QFileInfo(m_lastFile).baseName() + "_sheet"+ QString::number(cut->getSheetID()) +"_colortable" + QString::number(cut->getDefaultColorTable()),
		tr("PNG (*.png);;TGA (*.tga)"));
	return exportSpriteSheet(cut, fileName);
}

bool SpriteData::exportSpriteSheet(Cutout* cut, const QString& file) {
	if (!cut  || file.isEmpty())
		return false;

	const SpriteSheet* sheet = m_spriteSheets[cut->getSheetID()];
	const QString ext = QFileInfo(file).completeSuffix();

	if (ext == "tga") {
		if (PG::FILE::saveTGA(file.toStdString(), sheet->getSpritePG(cut, cut->getDefaultColorTable(), getColorTable())))
			return true;
	}
	else if (!ext.isEmpty()) {
		if (sheet->getSprite(cut->getDefaultColorTable(), getColorTable(), true).save(file, 0, 100))
			return true;
	}
	return false;
}

bool SpriteData::importSpriteAsIDs(int cutoutID, const QString& file) {
	if (cutoutID < 0 || cutoutID >= m_cutouts.size()) return false;
	return importSpriteAsIDs(*m_cutouts[cutoutID], file);
}

bool SpriteData::importSpriteAsIDs(int sheetID, int x, int y, int width, int height, const QString& file) {
	if (sheetID < 0 || sheetID >= m_spriteSheets.size()) return false;
	Cutout cut(sheetID, PG::UTIL::ivec2(x, y), PG::UTIL::ivec2(width, height));
	return importSpriteAsIDs(cut, file);
}

bool SpriteData::importSpriteAsIDs(Cutout& cut, const QString& fileIn) {
	QString file = fileIn;
	if (file.isEmpty()) {
		QFileDialog openDialog(nullptr);
		openDialog.setNameFilter(tr("PNG (*.png);;TGA (*.tga)"));

		QStringList fileNames;
		if (openDialog.exec()) {
			fileNames = openDialog.selectedFiles();
			if (fileNames.size() > 0) {
				file = fileNames[0];
			}
			else
				return false;
		}
	}

	assert_Test("sheetID is out of bound!", cut.getSheetID() > m_spriteSheets.size());
	SpriteSheet* sheet = m_spriteSheets[cut.getSheetID()];
	const QImage newCutout(file);
	const int imageSize = (newCutout.width() * newCutout.height());
	if (imageSize <= 0) return false;

	if (cut.getWidth() != newCutout.width() || cut.getHeight() != newCutout.height()) {
		QMessageBox::StandardButton reply = QMessageBox::warning(nullptr, "Resize?",
			"Given image has a different size then the sprite! Should the sprite be resized?",
			QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::Yes) {
			cut.setWidth((cut.getX() + newCutout.width() >= sheet->getWidth()) ? sheet->getWidth() - cut.getX() : newCutout.width());
			cut.setHeight((cut.getY() + newCutout.height() >= sheet->getHeight()) ? sheet->getHeight() - cut.getY() : newCutout.height());
		}
	}

	for (int iy = 0; iy < cut.getHeight(); iy++)
		for (int ix = 0; ix < cut.getWidth(); ix++)
			sheet->getSpriteSheet().set(cut.getX() + ix, cut.getY() + iy, (QColor(newCutout.pixel(ix, iy)).red() > 15) ? 15 : QColor(newCutout.pixel(ix, iy)).red());

	emit spriteSheetChanged(cut.getSheetID());
	return true;
}


inline int findColorIndex(int colortableID, const QColorTable& colortable, const QColor& color, int alpha, int colortableSize = 16) {
	for (unsigned int i = colortableID * colortableSize; i < colortable.size(); i++) {
		const QColor& comparecolor = colortable[i];

		if (alpha == 0 && comparecolor.alpha() == 0) return i;
		else if (color.red() == comparecolor.red() && color.blue() == comparecolor.blue() && color.green() == comparecolor.green() && alpha == comparecolor.alpha())
			return i;
	}
	return -1;
}

inline bool compareColorTables(int colortableID, int colortableSize, const QColorTable& colorTable, const QColorTable& compareTable) {
	for (const QColor& rgba : colorTable) {
		if (findColorIndex(colortableID, compareTable, rgba, rgba.alpha(), colortableSize) < 0) return false;
	}
	return true;
}

inline QColorTable extractColorTable(int offset, int colortableSize, const QColorTable& colorTable) {
	QColorTable out;
	out.reserve(colortableSize);
	for (unsigned int i = offset; i < offset + colortableSize; i++)
		out.push_back(colorTable[i]);
	return out;
}

inline QColorTable buildColorTable(const QImage& image) {
	QColorTable table;

	for (int y = 0; y < image.height(); y++)
		for (int x = 0; x < image.width(); x++) {
			QColor col(image.pixel(x, y));
			int alpha = qAlpha(image.pixel(x, y));

			bool found = false;
			for (const QColor& rgba : table) {
				if (alpha == 0 && rgba.alpha() == 0) {
					found = true;
					break;
				}
				if (rgba.red() == col.red() && rgba.green() == col.green() && rgba.blue() == col.blue() && rgba.alpha() == alpha) {
					found = true;
					break;
				}
			}
			if (!found) table.push_back(QColor(col.red(), col.green(), col.blue(), alpha));
		}
	return table;
}

inline QString openImage() {
	QFileDialog openDialog(nullptr);
	openDialog.setNameFilter(QObject::tr("PNG (*.png);;TGA (*.tga)"));

	QStringList fileNames;
	if (openDialog.exec()) {
		fileNames = openDialog.selectedFiles();
		if (fileNames.size() > 0)
			return fileNames[0];
	}
	return "";
}

bool SpriteData::importSpriteAsColor(int cutoutID, const QString& fileIn) {
	if (cutoutID < 0 || cutoutID >= m_cutouts.size()) return false;

	QString file = fileIn;
	if (file.isEmpty()) {
		file = openImage();
		if (file.isEmpty()) return false;
	}

	//target image
	QImage newColorCutout(file);

	return importSpriteAsColor(*m_cutouts[cutoutID], newColorCutout);
}

bool SpriteData::importSpriteAsColor(int sheetID, int x, int y, int width, int height, int colorTableOffset, QImage& image) {
	if (sheetID < 0 || sheetID >= m_spriteSheets.size()) return false;

	if (colorTableOffset < 0) {
		bool ok;
		colorTableOffset = QInputDialog::getInt(0, tr("Enter color table offset"),
			tr("Please select a color table offset:"), 0, 0, getNumberOfColors() / 16, 1, &ok);
		if (!ok) return false;
	}
	Cutout cut(sheetID, PG::UTIL::ivec2(x, y), PG::UTIL::ivec2(width, height), colorTableOffset);

	return importSpriteAsColor(cut, image);
}

bool SpriteData::importSpriteAsColorForSheet(int sheetID, const QString& fileIn) {
	QString file = fileIn;
	if (file.isEmpty()) {
		file = openImage();
		if (file.isEmpty()) return false;
	}

	//target image
	QImage newColorCutout(file);
	if (!(newColorCutout.width() == 32 || newColorCutout.width() == 64 ||  newColorCutout.width() == 128 || newColorCutout.width() == 256 || newColorCutout.width() == 512) ||
		!(newColorCutout.height() == 32 || newColorCutout.height() == 64 ||  newColorCutout.height() == 128 || newColorCutout.height() == 256 || newColorCutout.height() == 512)) {
		QMessageBox::critical(nullptr, "Error",
			"Given image has invalid size, must be power of 2 and between 32 - 512!",
			QMessageBox::Ok);
		return false;
	}

	//get sheet
	if (sheetID < 0 || sheetID >= m_spriteSheets.size()) 
		return false;
	
	SpriteSheet* sheet = m_spriteSheets[sheetID];
	if (sheet->getWidth() < newColorCutout.width() || sheet->getHeight() < newColorCutout.height()) {
		QMessageBox::StandardButton reply = QMessageBox::warning(nullptr, "Resize?",
			"Given image has a different size then the sprite sheet! Should the sprite sheet be resized?",
			QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::Yes) {
			const int oldWidth = sheet->getWidth();
			const int oldHeight = sheet->getHeight();
			sheet->set(newColorCutout.width(), newColorCutout.height(), sheet->getPowerOfColorTable(), true);
			resizeSpritesOnSheet(sheet, oldWidth, oldHeight, newColorCutout.width(), newColorCutout.height());
		}
	}

	return importSpriteAsColor(sheetID, 0, 0, newColorCutout.width(), newColorCutout.height(), -1, newColorCutout);
}

bool SpriteData::importSpriteAsColor(int sheetID, int x, int y, int width, int height, int colorTableOffset, const QString& fileIn) {
	QString file = fileIn;
	if (file.isEmpty()) {
		file = openImage();
		if (file.isEmpty()) return false;
	}

	//target image
	QImage newColorCutout(file);
	return importSpriteAsColor(sheetID, x, y, width, height, colorTableOffset, newColorCutout);
}

bool SpriteData::importSpriteAsColor(Cutout& cut, QImage& newColorCutout) {
	if (newColorCutout.isNull()) return false;

	//target image
	const int imageSize = (newColorCutout.width() * newColorCutout.height());
	if (newColorCutout.isNull() || imageSize <= 0) return false;

	//get sheet
	SpriteSheet* sheet = m_spriteSheets[cut.getSheetID()];
	const int colortableSize = sheet->getSizeOfColorTable();

	//check size
	if (cut.getWidth() != newColorCutout.width() || cut.getHeight() != newColorCutout.height()) {
		QMessageBox::StandardButton reply = QMessageBox::warning(nullptr, "Resize?",
			"Given image has a different size then the sprite! Should the sprite be resized?",
			QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::Yes) {
			cut.setWidth((cut.getX() + newColorCutout.width() >= sheet->getWidth()) ? sheet->getWidth() - cut.getX() : newColorCutout.width());
			cut.setHeight((cut.getY() + newColorCutout.height() >= sheet->getHeight()) ? sheet->getHeight() - cut.getY() : newColorCutout.height());
		}
	}

	//image colortables
	QColorTable imageTable = buildColorTable(newColorCutout);
	QColorTable currentTable = extractColorTable(cut.getDefaultColorTable(), colortableSize, getColorTable());
	PG_INFO_STREAM("HERE!");
	if (imageTable.size() > currentTable.size()) {
		QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error",
			"Your image has " + QString::number(imageTable.size()) + " colors, but the sprite sheet supports max " + QString::number(currentTable.size()) + " colors!\n"
			"Should the application try to automatically reduce the colors?",
			QMessageBox::Yes | QMessageBox::Cancel);

		if (reply == QMessageBox::Cancel) return false;

		//remap colors
		ImageLib::reduceColors(newColorCutout, currentTable.size());
		imageTable = buildColorTable(newColorCutout);
	}

	PG_INFO_STREAM("HERE!");
	QColorTable& colortable = getColorTable();
	if (!compareColorTables(cut.getDefaultColorTable(), 16, imageTable, colortable)) {
		QMessageBox::StandardButton reply = QMessageBox::warning(nullptr, "Continue?",
			"Given color image uses colors that is not inside the color table! "
			"Should the colors be insert into the color table or be reassigned?"
			"\nYes: Replace the current color table colors with the new colors form the image."
			"\nNo: Try to reassign the image colors to the  color table colors."
			,
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (reply == QMessageBox::Yes) {
			//resize table
			int targetSize = cut.getDefaultColorTable() * 16 + imageTable.size();
			const int mod = targetSize % 16;
			if (mod) targetSize += 16 - mod;
			targetSize -= getNumberOfColors();
			if (targetSize > 0) addColors(cut.getDefaultColorTable() * 16, targetSize);
			//set table
			for (unsigned int i = 0; i < imageTable.size(); i++) {
				unsigned int offset = i + cut.getDefaultColorTable() * 16;
				colortable[offset] = imageTable[i];
			}
			emit colorTableChanged(m_currentColorTable);
		}
		else if (reply == QMessageBox::No) {
			ImageLib::reassignColors(newColorCutout, currentTable);
			imageTable = buildColorTable(newColorCutout);
		}
		else return false;
	}


	PG_INFO_STREAM("HERE!");
	PG::UTIL::IDImage idImage(newColorCutout.width(), newColorCutout.height());
	for (int y = 0; y < newColorCutout.height(); y++)
		for (int x = 0; x < newColorCutout.width(); x++) {
			QColor col(newColorCutout.pixel(x, y));
			int alpha = qAlpha(newColorCutout.pixel(x, y));

			int indexOf = findColorIndex(cut.getDefaultColorTable(), colortable, col, alpha, 16);

			const int colortableID = indexOf - cut.getDefaultColorTable() * 16;
			if (indexOf < colortable.size() && colortableID >= 0) {
				idImage.set(x, y, (colortableID >= colortableSize) ? 0 : colortableID);
			}
			else {
				PG_INFO_STREAM("Out colortableID: " << colortableID << " index: " << indexOf << " colorTableSize: " << colortable.size() << " (" << col.red() << ", " << col.green() << ", " << col.blue() << ", " << alpha << ")")
					QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error",
						"Found a color inside the image which isn't inside the color table!"
						" Conversion must have failed!"
						"\nColor (" + QString::number(col.red()) + ", " + QString::number(col.green()) + ", " + QString::number(col.blue()) + ", " + QString::number(alpha) + ") at (" + QString::number(x) + ", " + QString::number(y) + ") dons't have corresponding ID!"
						"\nPlease make sure you use the same colors as the sprite and your image doesn't have any compression artifacts!",
						QMessageBox::Ok);
				return false;
			}
		}
	PG_INFO_STREAM("HERE!");
	//set image
	for (int y = 0; y < cut.getHeight() && y < idImage.getHeight(); y++)
		for (int x = 0; x < cut.getWidth() && x < idImage.getWidth(); x++) {
			sheet->getSpriteSheet().set(cut.getX() + x, cut.getY() + y, idImage.get(x, y));
		}
	emit spriteSheetChanged(cut.getSheetID());
	return true;
}

bool SpriteData::importSpriteSheetAsColor(Cutout* cut) {
	if (!cut || isExternal(cut))
		return false;
	return SpriteData::importSpriteSheetAsColor(cut, openImage());
}

bool SpriteData::importSpriteSheetAsColor(Cutout* cut, const QString& fileIn) {
	if (!cut || isExternal(cut) || fileIn.isEmpty())
		return false;

	QString file = fileIn;
	if (file.isEmpty()) {
		file = openImage();
		if (file.isEmpty()) return false;
	}

	//target image
	QImage newColorCutout(file);
	if (cut->getSheetID() < 0 || cut->getSheetID() >= m_spriteSheets.size())
		return false;
	SpriteSheet* sheet = m_spriteSheets[cut->getSheetID()];
	return importSpriteAsColor(cut->getSheetID(), 0, 0, sheet->getWidth(), sheet->getHeight(), cut->getDefaultColorTable(), newColorCutout);
}

bool SpriteData::dump(const QString& filepath) {
	if (filepath.isEmpty() || m_cutouts.empty()) return 0;

	QFile file(filepath);
	if (file.open(QIODevice::WriteOnly)) {
		QTextStream out(&file);

		for (const SpriteAnimation* ani : m_animations) {
			out << "Animation:\n\tid: " << ani->getID() << "\n\tname: " << ani->getName() << "\n\tnumber of layers: " << ani->getNumberOfLayers();

			for (const Layer* lay : ani->getLayers()) {
				out << "\n\t\tLayer:";
				out << "\n\t\t\t name: " << lay->getName();
				out << "\n\t\t\t duration: " << lay->getDuration();
				for (const Keyframe* key : lay->getKeyframes()) {
					out << "\n\t\t\tKeyframe:";

					out << "\n\t\t\t\tCutoutID: " << key->getCutoutID();
					out << "\n\t\t\t\tColortableID: " << key->getColortableID();

					out << "\n\t\t\t\tStart Frame: " << key->getStart();
					out << "\n\t\t\t\tDuration: " << key->getDuration();

					out << "\n\t\t\t\tOffsetX: " << key->getOffsetX();
					out << "\n\t\t\t\tOffsetY: " << key->getOffsetY();

					out << "\n\t\t\t\tAnchorX: " << key->getAnchorX();
					out << "\n\t\t\t\tAnchorY: " << key->getAnchorY();

					out << "\n\t\t\t\tScaleX: " << key->getScaleX();
					out << "\n\t\t\t\tScaleY: " << key->getScaleY();

					out << "\n\t\t\t\tRotation: " << key->getRotation();
					out << "\n\t\t\t\tTransparency: " << (int)key->getTransparency();
					out << "\n\t\t\t\tMic: " << (int)key->getMic();

					out << "\n\t\t\t\tCutout:";
					const Cutout* cut = m_cutouts[key->getCutoutID()];
					out << "\n\t\t\t\t\tIndex: " << cut->getSheetID();
					out << "\n\t\t\t\t\tWidth: " << cut->getWidth();
					out << "\n\t\t\t\t\tHeight: " << cut->getHeight();

					out << "\n\t\t\t\tCutout end.";
					out << "\n\t\t\tKeyframe end.";
				}
				out << "\n\t\tLayer end.";
			}
			out << "\nAnimation end.\n";

		}

		file.close();
	}
	else {
		file.close();
		return false;
	}
	return true;
}

Cutout* SpriteData::getCutout(int cutoutIndex) const {
	if (cutoutIndex < 0 || cutoutIndex >= m_cutouts.size()) {
		PG_INFO_STREAM("Cutout index out of bound! " << cutoutIndex);
		return nullptr;
	}
	assert_Test("Cutout is nullptr!", !m_cutouts[cutoutIndex]);
	return m_cutouts[cutoutIndex];
}

int SpriteData::getCoutoutIndex(Cutout* cutout) const {
	if (!cutout) return -1;
	auto findIt = std::find(m_cutouts.begin(), m_cutouts.end(), cutout);
	if (findIt != m_cutouts.end())
		return std::distance(m_cutouts.begin(), findIt);
	return -1;
}

SpriteSheet* SpriteData::getSpriteSheet(int spriteSheetIndex) const {
	if (spriteSheetIndex < 0 || spriteSheetIndex >= m_spriteSheets.size()) return nullptr;
	return m_spriteSheets[spriteSheetIndex];
}

void SpriteData::clearSelectedKey() {
	setSelectedKey(nullptr);
}

void SpriteData::unhideAllCutouts() {
	for (Cutout* cut : m_cutouts)
		cut->setHidden(false);
}

void SpriteData::update() {
	emit refresh();
}

void SpriteData::batchDoubleResize() {
	QStringList fileNames = QFileDialog::getOpenFileNames(nullptr,
		tr("Select Sprites to resize"), "", tr("Sprite (*.sh)"));

	if (fileNames.empty()) return;

	QString targetFolder = QFileDialog::getExistingDirectory(nullptr,
		tr("Select target output folder"), "");

	if (targetFolder.isEmpty()) return;

	for (const QString& file : fileNames) {
		if (!importSH(file)) continue;

		for (SpriteSheet* editTarget : m_spriteSheets) {

			const float xScale = 2;
			const float yScale = 2;
			editTarget->set(editTarget->getWidth() * 2, editTarget->getHeight() * 2, editTarget->getPowerOfColorTable(), true);
			//resize cutotus
			for (int i : editTarget->getCutoutIDs()) {
				Cutout* c = m_cutouts[i];
				c->setPosition(c->getX() * xScale, c->getY() * yScale);
				c->setSize(c->getWidth() * xScale, c->getHeight() * yScale);
			}
			//resize keyframes
			for (SpriteAnimation* ani : m_animations)
				for (Layer* layer : ani->getLayers())
					for (Keyframe* key : layer->getKeyframes())
						for (int i : editTarget->getCutoutIDs()) {
							if (i == key->getCutoutID()) {
								key->setScaleX(key->getScaleX() / xScale);
								key->setScaleY(key->getScaleY() / yScale);

								key->setAnchorX(key->getAnchorX() * xScale);
								key->setAnchorY(key->getAnchorY() * yScale);
								break;
							}
						}


		}

		exportSH(targetFolder + "/" + QFileInfo(file).fileName());
	}
}

QColorTable& SpriteData::getColorTable() {
	assert_Test("Color table index out of bound!", m_currentColorTable < 0 || m_currentColorTable >= m_colortables.size());
	return m_colortables[m_currentColorTable];
}
const QColorTable& SpriteData::getColorTable() const {
	assert_Test("Color table index out of bound!", m_currentColorTable < 0 || m_currentColorTable >= m_colortables.size());
	return m_colortables[m_currentColorTable];
}

void SpriteData::setCurrentColorTable(int index) {
	if (m_currentColorTable == index || index < 0) return;
	if (index >= m_colortables.size()) index = 0;
	m_currentColorTable = index;
	emit currentColorTableChanged();
}

int SpriteData::getCurrentColorTable() const {
	return m_currentColorTable;
}

void SpriteData::insertColorTable(int index, bool copy) {

	QColorTable table;
	if (copy)
		table = m_colortables[index];
	else {
		table.reserve(getNumberOfColors());
		for (unsigned int i = 0; i < getNumberOfColors(); i++)
			table.push_back(QColor(0, 0, 0));
	}
	index++;
	if (index < 0 || index >= m_colortables.size()) {
		m_colortables.insert(m_colortables.size(), table);
		emit colorTableAdded(m_colortables.size() - 1);
	}
	else {
		m_colortables.insert(index, table);
		emit colorTableAdded(index);
	}
	m_currentColorTable++;
	emit numberOfColorTablesChanged();
	emit currentColorTableChanged();
}

void SpriteData::removeColorTable(int index) {
	if (m_colortables.size() <= 1) return;

	if (index >= m_colortables.size())
		index = m_colortables.size() - 1;
	m_colortables.removeAt(index);
	emit colorTableRemoved(index);

	emit numberOfColorTablesChanged();
	if (m_currentColorTable) {
		m_currentColorTable--;
		emit currentColorTableChanged();
	}
}

bool SpriteData::exportColortable(int index) {
	QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Export color table"),
		QFileInfo(m_lastFile).baseName() + "_ColorTable" + QString::number(index + 1),
		tr("PNG (*.png);;TGA (*.tga)"));

	return exportColortable(fileName, index);
}

bool SpriteData::exportColortable(const QString& file, int index) {
	if (m_colortables.empty()) return false;
	assert_Test("Color table index out of bound!", index < 0 || index >= m_colortables.size());
	if (index < 0 || index >= m_colortables.size()) index = 0;
	const QColorTable& colortable = m_colortables[index];

	if (colortable.isEmpty() || file.isEmpty()) return false;
	const QString ext = QFileInfo(file).completeSuffix();
	if (ext == "tga") {
		PG::UTIL::RGBAImage img(colortable.size(), 1);
		unsigned int i = 0;
		for (const QColor& color : colortable) {
			img[i].r = color.red();
			img[i].g = color.green();
			img[i].b = color.blue();
			img[i].a = color.alpha();
			i++;
		}
		PG::FILE::saveTGA(file.toStdString(), img);
	}
	else if (!ext.isEmpty()) {
		QImage img(colortable.size(), 1, QImage::Format_RGBA8888);
		unsigned int i = 0;
		for (const QColor& color : colortable) {
			img.bits()[i] = color.red();
			img.bits()[i + 1] = color.green();
			img.bits()[i + 2] = color.blue();
			img.bits()[i + 3] = color.alpha();
			i += 4;
		}
		return img.save(file, 0, 100);
	}
	else
		return false;
	return true;
}


bool SpriteData::importColortable(int index) {
	QFileDialog openDialog(nullptr);
	openDialog.setNameFilter(tr("PNG (*.png);;TGA (*.tga)"));

	QStringList fileNames;
	if (openDialog.exec()) {
		fileNames = openDialog.selectedFiles();
		if (fileNames.size() > 0) {
			return importColortable(fileNames[0], index);
		}
	}
	return false;
}


bool SpriteData::importColortable(const QString& file, int index) {
	if (m_colortables.empty()) return false;

	assert_Test("Color table index out of bound!", index < 0 || index >= m_colortables.size());
	if (index < 0 || index >= m_colortables.size()) index = 0;
	QColorTable& colortable = m_colortables[index];

	if (file.isEmpty() || m_cutouts.isEmpty()) return false;

	QImage colorTable(file);
	const int imageSize = (colorTable.width() * colorTable.height());
	PG_INFO_STREAM("Color table size: " << imageSize);
	if (imageSize < 16) {
		QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error",
			"Color table is too small! There must be minimum 16 colors!",
			QMessageBox::Ok);
		return false;
	}

	const int colortableMinSize = getMaxUsedColortable();
	const int newcolortableMinSize = imageSize / 16;
	if (newcolortableMinSize < colortableMinSize) {
		QMessageBox::StandardButton reply = QMessageBox::warning(nullptr, "Continue?",
			"Given color table (" + QString::number(imageSize) + ") is smaller then the current color table (" + QString::number(colortable.size()) + ")! Color table IDs that are outside the rage will be set to 0.",
			QMessageBox::Yes | QMessageBox::Cancel);
		if (reply == QMessageBox::Cancel)
			return false;

		for (SpriteAnimation* ani : m_animations) {
			for (Layer* lay : ani->getLayers())
				for (Keyframe* key : lay->getKeyframes()) {
					if (key->getColortableID() > newcolortableMinSize)
						key->setColortableID(0);
				}
		}

		for (Cutout* cut : m_cutouts) {
			if (cut->getDefaultColorTable() > newcolortableMinSize)
				cut->setDefaultColorTable(0);
		}

	}

	colortable.clear();
	for (int y = 0; y < colorTable.height(); y++)
		for (int x = 0; x < colorTable.width(); x++) {
			colortable.push_back(QColor(colorTable.pixel(x, y)));
			colortable.back().setAlpha(qAlpha(colorTable.pixel(x, y)));
		}


	//ajust all colortables to same size
	for (QColorTable& colortable : m_colortables) {
		if (colortable.size() > imageSize) {
			for (unsigned int i = colortable.size(); i != imageSize; i--)
				colortable.removeLast();
		}
		else if (colortable.size() < imageSize) {
			for (unsigned int i = colortable.size(); i < imageSize; i++)
				colortable.push_back(QColor(0, 0, 0));
		}
	}

	//make color tables mod 16 = 0
	for (QColorTable& colortable : m_colortables) {
		const unsigned int mod = colortable.size() % 16;
		if (!mod) continue;
		const unsigned int add = 16 - mod;
		for (unsigned int i = 0; i < add; i++)
			colortable.push_back(QColor(0, 0, 0));
	}


	emit allColorTablesChanged();
	return true;
}



void SpriteData::exportColorsToClipboard(int start, int size, int index) {
	if (size < 0 || m_colortables.empty())
		return;
	assert_Test("Color table index out of bound!", index < 0 || index >= m_colortables.size());
	if (index < 0 || index >= m_colortables.size()) index = 0;
	QColorTable& colortable = m_colortables[index];

	if (start >= colortable.size())
		return;
	if ((start + size) >= colortable.size())
		size = colortable.size() - start;

	QString out;
	auto begin = colortable.begin() + start;
	out += begin->name(QColor::HexArgb);
	++begin;
	auto end = begin + size;
	std::for_each(begin, end, [&out](const QColor& c) { out += "," + c.name(QColor::HexArgb); });

	QApplication::clipboard()->setText(out);
}

void SpriteData::importColorsFromClipboard(int at, int index) {
	if (at < 0 || m_colortables.empty())
		return;
	assert_Test("Color table index out of bound!", index < 0 || index >= m_colortables.size());
	if (index < 0 || index >= m_colortables.size()) index = 0;
	QColorTable& colortable = m_colortables[index];

	if (at >= colortable.size())
		return;

	QString in = QApplication::clipboard()->text();
	if (in.size() > 2000)
		return;
	QStringList colorNames = in.split(",", QString::SkipEmptyParts);
	if (colorNames.size() > 512)
		return;

	auto begin = colortable.begin() + at;
	auto end = ((at + colorNames.size()) >= colortable.size()) ? colortable.end() : begin + colorNames.size();
	auto itName = colorNames.begin();

	std::for_each(begin, end, [&itName](QColor& c) {
		c.setNamedColor(itName->trimmed());
		++itName;
		if (!c.isValid()) c.setRgb(255, 0, 0);
	});


	emit colorTableChanged(index);
}



QColor SpriteData::getColor(int index) const {
	assert_Test("Color table index out of bound!", m_currentColorTable < 0 || m_currentColorTable >= m_colortables.size());
	const QColorTable& colortable = m_colortables[m_currentColorTable];
	if (index >= colortable.size()) return colortable.last();
	return colortable[index];
}

void SpriteData::setColor(int index, const QColor& color) {
	assert_Test("Color table index out of bound!", m_currentColorTable < 0 || m_currentColorTable >= m_colortables.size());
	QColorTable& colortable = m_colortables[m_currentColorTable];
	if (index < colortable.size() && colortable[index] != color) {
		colortable[index] = color;
		emit colorTableChanged(m_currentColorTable);
	}
}

void SpriteData::addColors(int index, int number) {
	for (QColorTable& colortable : m_colortables) {
		if (index < 0 || index >= colortable.size()) {
			for (unsigned int i = 0; i < number; i++)
				colortable.push_back(QColor(0, 0, 0, 255));
		}
		else
			for (unsigned int i = 0; i < number; i++)
				colortable.insert(index, QColor(0, 0, 0, 255));
	}

	emit colorTableChanged(-1);
}

void SpriteData::removeColors(int index, int number) {
	for (QColorTable& colortable : m_colortables) {
		if (colortable.size() > 16) {

			if (index < 0 || index >= colortable.size()) {
				for (unsigned int i = 0; i < number; i++)
					colortable.removeLast();
			}
			else
				for (unsigned int i = 0; i < number; i++)
					colortable.removeAt(index);
		}
	}
	emit colorTableChanged(-1);
}

bool SpriteData::reorganizeData() {
	//make sure the sprite sheets are sorted correctly
	bool discrepancyFound = false;
	auto splitIt = std::stable_partition(m_spriteSheets.begin(), m_spriteSheets.end(),
		[](SpriteSheet* sheet) {
			return !sheet->isExternal();
		});

	for (auto it = m_spriteSheets.begin(); it != splitIt; ++it) {
		SpriteSheet* sheet = *it;
		assert(!sheet->isExternal());
		int sheetIndex = std::distance(m_spriteSheets.begin(), it);
		for (int i : sheet->getCutoutIDs()) {
			Cutout* cut = getCutouts()[i];
			if (cut->getSheetID() != sheetIndex) {
				discrepancyFound = true;
				cut->setSheetID(sheetIndex);
			}
		}
	}

	return discrepancyFound;
}

void SpriteData::close() {
	setSelected(nullptr);
	clearSelectedKey();
	m_currentAnimation = -1;
	m_currentColorTable = -1;
	emit onAnimationChanged(nullptr);
	emit onCurrentAnimationChanged();

	for (Cutout* cut : m_cutouts)
		delete cut;
	m_cutouts.clear();

	m_colortables.clear();

	for (SpriteSheet* sheet : m_spriteSheets)
		delete sheet;
	m_spriteSheets.clear();
	if (!m_animations.empty()) {
		beginRemoveRows(QModelIndex(), 0, m_animations.size() - 1);
		for (SpriteAnimation* ani : m_animations)
			delete ani;
		m_animations.clear();
		endRemoveRows();
	}

	emit onNumberOfCutoutsChanged();
	emit numberOfSheetsChanged();
	emit allSpriteSheetsChanged();
	emit numberOfColorTablesChanged();
	emit allColorTablesChanged();
	emit onNumberOfAnimationsChanged();

}

// getters
bool SpriteData::isOpen() const {
	//TODO
	return false;
}

int SpriteData::getNumberOfAnimations() const {
	return m_animations.size();
}

int SpriteData::getNumberOfCutouts() const {
	return m_cutouts.size();
}

int SpriteData::getNumberOfColortables() const {
	return m_colortables.size();
}

int SpriteData::getNumberOfColortableSets() const {
	if (m_colortables.empty()) return 0;
	return m_colortables.front().size() / 16;
}

int SpriteData::getNumberOfColors() const {
	if (m_colortables.empty()) return 0;
	return m_colortables.front().size();
}

int SpriteData::getMaxUsedColortable() const {
	int maxColortable = 0;
	for (const SpriteAnimation* ani : m_animations) {
		for (const Layer* lay : ani->getLayers())
			for (const Keyframe* key : lay->getKeyframes()) {
				if (maxColortable < key->getColortableID())
					maxColortable = key->getColortableID();
			}
	}
	return maxColortable;
}

int SpriteData::getNumberOfSpriteSheets() const {
	return m_spriteSheets.size();
}

int SpriteData::getCurrentAnimationIndex() const {
	return m_currentAnimation;
}

QString SpriteData::getLastFileName() const {
	return m_lastFile;
}

bool SpriteData::push_backAnimation(const QString& name, int ID) {
	if (name.isEmpty() || ID < 0) return false;

	beginInsertRows(QModelIndex(), m_animations.size(), m_animations.size());
	m_animations.push_back(new SpriteAnimation(ID, name));
	m_animations.back()->push_backLayer(new Layer("layer0", m_animations.back()));
	m_animations.back()->getLayers().back()->push_backKeyframe(10, 0, 0, 0, 0, 100, 100, 0, 0, 0, 0, 0);
	endInsertRows();

	emit onNumberOfAnimationsChanged();
	return true;
}

bool SpriteData::removeAnimation(int index) {
	if (index < 0 || index >= m_animations.size()) return false;

	beginRemoveRows(QModelIndex(), index, index);
	delete m_animations[index];
	m_animations.removeAt(index);
	endRemoveRows();

	emit onNumberOfAnimationsChanged();
	return true;
}

SpriteAnimation* SpriteData::getCurrentAnimation() {
	if (m_currentAnimation < 0 || m_animations.empty()) return nullptr;
	return m_animations[m_currentAnimation];
}

const SpriteAnimation* SpriteData::getCurrentAnimation() const {
	if (m_currentAnimation < 0 || m_animations.empty()) return nullptr;
	return m_animations[m_currentAnimation];
}

PG::FILE::ColorTable SpriteData::getColortableGL(int index) const {
	assert_Test("Color table index out of bound!", m_currentColorTable < 0 || m_currentColorTable >= m_colortables.size());
	const QColorTable& colortable = m_colortables[index];

	std::vector<PG::UTIL::rgba> glColortable(colortable.size());

	auto it = glColortable.begin();
	for (const QColor& color : colortable) {
		(*it).r = color.red();
		(*it).g = color.green();
		(*it).b = color.blue();
		(*it).a = color.alpha();
		it++;
	}
	return glColortable;
}

QImage SpriteData::getSprite(unsigned int cutoutID, unsigned int colortableID) const {
	if (cutoutID >= m_cutouts.size() || colortableID >= getNumberOfColors() / 16) {
		qInfo() << "Invalid cutoutID or colortableID: " << cutoutID << ", " << colortableID;
		return QImage();
	}

	Cutout* cutout = m_cutouts[cutoutID];
	
	const SpriteSheet* sheet = m_spriteSheets[cutout->getSheetID()];
	if (sheet->getPowerOfColorTable() > 14) return QImage(); // cant be that big?

	const QColorTable& colortable = ([this, sheet]() -> const QColorTable & {
		if (sheet->isExternal() && sheet->isExternalOpened())
			return sheet->getExternalColortables().front();
		else
			return getColorTable();
		})();

	if (sheet->getSizeOfColorTable() < 0 || colortableID * 16 + sheet->getSizeOfColorTable() > colortable.size())
		colortableID = 0;

	return sheet->getSprite(cutout, colortableID, colortable);
}

const SpriteSheet* SpriteData::getSpriteSheet(unsigned int spriteID) const {
	assert_Test("Sprite ID out of bound!", spriteID >= m_spriteSheets.size());
	if (spriteID >= m_spriteSheets.size()) return nullptr;
	return m_spriteSheets[spriteID];
}

//setters
void SpriteData::setCurrentAnimationByIndex(int index) {
	if (index < 0) index = 0;
	else if (index >= m_animations.size()) index = m_animations.size() - 1;
	if (m_currentAnimation == index) return;

	m_currentAnimation = index;
	emit onCurrentAnimationChanged();
	emit onAnimationChanged(m_animations[m_currentAnimation]);
}

void SpriteData::setSelected(Cutout* cutout) {
	if (m_selected == cutout) return;
	m_selected = cutout;
	emit selectedChanged();
}

void SpriteData::setSelectedKey(Keyframe* key) {
	if (m_selectedKeyframe == key) return;
	m_selectedKeyframe = key;
	//if(m_selectedKeyframe)
		//PG_INFO_STREAM("key selected: "<<m_selectedKeyframe->getCutoutID());
	emit selectedKeyChanged();
}

void SpriteData::setIsolateSelection(bool value) {
	if (m_isolateSelection == value) return;
	m_isolateSelection = value;
	emit isolateSelectionChanged();
}

void SpriteData::keyPressEvent(QKeyEvent* event) {
	if (m_selectedKeyframe) {
		switch (event->modifiers()) {
		case Qt::ControlModifier: {
			switch (event->key()) {
			case Qt::Key_Up: {m_selectedKeyframe->setAnchorY(m_selectedKeyframe->getAnchorY() - 1); }break;
			case Qt::Key_Down: {m_selectedKeyframe->setAnchorY(m_selectedKeyframe->getAnchorY() + 1); }break;
			case Qt::Key_Right: {m_selectedKeyframe->setAnchorX(m_selectedKeyframe->getAnchorX() + 1); }break;
			case Qt::Key_Left: {m_selectedKeyframe->setAnchorX(m_selectedKeyframe->getAnchorX() - 1); }break;
			}
		}break;
		case Qt::AltModifier: {
			switch (event->key()) {
			case Qt::Key_Up: {m_selectedKeyframe->setAnchorY(m_selectedKeyframe->getAnchorY() + 1); m_selectedKeyframe->setOffsetY(m_selectedKeyframe->getOffsetY() - 1); }break;
			case Qt::Key_Down: {m_selectedKeyframe->setAnchorY(m_selectedKeyframe->getAnchorY() - 1); m_selectedKeyframe->setOffsetY(m_selectedKeyframe->getOffsetY() + 1); }break;
			case Qt::Key_Right: {m_selectedKeyframe->setAnchorX(m_selectedKeyframe->getAnchorX() - 1); m_selectedKeyframe->setOffsetX(m_selectedKeyframe->getOffsetX() + 1); }break;
			case Qt::Key_Left: {m_selectedKeyframe->setAnchorX(m_selectedKeyframe->getAnchorX() + 1); m_selectedKeyframe->setOffsetX(m_selectedKeyframe->getOffsetX() - 1); }break;
			}
		}break;
		case Qt::ShiftModifier: {
			switch (event->key()) {
			case Qt::Key_Right: {m_selectedKeyframe->setRotation(m_selectedKeyframe->getRotation() - 1); }break;
			case Qt::Key_Left: {m_selectedKeyframe->setRotation(m_selectedKeyframe->getRotation() + 1); }break;
			}
		}break;
		default: {
			switch (event->key()) {
			case Qt::Key_Up: {m_selectedKeyframe->setOffsetY(m_selectedKeyframe->getOffsetY() - 1); }break;
			case Qt::Key_Down: {m_selectedKeyframe->setOffsetY(m_selectedKeyframe->getOffsetY() + 1); }break;
			case Qt::Key_Right: {m_selectedKeyframe->setOffsetX(m_selectedKeyframe->getOffsetX() + 1); }break;
			case Qt::Key_Left: {m_selectedKeyframe->setOffsetX(m_selectedKeyframe->getOffsetX() - 1); }break;
			}
		}break;
		}
	}
}

bool  SpriteData::addCutout(int sheetID) {
	if (sheetID < 0 || sheetID >= m_spriteSheets.size()) return false;

	m_cutouts.push_back(new Cutout(sheetID, PG::UTIL::ivec2(0, 0), PG::UTIL::ivec2(100, 100), this));
	emit onNumberOfCutoutsChanged();
	m_spriteSheets[sheetID]->push_backCutoutID(m_cutouts.size() - 1);

	return true;
}

bool  SpriteData::addCutout(int sheetID, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char colorID) {
	if (sheetID < 0 || sheetID >= m_spriteSheets.size()) return false;
	SpriteSheet* sheet = m_spriteSheets[sheetID];

	if (width == 0) width = 1;
	if (height == 0) height = 1;

	if (x >= sheet->getWidth() - 1) x = sheet->getWidth() - 4;
	if (y >= sheet->getHeight() - 1) y = sheet->getHeight() - 4;

	if (x + width >= sheet->getWidth()) width = sheet->getWidth() - x;
	if (y + height >= sheet->getHeight()) height = sheet->getHeight() - y;

	m_cutouts.push_back(new Cutout(sheetID, PG::UTIL::ivec2(x, y), PG::UTIL::ivec2(width, height), colorID, this));
	emit onNumberOfCutoutsChanged();
	sheet->push_backCutoutID(m_cutouts.size() - 1);

	return true;
}

bool SpriteData::removeCutout(Cutout* cut) {
	return removeCutoutID(m_cutouts.indexOf(cut));
}

bool SpriteData::removeCutoutID(int id, bool warning) {
	PG_INFO_STREAM("Delete cutout: " << id);
	if (id == 0) {
		QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error",
			"You can't delete the zero sprite!",
			QMessageBox::Ok);
		return false;
	}


	if (warning) {
		QMessageBox::StandardButton reply = QMessageBox::warning(nullptr, "Delete sprite?",
			"Are you sure you want to delete the sprite?",
			QMessageBox::Yes | QMessageBox::No);

		if (reply == QMessageBox::No)
			return false;
	}

	if (id > 0) {
		delete m_cutouts[id];
		m_cutouts.removeAt(id);

		//fix the keyframes
		for (SpriteAnimation* ani : m_animations) {
			for (Layer* lay : ani->getLayers()) {
				for (Keyframe* key : lay->getKeyframes()) {
					if (key->getCutoutID() == id) {
						key->setCutoutID(0);
					}
					else if (key->getCutoutID() > id) {
						key->setCutoutID(key->getCutoutID() - 1);
					}
				}
			}
		}

		//fix the spritesheets
		for (SpriteSheet* sheet : m_spriteSheets) {
			if (sheet->removeCutoutID(id)) break;
		}

		for (SpriteSheet* sheet : m_spriteSheets) {
			for (int& i : sheet->getCutoutIDs())
				if (i > id) i--;
			sheet->refresh();
		}

		emit onNumberOfCutoutsChanged();
	}
	return false;
}

void SpriteData::cropCutout(int cutoutIndex) {
	if (cutoutIndex < 0 || cutoutIndex >= m_cutouts.size()) return;


	Cutout* cutout = m_cutouts[cutoutIndex];
	if (isExternal(cutout)) return;
	//QColorTable& colortable = m_colortables[cutout->getDefaultColorTable()];
	SpriteSheet* sheet = m_spriteSheets[cutout->getSheetID()];

	//find start and endpoint
	PG::UTIL::ivec2 start = cutout->getPosition();
	PG::UTIL::ivec2 end = cutout->getPosition() + PG::UTIL::ivec2(cutout->getWidth(), cutout->getHeight());

	if (start.x >= sheet->getWidth() || start.y >= sheet->getHeight()) return;

	if (end.x >= sheet->getWidth()) end.x = sheet->getWidth() - 1;
	if (end.y >= sheet->getHeight()) end.y = sheet->getHeight() - 1;

	qDebug() << "Crop" << cutoutIndex << start << end;

	const PG::UTIL::IDImage& img = sheet->getSpriteSheet();

	PG::UTIL::ivec2 newStart = end;
	PG::UTIL::ivec2 newEnd = start;
	for (int x = start.x; x < end.x; x++) {
		for (int y = start.y; y < end.y; y++) {
			unsigned char id = img.get(x, y);

			unsigned int colorTableIndex = cutout->getDefaultColorTable() * 16 + id;
			if (colorTableIndex >= getColorTable().size())
				colorTableIndex = id;

			const QColor& color = getColorTable()[colorTableIndex];

			//alpha test
			if (color.alpha() != 0) {
				if (x < newStart.x) newStart.x = x;
				if (y < newStart.y) newStart.y = y;
				if (x > newEnd.x) newEnd.x = x;
				if (y > newEnd.y) newEnd.y = y;
			}

		}
	}

	qDebug() << "Found" << newStart << newEnd;
	if (newStart.x < newEnd.x && newStart.y < newEnd.y) {
		cutout->setPosition(newStart);
		cutout->setSize(newEnd - newStart + PG::UTIL::ivec2(1, 1));
	}

}

void SpriteData::autoFindCutouts(int sheetID) {
	if (sheetID < 0 || sheetID >= m_spriteSheets.size()) return;


	bool ok;
	const int colorTableOffset = QInputDialog::getInt(0, tr("Enter color table offset"),
		tr("Please select a color table offset:"), 0, 0, getNumberOfColors() / 16, 1, &ok);
	if (!ok) return;

	QMessageBox::StandardButton btn = QMessageBox::question(nullptr, "Prevent overlap?", "Don't create a cutout at locations where a cutout is already defined?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	if (btn & QMessageBox::Cancel) return;

	SpriteSheet* sheet = m_spriteSheets[sheetID];
	const PG::UTIL::IDImage& img = sheet->getSpriteSheet();
	PG::UTIL::IDImage visited(img.getWidth(), img.getHeight());
	for (unsigned char& b : visited) b = 0;


	struct aabb {
		PG::UTIL::ivec2 start = PG::UTIL::ivec2(9999, 9999);
		PG::UTIL::ivec2 end = PG::UTIL::ivec2(-9999, -9999);

		inline bool intersects(int x, int y, int width, int height) {
			return (abs(start.x - x) * 2 < (end.x - start.x + width)) &&
				(abs(start.y - y) * 2 < (end.y - start.y + height));
		}

		inline bool intersects(const Cutout* cut) {
			return intersects(cut->getX(), cut->getY(), cut->getWidth(), cut->getHeight());
		}

		inline bool intersects(const aabb& cut) {
			return intersects(cut.start.x, cut.start.y, cut.end.x - cut.start.x, cut.end.y - cut.start.y);
		}
	};

	QList<aabb> sprites;

	for (int x = 0; x < img.getWidth(); x++) {
		for (int y = 0; y < img.getHeight(); y++) {
			unsigned char& v = visited.get(x, y);
			if (v) continue;


			unsigned char id = img.get(x, y);
			unsigned int colorTableIndex = colorTableOffset * 16 + id;
			if (colorTableIndex >= getColorTable().size())
				colorTableIndex = id;

			//qDebug()<<"At ("<<x<<", "<<y<<"):"<<id<<table[id].alpha();
			if (getColorTable()[colorTableIndex].alpha() != 0) {

				aabb sprite;
				QQueue<PG::UTIL::ivec2> queue;
				queue.push_back(PG::UTIL::ivec2(x, y));
				while (!queue.isEmpty()) {
					//grow in all directions
					PG::UTIL::ivec2 pos = queue.front();
					queue.pop_front();
					//qDebug()<<"LOOP At"<<pos<<queue.size();
					if (pos.x < 0 || pos.y < 0 || pos.x >= img.getWidth() || pos.y >= img.getHeight()) continue;
					unsigned char& visit = visited.get(pos.x, pos.y);
					if (visit) continue;
					visit = 1;
					id = img.get(pos.x, pos.y);

					colorTableIndex = colorTableOffset * 16 + id;
					if (colorTableIndex >= getColorTable().size())
						colorTableIndex = id;
					if (getColorTable()[colorTableIndex].alpha() == 0) continue;
					//qDebug()<<"LOOP At"<<pos<<visit;

					if (pos.x < sprite.start.x) sprite.start.x = pos.x;
					if (pos.y < sprite.start.y) sprite.start.y = pos.y;
					if (pos.x > sprite.end.x) sprite.end.x = pos.x;
					if (pos.y > sprite.end.y) sprite.end.y = pos.y;


					//add all directions
					queue.push_back(pos + PG::UTIL::ivec2(1, 0));
					queue.push_back(pos + PG::UTIL::ivec2(0, 1));
					queue.push_back(pos + PG::UTIL::ivec2(-1, 0));
					queue.push_back(pos + PG::UTIL::ivec2(0, -1));
					queue.push_back(pos + PG::UTIL::ivec2(1, 1));
					queue.push_back(pos + PG::UTIL::ivec2(1, -1));
					queue.push_back(pos + PG::UTIL::ivec2(-1, 1));
					queue.push_back(pos + PG::UTIL::ivec2(-1, -1));
				}
				if (sprite.start.x < sprite.end.x && sprite.start.y < sprite.end.y)
					sprites.push_back(sprite);

			}

			v = 1;
		}
	}

	if (btn & QMessageBox::Yes) {
		for (const Cutout* cut : m_cutouts) {
			if (cut->getSheetID() != sheetID || isExternal(cut)) continue;

			QMutableListIterator<aabb> i(sprites);
			while (i.hasNext()) if (i.next().intersects(cut)) i.remove();
		}
	}

	//clean up overlapping sprites
	QList<aabb> spritesClean;
	for (unsigned int a = 0; a < sprites.size(); a++) {
		aabb& spriteA = sprites[a];
		if (spriteA.start.x < 0) continue;
		for (unsigned int b = a + 1; b < sprites.size(); b++) {
			aabb& spriteB = sprites[b];
			if (spriteB.start.x < 0) continue;

			if (spriteA.intersects(spriteB)) {
				if (spriteB.start.x < spriteA.start.x) spriteA.start.x = spriteB.start.x;
				if (spriteB.start.y < spriteA.start.y) spriteA.start.y = spriteB.start.y;
				if (spriteB.end.x > spriteA.end.x) spriteA.end.x = spriteB.end.x;
				if (spriteB.end.y > spriteA.end.y) spriteA.end.y = spriteB.end.y;
				//remove B
				spriteB.start.x = -100;
			}
		}
		spritesClean.push_back(spriteA);
	}

	for (const aabb& sprite : spritesClean)
		addCutout(sheetID, sprite.start.x, sprite.start.y, sprite.end.x - sprite.start.x + 1, sprite.end.y - sprite.start.y + 1, colorTableOffset);
}

bool SpriteData::addNewSpriteSheet() {
	if (m_spriteSheets.empty()) return false;
	CreateEmptySpriteSheet create;
	create.exec();

	if (create.isAccepted()) 
		return addNewSpriteSheet(create.getWidth(), create.getHeight(), create.getColorTablePower(), create.getExternalID());
	return false;
}

bool SpriteData::addNewSpriteSheet(int width, int height, int powerOfColorTable, int externalId) {
	if (width <= 0 || height <= 0 || powerOfColorTable <= 0) return false;

	if (externalId < 0) {
		auto findIt = std::find_if(m_spriteSheets.begin(), m_spriteSheets.end(), [](SpriteSheet* sheet) { return sheet->isExternal(); });
		std::for_each(findIt, m_spriteSheets.end(), [this](SpriteSheet* sheet) {
			for (int i : sheet->getCutoutIDs()) {
				Cutout* cut = m_cutouts[i];
				cut->setSheetID(cut->getSheetID() + 1);
			}
		});
		m_spriteSheets.insert(findIt, new SpriteSheet(-1, width, height, powerOfColorTable, this));

	}
	else {
		auto findIt = std::find_if(m_spriteSheets.begin(), m_spriteSheets.end(), [externalId](SpriteSheet* sheet) { return sheet->getExternalID() == externalId; });
		if (findIt == m_spriteSheets.end()) {
			m_spriteSheets.push_back(new SpriteSheet(externalId, this));
		}
		else {
			QMessageBox::warning(nullptr, "Duplicate sprite sheet",
				"External sprite sheet with the id "+QString::number(externalId)+ " is already present!"
				"\nIt is sprite sheet Nr. "+QString::number(std::distance(m_spriteSheets.begin(), findIt) + 1)+ ".",
				QMessageBox::Ok);
			return false;
		}
	}


	emit spriteSheetAdded();
	emit numberOfSheetsChanged();
	return true;
}

bool SpriteData::removeSpriteSheet(unsigned int index) {
	if (index >= m_spriteSheets.size()) return false;
	SpriteSheet* editTarget = m_spriteSheets[index];
	for (int id : editTarget->getCutoutIDs())
		removeCutoutID(id, false);

	for (Cutout* cut : m_cutouts) {
		if (cut->getSheetID() >= index) {
			cut->setSheetID(cut->getSheetID() - 1);
		}
	}


	m_spriteSheets.removeAt(index);
	emit numberOfSheetsChanged();
	emit spriteSheetRemoved(index);
	return true;
}
void SpriteData::resizeSpritesOnSheet(SpriteSheet* sheet, int oldWidth, int oldHeight, int targetWidth, int targetHeight) {
	const float xScale = targetWidth / (float)oldWidth;
	const float yScale = targetHeight / (float)oldHeight;
	//resize cutotus
	for (int i : sheet->getCutoutIDs()) {
		Cutout* c = m_cutouts[i];
		c->setPosition(c->getX() * xScale, c->getY() * yScale);
		c->setSize(c->getWidth() * xScale, c->getHeight() * yScale);
	}

	//resize keyframes
	for (SpriteAnimation* ani : m_animations)
		for (Layer* layer : ani->getLayers())
			for (Keyframe* key : layer->getKeyframes())
				for (int i : sheet->getCutoutIDs()) {
					if (i == key->getCutoutID()) {
						key->setScaleX(key->getScaleX() / xScale);
						key->setScaleY(key->getScaleY() / yScale);

						key->setAnchorX(key->getAnchorX() * xScale);
						key->setAnchorY(key->getAnchorY() * yScale);
						break;
					}
				}
}

bool SpriteData::editSpriteSheet(unsigned int index) {
	if (index >= m_spriteSheets.size()) return false;
	SpriteSheet* editTarget = m_spriteSheets[index];

	CreateEmptySpriteSheet create(editTarget->getWidth(), editTarget->getHeight(), editTarget->getPowerOfColorTable(), editTarget->getExternalID());
	create.exec();
	if (create.isAccepted()) {
		if (create.isDelete()) {
			if (index == 0) {
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error",
					"You can't delete the first sprite sheet!",
					QMessageBox::Ok);
				return false;
			}

			QMessageBox::StandardButton reply = QMessageBox::warning(nullptr, "Delete sprite sheet?",
				"Are you sure you want to delete the sprite sheet?",
				QMessageBox::Yes | QMessageBox::Cancel);

			if (reply == QMessageBox::No)
				return false;

			return removeSpriteSheet(index);

		}

		if (editTarget->isExternal() && create.getExternalID() < 0) { //make external to none external
			editTarget->makeNotExternal(create.getWidth(), create.getHeight(), create.getColorTablePower());
		}
		else if (create.getExternalID() >= 0) { // make external
			auto findIt = std::find_if(m_spriteSheets.begin(), m_spriteSheets.end(),
				[externalId = create.getExternalID()](SpriteSheet* sheet) { return sheet->getExternalID() == externalId; });
			if (findIt == m_spriteSheets.end()) {
				editTarget->makeExternal(create.getExternalID());
			}
			else {
				QMessageBox::warning(nullptr, "Duplicate sprite sheet",
					"External sprite sheet with the id " + QString::number(create.getExternalID()) + " is already present!"
					"\nIt is sprite sheet Nr. " + QString::number(std::distance(m_spriteSheets.begin(), findIt) + 1) + ".",
					QMessageBox::Ok);
				return false;
			}
			return true;
		}

		//color table too small?
		const int colorTableSetsToAdd = (pow(2, create.getColorTablePower()) / 16) - getNumberOfColortableSets();
		if (colorTableSetsToAdd > 0) addColors(-1, colorTableSetsToAdd * 16);

		const int oldWidth = editTarget->getWidth();
		const int oldHeight = editTarget->getHeight();
		editTarget->set(create.getWidth(), create.getHeight(), create.getColorTablePower(), true);
		emit spriteSheetChanged(index);

		if (create.getResizeSprites())
			resizeSpritesOnSheet(editTarget, oldWidth, oldHeight, create.getWidth(), create.getHeight());

		return true;
	}
	return false;
}

void SpriteData::renameCurrentAnimation() {
	if (m_currentAnimation < 0) return;
	bool ok = false;
	QString name = QInputDialog::getText(nullptr, tr("New animation name"),
		tr("New name:"), QLineEdit::Normal,
		m_animations[m_currentAnimation]->getName(),
		&ok);
	if (ok && !name.isEmpty())
		renameCurrentAnimation(name);
}

void SpriteData::renameCurrentAnimation(const QString& newName) {
	renameAnimation(newName, m_currentAnimation);
}

void SpriteData::renameAnimation(const QString& newName, unsigned int index) {
	if (index < 0 || index > m_animations.size()) return;
	m_animations[m_currentAnimation]->setName(newName);
}

// QAbstractListModel
QVariant SpriteData::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (index.row() >= getNumberOfAnimations())
		return QVariant();

	if (role == Qt::DisplayRole) {
		const SpriteAnimation* ani = m_animations.at(index.row());
		return QString::number(index.row()) + ": ID: " + QString::number(ani->getID()) + " Name: " + ani->getName() + " Duration: " + QString::number(ani->getTotalFrames());
	}
	else
		return QVariant();
}
int SpriteData::rowCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return 0;
	return getNumberOfAnimations();
}

QDebug operator<< (QDebug d, const PG::UTIL::ivec2& m) {
	d << "(" << m.x << ", " << m.y << ")";
	return d;
}

QDebug operator<< (QDebug d, const PG::UTIL::ivec3& m) {
	d << "(" << m.x << ", " << m.y << ", " << m.z << ")";
	return d;
}

