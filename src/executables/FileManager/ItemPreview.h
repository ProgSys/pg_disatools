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


#ifndef ITEMPREVIEW_H_
#define ITEMPREVIEW_H_

#include <QWidget>
#include <QGraphicsView>
class ItemPreview: public QGraphicsView {
	Q_OBJECT
public:
	explicit ItemPreview(QWidget *parent = 0);

	bool isPreviewEnabled() const;

	virtual ~ItemPreview();

public slots:
	void setShowsImage(bool showsImage = true);
	void setEnablePreview(bool enable = true);
	void toggleEnablePreview();

protected :
    virtual void contextMenuEvent(QContextMenuEvent* event);
private:
    bool m_showsImage = false;
    bool m_previewEnabled = true;
    QAction* m_disablePreview;
};

#endif /* ITEMPREVIEW_H_ */
