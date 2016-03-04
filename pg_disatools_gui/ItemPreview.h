/*
 * ItemPreview.h
 *
 *  Created on: 04.03.2016
 *      Author: ProgSys
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
