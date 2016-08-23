/*
 * QMLQList.h
 *
 *  Created on: 23.08.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_FILES_MARKER_H_
#define INCLUDE_FILES_MARKER_H_

#include <QObject>
#include <QList>
#include <QAbstractListModel>

class Marker: public QObject{
	Q_OBJECT
	Q_PROPERTY(unsigned int start READ getStart WRITE setStart NOTIFY onStartChanged)
	Q_PROPERTY(unsigned int duration READ getDuration WRITE setDuration NOTIFY onDirationChanged)
	Q_PROPERTY(unsigned int a READ getA WRITE setA NOTIFY onAChanged)
	Q_PROPERTY(unsigned int b READ getB WRITE setB NOTIFY onBChanged)
	Q_PROPERTY(unsigned int type READ getType WRITE setType NOTIFY onTypeChanged)
public:
	Marker(QObject *parent = 0);
	Marker(int start, int duration, short A,unsigned short B, QObject *parent = 0);
	Marker(int start, int duration, unsigned int type, QObject *parent = 0);
	Marker(int start, int duration, unsigned int type, short A,unsigned short B,  QObject *parent = 0);
	Marker(const Marker& marker);
	virtual ~Marker();

	void operator =(const Marker& marker);

	//getters
	int getStart() const;
	int getDuration() const;
	short getA() const;
	unsigned short getB() const;
	unsigned int getType() const;

	//setters
	void setStart(int start);
	void setDuration(int duration);
	void setA(short a);
	void setB(unsigned short b);
	void setType(unsigned int type);

signals:
	void onStartChanged();
	void onDirationChanged();

	void onAChanged();
	void onBChanged();

	void onTypeChanged();
private:
	int m_start = 0;
	int m_duration = 1;
	short m_B = 0;
	short m_A = 0;
	unsigned int m_type = 0;
};

Q_DECLARE_METATYPE( Marker );
Q_DECLARE_METATYPE( Marker* );

class MarkersList: public QAbstractListModel {
	Q_OBJECT
public:
	MarkersList(QObject *parent = 0):  QAbstractListModel(parent){}
	MarkersList(const MarkersList& list):  QAbstractListModel(list.parent()){
		for(const Marker* t: list.getList())
			push_back(new Marker(*t));

	}
	virtual ~MarkersList() {
		for(Marker* t: m_list)
			delete t;
	}

	QList<Marker*>& getList(){
		return m_list;
	}
	const QList<Marker*>& getList() const{
		return m_list;
	}

	bool remove(Marker* mark);
	bool add(Marker* mark);
	int findNextFreeframe(int startFrame = 0) const;

	void operator= (const MarkersList& list){
		QAbstractListModel::setParent(list.parent());
		for(const Marker* t: list.getList())
			push_back(new Marker(*t));
	}

	void push_back(Marker* item){
		beginInsertRows(index(0), m_list.size(), m_list.size()+1);
		m_list.push_back(item);
		endInsertRows();
		emit sizeChanged();
	}


	bool empty() const{
		return m_list.isEmpty();
	}

	int size() const{
		return m_list.size();
	}

	bool contains(const Marker* const item){
		for(const Marker* mark: m_list)
			if(mark == item)
				return true;
		return false;
	}


	bool contains(Marker* const item){
		return m_list.contains(item);
	}

	Marker* operator[](int index){
		return m_list[index];
	}

	const Marker* operator[](int index) const{
		return m_list[index];
	}

	//QAbstractListModel
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const final{
		   if (!index.isValid())
		        return QVariant();

		    if (index.row() >= m_list.size())
		        return QVariant();

		    if (role == Qt::DisplayRole){
		    	return QVariant::fromValue(m_list.at(index.row()));
		    }else
		        return QVariant();
	}
	virtual int rowCount(const QModelIndex & parent = QModelIndex()) const final{
	    if(parent.isValid())
	        return 0;
		return m_list.size();
	}
signals:
	void sizeChanged();
private:
	QList<Marker*> m_list;
};

Q_DECLARE_METATYPE( MarkersList);
Q_DECLARE_METATYPE( MarkersList* );

#endif /* INCLUDE_FILES_MARKER_H_ */
