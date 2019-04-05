/*
 * DataFile.h
 *
 *  Created on: 04.09.2016
 *      Author: ProgSys
 */

#pragma once

#include <QAbstractListModel>
#include <QString>
#include <Util/Misc/TreeItem.h>

class DataFile : public QAbstractListModel {
	Q_OBJECT
public:
	DataFile(QObject *parent = 0);
	virtual ~DataFile();
	virtual int getColumnWidth(int index) const;

	virtual QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
	virtual  QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	virtual QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	virtual QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

public slots:
	virtual bool open(const QString& filepath);
	virtual bool save(const QString& filepath);

	virtual bool exportCSV(const QString& filepath) const;

	virtual bool insertFront();
	virtual bool insertBack();
	virtual bool insertAt(int index);
	virtual bool removeAt(int index);
protected:

	TreeItem* m_root = nullptr;
};