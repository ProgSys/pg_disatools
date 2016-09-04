/*
 * DungeonDAT.h
 *
 *  Created on: 04.09.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_FILES_DUNGEONDAT_H_
#define INCLUDE_FILES_DUNGEONDAT_H_

#include <files/DataFile.h>
#include <QList>
#include <files/TreeItem.h>



class DungeonDAT: public DataFile {
	Q_OBJECT
public:
	DungeonDAT(QObject *parent = 0);
	virtual ~DungeonDAT();

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

public slots:
	bool open(const QString& filepath) final;
	bool save(const QString& filepath) final;
private:
	TreeItem* m_root;
};

#endif /* INCLUDE_FILES_DUNGEONDAT_H_ */
