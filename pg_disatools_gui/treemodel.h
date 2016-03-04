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


#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QtCore>
#include <QGraphicsScene>

#include <pg/files/PG_PSPFS.h>
#include <string>
#include <vector>


class TreeModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit TreeModel(QObject *parent = 0);
    explicit TreeModel(const QString &file, QObject *parent = 0);
    ~TreeModel();

    void openFile(const QString &file);
    void extractFile(const QString &file, const QString &dir) const;
    bool addFile(const QString &file);
    int addFiles(const QStringList &files);
    int removeFiles(const QStringList &files);
    bool hasDataChanged() const;
    bool save();
    bool setGraphicsScene(const QString &file, QGraphicsScene* scene) const;

    void hideFiles(const QString &extention, bool hide);

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

public slots:
    bool saveFile();
    bool saveFileAs(const QString& filepath);

private:

    PG::FILE::PSPFS m_pspfsFile;

    std::vector<std::string> m_hideExtentions;


};

#endif // TREEMODEL_H
