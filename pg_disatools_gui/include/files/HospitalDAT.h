/*
 * HospitalDAT.h
 *
 *  Created on: 05.09.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_FILES_HOSPITALDAT_H_
#define INCLUDE_FILES_HOSPITALDAT_H_

#include <files/DataFile.h>
#include <QList>
#include <util/TreeItem.h>

class HospitalDAT: public DataFile  {
	Q_OBJECT
public:
	HospitalDAT(QObject *parent = 0);
	virtual ~HospitalDAT();

	Qt::ItemFlags flags(const QModelIndex &index) const final;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) final;

public slots:
	bool open(const QString& filepath) final;
	bool save(const QString& filepath) final;

	bool insertFront() final;
	bool insertBack() final;
	bool insertAt(int index) final;
	bool removeAt(int index) final;

};

#endif /* INCLUDE_FILES_HOSPITALDAT_H_ */
