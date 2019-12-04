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
#pragma once

#include <QDialog>

namespace Ui {
class IncreaseMemAlloc;
}

class IncreaseMemAlloc : public QDialog
{
    Q_OBJECT

public:
    explicit IncreaseMemAlloc(QWidget *parent = 0);

    ~IncreaseMemAlloc();

private slots:
	/*!
	* @brief Is called when the user has selected an exe.
	*/
	void locate(const QString& path);
private:
	/*!
	* @brief Will load 'allocMemSearch.txt' into memory.
	* @detals The content of the file is used to find the locations of the values
	* which shall be modified. They are marked with a '#'.
	*/
	bool loadSchema();
	/*!
	* @brief Will return total the size of the data to be replaced.  
	* @detals Is equal to createReplacementData(0).size()
	*/
	int getReplacementDataSize() const;
	/*!
	* @brief Will return the current memory allocation set in m_fileData
	*/
	std::uint32_t getCurrentMemSize() const;
	/*!
	* @brief Will insert the given number into the schema
	*/
	QByteArray createReplacementData(std::uint32_t memSize) const;

	void save(const QString& path);

	void fail(const QString& info);
	void success();

    Ui::IncreaseMemAlloc* ui;
	QByteArrayList m_schema;

	QByteArray m_fileData;
	unsigned int m_startAddress = 0;

};

