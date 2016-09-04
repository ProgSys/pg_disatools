/*
 * DataFile.cpp
 *
 *  Created on: 04.09.2016
 *      Author: ProgSys
 */

#include <files/DataFile.h>

DataFile::DataFile(QObject *parent): QAbstractListModel(parent) {
	// TODO Auto-generated constructor stub

}

DataFile::~DataFile() {
	// TODO Auto-generated destructor stub
}

bool DataFile::open(const QString& filepath){ return false;}
bool DataFile::save(const QString& filepath){ return false;}
