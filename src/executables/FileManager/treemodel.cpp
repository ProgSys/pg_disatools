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


#include "treemodel.h"

#include <iostream>
#include <QGraphicsPixmapItem>
#include <QFileInfo>
#include <QProgressDialog>
#include <QtConcurrent/QtConcurrentRun>

#include <Files/PG_ImageFiles.h>
#include <Files/PG_TX2.h>

#include <Files/PG_MPP.h>
#include <Files/PG_PSPFS.h>
#include <Files/PG_PSFS.h>
#include <Files/PG_StartDAT.h>
#include <Files/PG_SOLA.h>
#include <Files/PG_FileTests.h>
#include <Files/PG_IMY.h>
#include <Files/PG_NISPACK.h>
#include <Files/PG_DSARCFL.h>
#include <Files/PG_ANMD2.h>
#include <Files/PG_Script.h>
#include <Files/PG_SH.h>
#include <Files/PG_ARC.h>
#include <QCryptographicHash>

#include <EnterValue.h>
#include <Util/Misc/ResourcePath.h>
#include <Stream/PG_StreamInByteArray.h>

inline void openProgress(QProgressDialog& progress, const QString& title = "In progress") {
	progress.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint); // | Qt::WindowStaysOnTopHint
	progress.setWindowTitle("Please wait.");
	progress.setWindowModality(Qt::WindowModal);
	progress.setLabelText(title);
	progress.setCancelButton(0);
	progress.setRange(0, 101);
	progress.show();
}

TreeModel::TreeModel(QObject* parent)
	:QAbstractItemModel(parent), m_fileExtractor(nullptr) {

}

TreeModel::TreeModel(const QString& data, QObject* parent)
	: QAbstractItemModel(parent), m_fileExtractor(nullptr) {

	m_fileExtractor = nullptr;
	open(data);

}


bool TreeModel::open(const QString& file) {
	if (m_fileExtractor) delete m_fileExtractor;
	m_fileExtractor = nullptr;
	for (QTemporaryFile* temp : m_tempFiles)
		delete temp;
	m_tempFiles.clear();
	QAbstractItemModel::layoutAboutToBeChanged();

	QFileInfo fileInfo(file);
	const QString ext = fileInfo.suffix().toUpper();
	if (ext == "DAT") {
		m_openedFileType = "DAT";
		if (PG::FILE::isPSPFS(file.toStdString())) {
			m_fileExtractor = new PG::FILE::PSPFS();
		}else if (PG::FILE::isPSFS(file.toStdString())) {
			m_fileExtractor = new PG::FILE::PSFS();
		}
		else if (PG::FILE::isNISPACK(file.toStdString())) {
			m_fileExtractor = new PG::FILE::NISPACK();
		}
		else if (PG::FILE::isDSARC_FL(file.toStdString())) {
			m_fileExtractor = new PG::FILE::DSARC_FL();
		}
		else if (PG::FILE::isSpriteSheetPackage(file.toStdString())) {
			m_fileExtractor = new PG::FILE::SOLA();
		}
		else if (PG::FILE::isANMD2(file.toStdString())) {
			m_fileExtractor = new PG::FILE::ANMD2();
		}
		else if (PG::FILE::isScript(file.toStdString())) {
			m_fileExtractor = new PG::FILE::Script();
		}
		else if (PG::FILE::isDSARCIDX(file.toStdString())) {
			m_openedFileType = "ARC";
			m_fileExtractor = new PG::FILE::ARC();
		}
		else {
			m_fileExtractor = new PG::FILE::StartDAT();
		}
	}
	else if (ext == "PBD" && PG::FILE::isDSARC_FL(file.toStdString())) {
		m_openedFileType = "PBD";
		m_fileExtractor = new PG::FILE::DSARC_FL();
	}
	else if (ext == "MPP") {
		m_openedFileType = "MPP";
		m_fileExtractor = new PG::FILE::MPP();
	}
	else if (ext == "ARC") {
		m_openedFileType = "ARC";
		m_fileExtractor = new PG::FILE::ARC();
	}
	else {
		qInfo() << "Unknown file format: '" << ext << "' file: '" << file << "'!";
		return false;
	}

	if (!m_fileExtractor) return false;

	QProgressDialog progress;
	openProgress(progress, "Open in progress");

	QFuture<bool> f1 = QtConcurrent::run(m_fileExtractor, &PG::FILE::ExtractorBase::open, file.toStdString(), &m_percentIndicator);
	while (f1.isRunning()) {
		progress.setValue(m_percentIndicator.percent);
		progress.update();
	}

	bool result = f1.result();

	if (result) {
		qInfo() << "Couldn't open: '" << file << "'!";
		delete m_fileExtractor;
		m_fileExtractor = nullptr;
		QAbstractItemModel::layoutChanged();
		return false;
	}
	progress.close();

	QAbstractItemModel::layoutChanged();

	return true;
}

bool TreeModel::extractFileName(const QModelIndex& index, const QString& filepath) const {
	if (!m_fileExtractor || !index.isValid()) return true;
	PG::FILE::fileInfo* item = static_cast<PG::FILE::fileInfo*>(index.internalPointer());

	if (m_fileExtractor->extract(*item, filepath.toStdString())) {
		qInfo() << "Couldn't extract " << QString::fromStdString(item->name.getPath());
		return true;
	}
	return false;

}

bool TreeModel::extractFile(const QModelIndex& index, const QString& dir) const {
	if (!m_fileExtractor || !index.isValid()) return true;

	PG::FILE::fileInfo* item = static_cast<PG::FILE::fileInfo*>(index.internalPointer());
	const std::string targetFile = dir.toStdString() + "/" + item->name.getPath();
	if (m_fileExtractor->extract(*item, targetFile)) {
		qInfo() << "Couldn't extract " << QString::fromStdString(item->name.getPath());
		return true;
	}
	return false;
}

int TreeModel::extract(const QModelIndexList& indeces, const QString& dir) const {

	int extracted = 0;
	for (const QModelIndex& index : indeces) {
		PG::FILE::fileInfo* item = static_cast<PG::FILE::fileInfo*>(index.internalPointer());
		const std::string targetFile = dir.toStdString() + "/" + item->name.getPath();
		if (m_fileExtractor->extract(*item, targetFile)) {
			qInfo() << "Couldn't extract " << QString::fromStdString(item->name.getPath());
		}
		else
			extracted++;

		m_percentIndicator.percent = (extracted / float(indeces.size())) * 100;
	}

	return extracted;
}

bool TreeModel::addFile(const QString& file) {
	if (file.isEmpty()) return false;
	return add({ file });
}

int TreeModel::add(const QStringList& files) {
	if (!m_fileExtractor || files.empty()) return 0;
	QAbstractItemModel::layoutAboutToBeChanged();

	QProgressDialog progress;
	openProgress(progress, "Insert in progress");

	int filesAddedOrChanged = 0;
	if (getType() == "SOLA") { //sprite sheets also need a ID
		PG::FILE::SOLA* sola = static_cast<PG::FILE::SOLA*>(m_fileExtractor);
		if (!sola) {
			qInfo() << "SOLA cast failed!";
			return 0;
		}

		for (const QString& str : files) {
			if (true && sola->exists(str.toStdString())) {
				if (sola->insert(str.toStdString())) {
					qInfo() << __LINE__ << ": Couldn't add file: '" << str << "'";
				}
				else
					filesAddedOrChanged++;
			}
			else {
				//will be added to the back
				int id = 0;
				progress.hide();
				if (EnterValue::openEnterIntDialog(id, 1, 65535, "Please enter a ID", "Please enter a ID for the file:\n '" + QFileInfo(str).fileName() + "'")
					&& !sola->insert(str.toStdString(), id)
					) {
					filesAddedOrChanged++;
				}
				else
					qInfo() << __LINE__ << ": Couldn't add file: '" << str << "'";
				progress.show();
				m_percentIndicator.percent = (filesAddedOrChanged / float(files.size())) * 100;
				progress.setValue(m_percentIndicator.percent);
			}
		}
	}
	else //normal add
		for (const QString& str : files) {
			if (m_fileExtractor->insert(str.toStdString())) {
				qInfo() << __LINE__ << ": Couldn't add file: '" << str << "'";
			}
			else
				filesAddedOrChanged++;
			m_percentIndicator.percent = (filesAddedOrChanged / float(files.size())) * 100;
			progress.setValue(m_percentIndicator.percent);
		}

	progress.close();
	qInfo() << "Added Files/";
	QAbstractItemModel::layoutChanged();
	return filesAddedOrChanged;
}

bool TreeModel::replace(const QModelIndex& index, const QString& file, bool keepName) {
	if (!m_fileExtractor || file.isEmpty() || !index.isValid()) return false;

	PG::FILE::fileInfo* item = static_cast<PG::FILE::fileInfo*>(index.internalPointer());
	if (m_fileExtractor->replace(*item, file.toStdString(), keepName)) {
		qInfo() << __LINE__ << ": Couldn't remove file: '" << QString::fromStdString(item->name.getPath()) << "'";
		return true;
	}
	return false;
}

bool TreeModel::remove(const QModelIndex& index) {
	if (!m_fileExtractor || !index.isValid()) return true;

	QAbstractItemModel::layoutAboutToBeChanged();
	PG::FILE::fileInfo* item = static_cast<PG::FILE::fileInfo*>(index.internalPointer());
	if (m_fileExtractor->remove(m_fileExtractor->toIterator(item))) {
		qInfo() << __LINE__ << ": Couldn't remove file: '" << QString::fromStdString(item->name.getPath()) << "'";
		return true;
	}
	QAbstractItemModel::layoutChanged();

	return false;
}

unsigned int TreeModel::remove(QList<PG::FILE::fileInfo*>& indices) {
	if (!m_fileExtractor) return true;
	QAbstractItemModel::layoutAboutToBeChanged();
	qSort(indices.begin(), indices.end(), [](const PG::FILE::fileInfo* a, const PG::FILE::fileInfo* b) {
		return a > b;
		});

	unsigned int removed = 0;
	for (PG::FILE::fileInfo* index : indices) {
		if (!m_fileExtractor->remove(m_fileExtractor->toIterator(index)))
			removed++;
		m_percentIndicator.percent = (removed / float(indices.size())) * 100;
	}

	QAbstractItemModel::layoutChanged();
	return removed;
}

bool TreeModel::decompresIMYPack(const QModelIndex& index) {
	if (!m_fileExtractor || !index.isValid()) return false;
	const PG::FILE::fileInfo* item = static_cast<const PG::FILE::fileInfo*>(index.internalPointer());
	if (!item || !item->isCompressed() || !item->isPackage()) {
		qInfo() << __LINE__ << ": File isn't a IMY pack '" << QString::fromStdString(item->name.getPath()) << "'";
		return false;
	}

	QTemporaryFile* temp = new QTemporaryFile(QString::fromStdString(item->name.getPath()), this);
	if (!temp->open()) {
		delete temp;
		qInfo() << __LINE__ << ": Couldn't create temp file for IMY pack '" << QString::fromStdString(item->name.getPath()) << "'";
		return false;
	}
	temp->close();
	m_tempFiles.push_back(temp);

	char* c = nullptr;
	unsigned int sile_size = 0;
	if ((sile_size = m_fileExtractor->extract(*item, c)) == 0) {
		if (c) delete c;
		qInfo() << __LINE__ << ": Couldn't extract IMY pack: '" << QString::fromStdString(item->name.getPath()) << "'";
		return false;
	}

	const std::string fileName = temp->fileName().toStdString();
	PG::FILE::decompressIMYPackage(c, sile_size, fileName, &m_percentIndicator);

	return !replace(index, temp->fileName(), true);
}

bool TreeModel::decompresIMYPack(const QModelIndex& index, const QString& path, bool toFolder) const {
	if (!m_fileExtractor || !index.isValid() || path.isEmpty()) return false;
	const PG::FILE::fileInfo* item = static_cast<const PG::FILE::fileInfo*>(index.internalPointer());
	if (!item || !item->isCompressed() || !item->isPackage()) {
		qInfo() << __LINE__ << ": File isn't a IMY pack '" << QString::fromStdString(item->name.getPath()) << "'";
		return false;
	}

	char* c = nullptr;
	unsigned int file_size = 0;
	if ((file_size = m_fileExtractor->extract(*item, c)) == 0) {
		if (c) delete c;
		qInfo() << __LINE__ << ": Couldn't extract IMY pack: '" << QString::fromStdString(item->name.getPath()) << "'";
		return false;
	}

	if (toFolder)
		PG::FILE::decompressIMYPackage(c, file_size, (path + "/" + QString::fromStdString(item->name.getFile())).toStdString(), &m_percentIndicator);
	else
		PG::FILE::decompressIMYPackage(c, file_size, path.toStdString(), &m_percentIndicator);
	return true;
}

bool TreeModel::decompresIMY(const QModelIndex& index, const QString& path, bool toFolder) const {
	if (!m_fileExtractor || !index.isValid() || path.isEmpty()) return false;
	const PG::FILE::fileInfo* item = static_cast<const PG::FILE::fileInfo*>(index.internalPointer());
	if (!item || !item->isCompressed()) {
		qInfo() << __LINE__ << ": File isn't a IMY '" << QString::fromStdString(item->name.getPath()) << "'";
		return false;
	}


	char* c = nullptr;
	unsigned int file_size = 0;
	if ((file_size = m_fileExtractor->extract(*item, c)) == 0) {
		if (c) delete c;
		qInfo() << __LINE__ << ": Couldn't extract IMY: '" << QString::fromStdString(item->name.getPath()) << "'";
		return false;
	}
	if(toFolder)
		PG::FILE::decompressIMY(c, file_size, (path + "/" + QString::fromStdString(item->name.getFile())).toStdString(), &m_percentIndicator);
	else
		PG::FILE::decompressIMY(c, file_size, path.toStdString(), &m_percentIndicator);

	return true;
}


bool TreeModel::hasDataChanged() const {
	return m_fileExtractor->isChanged();
}

bool TreeModel::save() {
	if (!m_fileExtractor) return false;

	QAbstractItemModel::layoutAboutToBeChanged();
	PG::UTIL::File file = m_fileExtractor->getOpendFile();
	if (m_fileExtractor->save(&m_percentIndicator)) {
		QAbstractItemModel::layoutChanged();
		return false;
	}
	for (QTemporaryFile* temp : m_tempFiles)
		delete temp;
	m_tempFiles.clear();
	//bool success = open(QString::fromStdString(file.getPath()));
	QAbstractItemModel::layoutChanged();
	return true;
}

bool TreeModel::saveAs(const QString& filepath) {
	if (!m_fileExtractor) return false;
	QAbstractItemModel::layoutAboutToBeChanged();
	if (!m_fileExtractor->save(filepath.toStdString(), &m_percentIndicator)) {
		m_fileExtractor->open(filepath.toStdString(), &m_percentIndicator);
		QAbstractItemModel::layoutChanged();
		return true;
	}
	else {
		//if saving faled try to reopen it
		PG::UTIL::File file = m_fileExtractor->getOpendFile();
		const bool success = open(QString::fromStdString(file.getPath()));
		QAbstractItemModel::layoutChanged();
		return success;
	}

	return false;
}

bool TreeModel::getImage(const QModelIndex& index, PG::UTIL::RGBAImage& imageOut, bool alpha) const {
	if (!m_fileExtractor) return false;

	const PG::FILE::fileInfo* item = static_cast<const PG::FILE::fileInfo*>(index.internalPointer());
	std::string fileExt = item->name.getFileExtension();
	std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);

	if (fileExt != "tx2" && fileExt != "txp" && fileExt != "sh") {
		qInfo() << "File preview not supported for:'" << QString::fromStdString(item->name.getPath()) << "'";
		return true;
	}

	char* c = nullptr;
	unsigned int sile_size = 0;
	if ((sile_size = m_fileExtractor->extract(*item, c)) == 0) {
		if (c) delete[] c;
		qInfo() << "Couldn't extract image file: '" << QString::fromStdString(item->name.getPath()) << "'";
		return true;
	}

	if (fileExt == "tx2" || fileExt == "txp") {
		if (PG::FILE::decompressTX2(c, sile_size, imageOut)) {
			qInfo() << "Couldn't decompress TX2 image file: '" << QString::fromStdString(item->name.getPath()) << "'";
			if (c) delete[] c;
			return true;
		}
		if (c) delete[] c;
		c = nullptr;

		if (imageOut.getWidth() == 0 || imageOut.getHeight() == 0) {
			qInfo() << "Image has width or height is zero!";
			return true;
		}

	}
	else if (fileExt == "sh") {
		PG::FILE::SH sh;
		PG::STREAM::InByteArray reader(c, sile_size);
		sh.open(&reader);

		if (c) delete[] c;
		c = nullptr;

		if (sh.getSpriteSheets().empty() || sh.getSheetsInfos().empty() || sh.getColortables().empty()) {
			qInfo() << "SH file has no sprite sheets: '" << QString::fromStdString(item->name.getPath()) << "'";
			return true;
		}

		const PG::UTIL::IDImage& idImage = sh.getSpriteSheets().front();

		if (idImage.getWidth() <= 0 || idImage.getHeight() <= 0 || idImage.empty()) {
			qInfo() << "Image has width or height is zero!";
			return true;
		}

		imageOut.resize(idImage.getWidth(), idImage.getHeight());
		auto itID = idImage.begin();
		auto itColor = imageOut.begin();
		assert(idImage.size() == imageOut.size());
		for (; itID != idImage.end(); ++itID, ++itColor) {
			if (*itID < sh.getColortables().front().size())
				*itColor = sh.getColortables().front()[*itID];
			else
				*itColor = sh.getColortables().front().front();
		}
	}
	else {
		qInfo() << "File preview not supported for: '" << QString::fromStdString(item->name.getPath()) << "'";
		if (c) delete[] c;
		return true;
	}

	if (!alpha) {
		for (PG::UTIL::rgba& rgba : imageOut) {
			rgba.a = 255;
		}
	}

	if (c) delete[] c;
	return false;
}

bool TreeModel::setGraphicsScene(const QModelIndex& index, QGraphicsScene* scene) const {
	if (!m_fileExtractor) return false;

	PG::UTIL::RGBAImage img;
	if (getImage(index, img, false) || img.empty()) {
		qInfo() << "Couldn't open image file!";
		return true;
	}

	QImage qimg((unsigned char*)img.data(), img.getWidth(), img.getHeight(), QImage::Format_RGBA8888);

	QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qimg));
	scene->clear();
	scene->addItem(item);

	return false;



}

QVariant TreeModel::data(const QModelIndex& index, int role) const {
	if (!m_fileExtractor || !index.isValid())
		return QVariant();


	const PG::FILE::fileInfo* item = static_cast<const PG::FILE::fileInfo*>(index.internalPointer());
	if (item == nullptr) return QVariant();

	QString exp = QString::fromStdString(item->getFileExtension());

	if (role == Qt::DecorationRole && index.column() == 0) {
		if (item->isCompressed())
			if (item->isPackage())
				return QPixmap(getResourcePath() + "/materials/icons/compress.png");
			else
				return QPixmap(getResourcePath() + "/materials/icons/compress_file.png");
		else if (item->isPackage())
			return QPixmap(getResourcePath() + "/materials/icons/archive.png");
		else if (exp == "PNG" || exp == "TX2")
			return QPixmap(getResourcePath() + "/materials/icons/image.png");
		else if (exp == "OGG")
			return QPixmap(getResourcePath() + "/materials/icons/note.png");
		else if (exp == "GEO" || exp == "MPP")
			return QPixmap(getResourcePath() + "/materials/icons/geometry.png");
		else if (item->fileType == PG::FILE::fileInfo::SH)
			return QPixmap(getResourcePath() + "/materials/icons/sprite.png");
		return QVariant();
	}
	else if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case 0:
		{
			if (item->isExternalFile())
				return QVariant(QString::fromStdString(item->name.getPath()).append('*'));
			else
				return QVariant(QString::fromStdString(item->name.getPath()));
		}
		break;
		case 1:
			return QVariant(item->size);
			break;
		case 2:
			return QVariant(exp);
			break;
		case 3:
			return QString("%1").arg(index.row(), 5, 10, QChar('0')); //QVariant(QString::number(item->index));
			break;
		default:
			return QVariant();
			break;
		}
	}
	else
		return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
	if (!index.isValid())
		return 0;

	return QAbstractItemModel::flags(index);
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
		case 0:
			return QVariant(QString("File"));
			break;
		case 1:
			return QVariant(QString("Size"));
			break;
		case 2:
			return QVariant(QString("Format"));
			break;
		case 3:
			return QVariant(QString("Index"));
			break;
		default:
			return QVariant();
			break;
		}
	}
	// return rootItem->data(section);

	return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
	if (!m_fileExtractor || !hasIndex(row, column, parent))
		return QModelIndex();

	if (row >= m_fileExtractor->size())
		return QModelIndex();

	PG::FILE::fileInfo* info = m_fileExtractor->getDataPointer(row);

	return createIndex(row, column, info); //QModelIndex( file );

}

QModelIndex TreeModel::parent(const QModelIndex& index) const {
	if (!index.isValid())
		return QModelIndex();

	return QModelIndex();

}

int TreeModel::rowCount(const QModelIndex& parent) const {
	if (!m_fileExtractor || parent.isValid())
		return 0;

	return m_fileExtractor->size();
}

bool TreeModel::saveImage(const QModelIndex& index, const QString& targetfile) {
	if (!m_fileExtractor) return false;
	PG::UTIL::RGBAImage img;

	if (getImage(index, img, true) || img.empty()) {
		qInfo() << "Couldn't extract image file.";
		return false;
	}

	qInfo() << " Trying to export image to '" << targetfile << "'.";

	QFileInfo fInfo(targetfile);
	QString ext = fInfo.suffix();
	if (ext == "tga") {
		return PG::FILE::saveTGA(targetfile.toStdString(), img);
	}
	else if (ext == "pgm" || ext == "pnm") {
		return PG::FILE::saveNetPNM(targetfile.toStdString(), img);
	}
	else {
		QImage qimg((unsigned char*)img.data(), img.getWidth(), img.getHeight(), QImage::Format_RGBA8888);
		return qimg.save(targetfile, 0, 100);
	}


}

bool TreeModel::checkIsValid(QString& outMessage) const {
	if (m_fileExtractor) {
		outMessage.clear();
		outMessage = m_fileExtractor->getError();
		return outMessage.isEmpty();
	}
	else {
		outMessage = "There is no file opened!";
		return false;
	}
}

bool TreeModel::isOpen() const {
	return m_fileExtractor && !m_fileExtractor->isEmpty();
}

QString TreeModel::getOpenedFileName() const {
	if (m_fileExtractor) {
		return QString::fromStdString(m_fileExtractor->getOpendFile().getName());
	}
	else {
		return "";
	}
}

const QString& TreeModel::getOpenedType() const {
	return m_openedFileType;
}

QString TreeModel::getType() const {
	if (m_fileExtractor) {
		return m_fileExtractor->getType();
	}
	else
		return "";
}


bool TreeModel::writeManifest(const QString& file) {
	if (!m_fileExtractor || file.isEmpty()) return false;
	QFile out(file);
	m_percentIndicator.percent = 0.0f;

	if (out.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream stream(&out);
		stream << left << qSetFieldWidth(6) << "Index" << qSetFieldWidth(0) << ";" << qSetFieldWidth(16) << "Offset" << qSetFieldWidth(0) << ";";
		stream << qSetFieldWidth(16) << "Size" << qSetFieldWidth(0) << ";" << qSetFieldWidth(32) << "Md5" << qSetFieldWidth(0) << ";" << "Name" << "\n";

		for (int i = 0; i < m_fileExtractor->size(); ++i) {
			m_percentIndicator.percent = i / (float)m_fileExtractor->size();
			const  PG::FILE::fileInfo& info = m_fileExtractor->get(i);
			stream << qSetFieldWidth(6) << QString::number(i) << qSetFieldWidth(0) << ";" << qSetFieldWidth(16) << info.getOffset() << qSetFieldWidth(0) << ";";
			stream << qSetFieldWidth(16) << info.getSize() << qSetFieldWidth(0) << ";";
			char* data = nullptr;
			m_fileExtractor->extract(info, data);
			if (data) {
				QCryptographicHash hasher(QCryptographicHash::Md5);
				hasher.addData(data, info.getSize());
				delete[] data;
				stream << qSetFieldWidth(32) << hasher.result().toHex(0);
			}
			else {
				stream << qSetFieldWidth(32) << 0;
			}
			stream << qSetFieldWidth(0) << ";" << QString::fromStdString(info.getName().getPath());
			if (info.isExternalFile())
				stream << "*";
			stream << "\n";
		}
	}
	out.close();

	return true;
}

int TreeModel::columnCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return 0;
	return 4;

}

float TreeModel::getProgress() const {
	return m_percentIndicator.percent;
}

PG::FILE::fileProperties TreeModel::getFileProperties(PG::FILE::fileInfo* item) const {
	if (m_fileExtractor) return m_fileExtractor->getFileProperties(*item);
	return PG::FILE::fileProperties("");
}


TreeModel::~TreeModel() {
	if (m_fileExtractor) delete m_fileExtractor;
	for (QTemporaryFile* temp : m_tempFiles)
		delete temp;
}


