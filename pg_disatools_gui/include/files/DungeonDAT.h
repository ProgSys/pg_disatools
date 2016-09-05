/*
 * DungeonDAT.h
 *
 *  Created on: 04.09.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_FILES_DUNGEONDAT_H_
#define INCLUDE_FILES_DUNGEONDAT_H_

#include <files/DataFile.h>

class DungeonDAT: public DataFile {
	Q_OBJECT
public:
	DungeonDAT(QObject *parent = 0);
	virtual ~DungeonDAT();

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

public slots:
	bool open(const QString& filepath) final;
	bool save(const QString& filepath) final;

};

#endif /* INCLUDE_FILES_DUNGEONDAT_H_ */
