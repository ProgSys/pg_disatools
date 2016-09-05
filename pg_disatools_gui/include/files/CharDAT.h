/*
 * CharDAT.h
 *
 *  Created on: 05.09.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_FILES_CHARDAT_H_
#define INCLUDE_FILES_CHARDAT_H_

#include <files/DataFile.h>

class CharDAT: public DataFile {
	Q_OBJECT
public:
	CharDAT(QObject *parent = 0);
	virtual ~CharDAT();

bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

public slots:
	bool open(const QString& filepath) final;
	bool save(const QString& filepath) final;
};

#endif /* INCLUDE_FILES_CHARDAT_H_ */
