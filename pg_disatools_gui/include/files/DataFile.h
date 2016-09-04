/*
 * DataFile.h
 *
 *  Created on: 04.09.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_FILES_DATAFILE_H_
#define INCLUDE_FILES_DATAFILE_H_

#include <QAbstractListModel>
#include <QString>

class DataFile : public QAbstractListModel {
	Q_OBJECT
public:
	DataFile(QObject *parent = 0);
	virtual ~DataFile();
public slots:
	virtual bool open(const QString& filepath);
	virtual bool save(const QString& filepath);
};

#endif /* INCLUDE_FILES_DATAFILE_H_ */
