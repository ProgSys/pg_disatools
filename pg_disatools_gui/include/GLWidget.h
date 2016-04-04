/*
 * GLWidget.h
 *
 *  Created on: 04.04.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_GLWIDGET_H_
#define INCLUDE_GLWIDGET_H_

#include <QOpenGLWidget>

class GLWidget : public QOpenGLWidget {
	Q_OBJECT
public:
	explicit GLWidget(QWidget *parent = 0);

    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    virtual ~GLWidget();
};

#endif /* INCLUDE_GLWIDGET_H_ */
