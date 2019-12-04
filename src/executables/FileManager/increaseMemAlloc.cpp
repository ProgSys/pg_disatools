/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2019  ProgSys
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
#include "increaseMemAlloc.h"
#include "ui_increaseMemAlloc.h"
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QFont>
#include <QByteArray>
#include <qmessagebox.h>
#include <Util/Misc/ResourcePath.h>
#include <Stream/PG_StreamInByteFile.h>

IncreaseMemAlloc::IncreaseMemAlloc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IncreaseMemAlloc)
{
    ui->setupUi(this);
	setWindowIcon(QIcon(":/disa_tools_icon.ico"));
	setWindowTitle("Increase memory allocation");

	if (!loadSchema()) {
		QMessageBox::critical(this, "Failed to read schema file!", "Failed to open 'allocMemSearch.txt' from resource folder. Try to reinstall.", QMessageBox::Close);
		close();
	}

	QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	fixedFont.setStyleHint(QFont::TypeWriter);
	ui->preview->setFont(fixedFont);

	connect(ui->btnOpenExe, &QPushButton::clicked, [this]() {
		QFileDialog openDialog(this);
		openDialog.setNameFilter(tr("Executable (*.exe);;"));

		if (openDialog.exec()) {
			QStringList fileNames = openDialog.selectedFiles();
			if (fileNames.empty()) return;
			ui->pathEdit->setText(fileNames.front());
			locate(ui->pathEdit->text().trimmed());
		}
	});

	connect(ui->pathEdit, &QLineEdit::editingFinished, [this]() {
		locate(ui->pathEdit->text().trimmed());
	});

	connect(ui->spMemSize, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int value) {
		if (m_schema.empty()) return;
		QString previewText(createReplacementData(value).toHex());

		int startAddress = m_startAddress;
		for (int i = 0; i < previewText.size(); ++i) {
			QString insertStr = QString::number(startAddress) + ": ";
			startAddress += 16;
			previewText.insert(i, insertStr);
			i += insertStr.size() + 32;
			previewText.insert(i, "\n");
		}
		
		ui->preview->setPlainText(previewText);

		ui->infoLabel->setText("bytes (" + QString::number(std::roundf(value / 10000.f) / 100.0f) + " MB)");
	});

	connect(ui->btnOverride, &QPushButton::clicked, this, [this]() {
		if (m_schema.empty() || m_fileData.size() < 10 || m_startAddress < 10 || m_startAddress >= m_fileData.size())
			return;

		QFileInfo exe(ui->pathEdit->text().trimmed());
		if (exe.completeSuffix() != "exe" || exe.size() != m_fileData.size()) {
			QMessageBox::critical(this, "Failed to modify", "The target file was changed after it was opened?", QMessageBox::Close);
			close();
			return;
		}
		save(exe.absoluteFilePath());
	});

	connect(ui->btnSaveAs, &QPushButton::clicked, this, [this]() {
		if (m_schema.empty() || m_fileData.size() < 10 || m_startAddress < 10 || m_startAddress >= m_fileData.size())
			return;

		QString path = QFileDialog::getSaveFileName(this, tr("Save File"),
			ui->pathEdit->text(), "Executable(*.exe)");
		if (path.isEmpty())
			return;
		save(path);
		});
	
	locate("");//set text
}

IncreaseMemAlloc::~IncreaseMemAlloc()
{
    delete ui;
}

bool IncreaseMemAlloc::loadSchema() {
	const QString pathToSchema = getResourcePath() + "/allocMemSearch.txt";
	QFile schema(pathToSchema);
	if (schema.open(QIODevice::ReadOnly)) {
		QByteArray bytes = schema.readAll();
		if (bytes.size() < 5)
			return false;

		if (bytes.front() == '#' || bytes.back() == '#')
			return false;

		m_schema = bytes.split('#');
		if(m_schema.empty())
			return false;

		for (QByteArray& data : m_schema) {
			data = QByteArray::fromHex(data);
			if (data.isEmpty()) {
				m_schema.clear();
				return false;
			}
		}

		return true;
	}
	else return false;
}

int IncreaseMemAlloc::getReplacementDataSize() const {
	int size = 0;
	for (auto it = m_schema.begin(); it != std::prev(m_schema.end()); ++it) {
		size += it->size() + 4;
	}
	size += m_schema.back().size();
	return size;
}

void IncreaseMemAlloc::locate(const QString& path) {
	if (path.isEmpty()) {
		fail("Please locate Disgaea PC exe. (\"../SteamApps/common/Disgaea PC\")");
		return;
	}

	QFileInfo exe(path);
	if (exe.completeSuffix() != "exe" || !exe.exists()) {
		fail("File is not an .exe or doesn't exist !");
		return;
	}

	QFile file(path);
	if (file.open(QIODevice::ReadOnly)) {
		m_fileData = file.readAll();

		//compare to schema
		auto compare = [this](QByteArray::iterator& it)->bool {
				for (const QByteArray& schema : m_schema) {
					for (auto compareIt = schema.begin(); compareIt != schema.end(); ++compareIt, ++it) {
						if (*compareIt != *it)
							return false;
					}

					it += 4; //skip 4 bytes
				}
				return true;
		};

		m_startAddress = 0;
		auto endIt = std::prev(m_fileData.end(), getReplacementDataSize());
		for (auto it = m_fileData.begin(); it != m_fileData.end(); ++it) {
			auto beforeComp = it;
			if (compare(it)) {
				m_startAddress = std::distance(m_fileData.begin(), beforeComp);
				break;
			}
		}

		//faild to find addresses to replace
		if (m_startAddress < 10) {
			fail("Failed to find starting address!");
			return;
		}

		//success
		success();
	}
	else {
		fail("Failed to open '" + path + "'!");
		return;
	}
}

std::uint32_t IncreaseMemAlloc::getCurrentMemSize() const {
	assert(m_startAddress < (m_fileData.size() - getReplacementDataSize()));

	auto extract = [](QByteArray::const_iterator& it) {
		std::uint32_t memSize = 0;
		memSize |= (uint32_t)(*it++) & 0x000000ff;
		memSize |= (uint32_t)(*it++) << 8 & 0x0000ff00;
		memSize |= (uint32_t)(*it++) << 16 & 0x00ff0000; 
		memSize |= (uint32_t)(*it++) << 24 & 0xff000000;
		return memSize;
	};

	std::vector<std::uint32_t> memSizes;
	auto dataIt = std::next(m_fileData.begin(), m_startAddress);
	for (auto it = m_schema.begin(); it != std::prev(m_schema.end()); ++it) {
		std::advance(dataIt, it->size());
		memSizes.push_back(extract(dataIt));
	}
	if(memSizes.empty() || !std::equal(memSizes.begin() + 1, memSizes.end(), memSizes.begin()))
		return 356515840; //default
	return memSizes.front();

}

QByteArray IncreaseMemAlloc::createReplacementData(std::uint32_t memSize) const {
	assert(m_startAddress > 0 && m_schema.size() > 0);
	QByteArray out;

	for (auto it = m_schema.begin(); it != std::prev(m_schema.end()); ++it) {
		out.append(*it);
		out.append((char*)&memSize, sizeof(std::uint32_t));
	}
	out.append(m_schema.back());

	return out;
}

void IncreaseMemAlloc::fail(const QString& info) {
	m_startAddress = 0;
	m_startAddress = 0;
	ui->spMemSize->setEnabled(false);
	ui->btnOverride->setEnabled(false);
	ui->btnSaveAs->setEnabled(false);
	ui->preview->setPlainText(info);
}

void IncreaseMemAlloc::success() {
	ui->spMemSize->setEnabled(true);
	ui->btnOverride->setEnabled(true);
	ui->btnSaveAs->setEnabled(true);
	ui->spMemSize->setValue(getCurrentMemSize());
}

void IncreaseMemAlloc::save(const QString& path) {
	QFile file(path);
	if (file.open(QIODevice::WriteOnly)) {
		QByteArray replace = createReplacementData(ui->spMemSize->value());
		m_fileData.replace(m_startAddress, replace.size(), replace);
		file.write(m_fileData);
		file.close();
	}
	else {
		QMessageBox::critical(this, "Failed to save!", "Failed to save exe to:\n"+path, QMessageBox::Close);
	}
}