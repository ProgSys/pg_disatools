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
#include <Marker.h>
#include <Util/PG_Exception.h>

////// MARKER //////

Marker::Marker(QObject *parent): QObject(parent)
{}

Marker::Marker(int start, int duration, short x, short y, QObject *parent): QObject(parent),
		m_start(start), m_duration(duration), m_global_x(x), m_global_y(y)
		{
		if(m_start < 0) m_start = 0;
		if(m_duration < 1) m_duration = 1;
		}

Marker::Marker(int start, int duration, unsigned int typeValue, QObject *parent ): QObject(parent),
	m_start(start), m_duration(duration), m_global_x(0), m_global_y(0)
{
	setTypeValue(typeValue, true);
}

Marker::Marker(int start, int duration,  unsigned int typeValue, short x, short y, QObject *parent): QObject(parent),
		m_start(start), m_duration(duration), m_global_x(x), m_global_y(y)
{
	setTypeValue(typeValue, true);
}

Marker::Marker(const Marker& marker):
		QObject(marker.parent()), m_start(marker.getStart()), m_duration(marker.getDuration()), m_global_x(marker.getX()), m_global_y(marker.getY()), m_type(marker.getTypeBits())
{}

Marker::~Marker(){

}

void Marker::operator =(const Marker& marker){
	setParent(marker.parent());

	setStart(marker.getStart());
	setDuration(marker.getDuration());
	setX(marker.getX());
	setY(marker.getY());
	setTypeBits(marker.getTypeBits());

}

//getters
int Marker::getStart() const{
	return m_start;
}
int Marker::getDuration() const{
	return m_duration;
}
short Marker::getX() const{
	return m_global_x;
}
short Marker::getY() const{
	return m_global_y;
}

unsigned int Marker::getTypeBits() const{
	return m_type;
}

std::vector<unsigned int> Marker::getTypes() const {
	if (m_type == 0) return { 0 };
	std::vector<unsigned int> types;
	unsigned int bit = KnownTypeMask_LoopStart;
	for (int i = 1; i < 4; ++i, bit <<= 1) {
		if (m_type & bit) types.push_back(i);
	}
	return types;
}

//setters
void Marker::setStart(int start){
	//if(m_start == start) return;
	if(start < 0)
		m_start = 0;
	else
		m_start = start;
	emit onStartChanged();
}
void Marker::setDuration(int duration){
	if(m_duration == duration) return;
	if(duration < 1)
		m_duration = 1;
	else
		m_duration = duration;
	emit onDirationChanged();
}
void Marker::setX(short x){
	if(m_global_x == x) return;
	m_global_x = x;
	emit onXChanged();
}
void Marker::setY(short y){
	if(m_global_y == y) return;
	m_global_y = y;
	emit onYChanged();
}

void Marker::setTypeBits(unsigned int type){
	if(m_type == type) return;
	m_type = type;
	emit onTypeChanged();
}

void Marker::setTypeValue(unsigned int type, bool enabled) {
	auto mask = 1U << type;
	if (bool(m_type & mask) == enabled) return; //already set
	m_type ^= mask; //flip bit
	emit onTypeChanged();
}


bool MarkersList::remove(Marker* mark){
	const int indexof = m_list.indexOf(mark);
	if(indexof != -1){
		beginRemoveRows(QModelIndex(), indexof, indexof);
		m_list.removeAt(indexof);
		endRemoveRows();
		delete mark;
		emit sizeChanged();
		return true;
	}
	return false;
}

bool MarkersList::add(Marker* mark){
	assert_Test("Marker is already inside!", m_list.contains(mark));
	if(mark){
		beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
		m_list.push_back(mark);
		endInsertRows();
	}
	return false;
}

int MarkersList::findNextFreeframe(int startFrame) const{
	if(startFrame < 0) startFrame = 0;

	while(true){
		bool exit = true;
		for(const Marker* mark: m_list){
			if(startFrame >= mark->getStart() && startFrame < (mark->getStart()+mark->getDuration())){
				startFrame +=  mark->getDuration();
				exit = false;
			}
		}
		if(exit) return startFrame;
	}

	return startFrame;
}


