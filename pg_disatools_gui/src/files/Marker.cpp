/*
 * Marker.cpp
 *
 *  Created on: 23.08.2016
 *      Author: ProgSys
 */

#include <files/Marker.h>
#include <pg/util/PG_Exception.h>


////// MARKER //////

Marker::Marker(QObject *parent): QObject(parent)
{}

Marker::Marker(int start, int duration, short A, unsigned short B, QObject *parent): QObject(parent),
		m_start(start), m_duration(duration), m_A(A), m_B(B)
		{
		if(m_start < 0) m_start = 0;
		if(m_duration < 1) m_duration = 1;
		}

Marker::Marker(int start, int duration, unsigned int type, QObject *parent ): QObject(parent),
	m_start(start), m_duration(duration), m_A(0), m_B(0), m_type(type)
{}

Marker::Marker(int start, int duration,  unsigned int type, short A,unsigned short B, QObject *parent): QObject(parent),
		m_start(start), m_duration(duration), m_A(A), m_B(B), m_type(type)
{}

Marker::Marker(const Marker& marker):
		QObject(marker.parent()), m_start(marker.getStart()), m_duration(marker.getDuration()), m_A(marker.getA()), m_B(marker.getB()), m_type(marker.getType())
{}

Marker::~Marker(){

}

void Marker::operator =(const Marker& marker){
	setParent(marker.parent());

	setStart(marker.getStart());
	setDuration(marker.getDuration());
	setA(marker.getA());
	setB(marker.getB());
	setType(marker.getType());

}

//getters
int Marker::getStart() const{
	return m_start;
}
int Marker::getDuration() const{
	return m_duration;
}
short Marker::getA() const{
	return m_A;
}
unsigned short Marker::getB() const{
	return m_B;
}

unsigned int Marker::getType() const{
	return m_type;
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
void Marker::setA(short a){
	if(m_A == a) return;
	m_A = a;
	emit onAChanged();
}
void Marker::setB(unsigned short b){
	if(m_B == b) return;
	m_B = b;
	emit onBChanged();
}

void Marker::setType(unsigned int type){
	if(m_type == type) return;
	m_type = type;
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
	return false;;
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


