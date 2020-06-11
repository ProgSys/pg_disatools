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


#pragma once

#include <QAbstractItemModel>
#include <QtCore>
#include <QGraphicsScene>

#include <Files/PG_ExtractorBase.h>
#include <string>
#include <vector>
#include <Util/PG_Image.h>

class TreeModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit TreeModel(QObject *parent = 0);
    explicit TreeModel(const QString &file, QObject *parent = 0);



    bool remove(const QModelIndex& index );
    unsigned int remove(QList<PG::FILE::fileInfo*>& index);

    bool hasDataChanged() const;

    bool setGraphicsScene(const QModelIndex& index, QGraphicsScene* scene) const;

    //void hideFiles/(const QString &extention, bool hide);

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    float getProgress() const;
    PG::FILE::fileProperties getFileProperties(PG::FILE::fileInfo *item) const;

    virtual ~TreeModel();
public slots:
	bool open(const QString &file);
    bool save();
    bool saveAs(const QString& filepath);
    bool saveImage(const QModelIndex& index, const QString& targetfile);

    bool addFile(const QString &file);
    int add(const QStringList & files);

    bool replace(const QModelIndex& index, const QString &file, bool keepName = false);

    bool extractFileName(const QModelIndex& index, const QString &filepath) const;
    bool extractFile(const QModelIndex& index, const QString &dir) const;
    int extract(const QModelIndexList& indeces, const QString &dir) const;

    bool decompresIMYPack(const QModelIndex& index); //decompres IMY pack and add it back in into the archive
    /*!
     * @para toFolder if true then the given path is not a filepath but a directory.
     */
    bool decompresIMYPack(const QModelIndex& index, const QString &path, bool toFolder = false) const; //save the decompress IMY pack as a seperate file

    bool checkIsValid(QString& outMessage) const;

    bool isOpen() const;
    QString getOpenedFileName() const;
    const QString& getOpenedType() const;

    QString getType() const;

    bool writeManifest(const QString& file);
private:

    PG::FILE::ExtractorBase* m_fileExtractor;
    mutable PercentIndicator m_percentIndicator;
    QString m_openedFileType;
    QList<QTemporaryFile*> m_tempFiles;

    std::vector<std::string> m_hideExtentions;


    bool getImage(const QModelIndex& index, PG::UTIL::RGBAImage& imageOut, bool alpha = true) const;

};

