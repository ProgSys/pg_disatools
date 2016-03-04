/*
 * ItemPreview.cpp
 *
 *  Created on: 04.03.2016
 *      Author: ProgSys
 */

#include "ItemPreview.h"
#include <QMenu>
#include <QDebug>
#include <QContextMenuEvent>

ItemPreview::ItemPreview(QWidget *parent)
	: QGraphicsView(parent)
{
	// TODO Auto-generated constructor stub
	m_disablePreview = new QAction("Disable Preview",  this);
	connect(m_disablePreview, SIGNAL(triggered()), this, SLOT(toggleEnablePreview()));
}

void ItemPreview::setShowsImage(bool showsImage){
	m_showsImage = showsImage;
}

void ItemPreview::setEnablePreview(bool enable){
	m_previewEnabled = enable;
	if(m_previewEnabled){
		m_disablePreview->setText("Disable Preview");
	}else{
		m_showsImage = false;
		m_disablePreview->setText("Enable Preview");
	}
}

void ItemPreview::toggleEnablePreview(){
	setEnablePreview(!m_previewEnabled);
}

bool ItemPreview::isPreviewEnabled() const{
	return m_previewEnabled;
}

void ItemPreview::contextMenuEvent(QContextMenuEvent* event) {
		QMenu menu(this);
		//menu.setStyleSheet("background-color: #f0f0f0;");
		if(m_showsImage)
			menu.addActions(this->actions());
		menu.addAction(m_disablePreview);

		menu.exec(event->globalPos());

}


ItemPreview::~ItemPreview() {
	// TODO Auto-generated destructor stub
}


