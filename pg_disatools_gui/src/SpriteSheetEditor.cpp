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
#include <SpriteSheetEditor.h>
#include "ui_spritesheeteditor.h"

#include <QFileDialog>
#include <TitleDefine.h>
#include <QMessageBox>
#include <QColorDialog>
#include <QtQml>
#include <Timeline.h>

inline void about(){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("About");
    //msgBox.setWindowIcon(QMessageBox::Information);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(
    			"This gui application allows you to view and extract Disgaea PC sprite sheets.<br>"\
    			"Currently it is very basic.<br>"\

				"You can find the source code here: <a href='https://github.com/ProgSys/pg_disatools/'>https://github.com/ProgSys/pg_disatools</a><br>"

				"<br><b>GNU Lesser General Public License (LGPL):</b> <br>"
				"<br>Copyright (C) 2016  ProgSys"\
                "<br><br>This program is free software: you can redistribute it and/or modify"\
                "<br>it under the terms of the GNU Lesser General Public License as published by"\
                "<br>the Free Software Foundation, version 3 of the License."\

                "<br><br>This program is distributed in the hope that it will be useful,"\
                "<br>but WITHOUT ANY WARRANTY; without even the implied warranty of"\
                "<br>MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"\
                "<br>GNU Lesser General Public License for more details."\

                "<br><br>You should have received a copy of the GNU Lesser General Public License"\
                "<br>along with this program.  If not, see <a href='http://doc.qt.io/qt-5/lgpl.html'> http://doc.qt.io/qt-5/lgpl.html</a>"\
				"<br>or <a href='http://www.gnu.org/licenses/'> http://www.gnu.org/licenses/ </a>."\
                );

    msgBox.exec();
}

SpriteSheetEditor::SpriteSheetEditor(QWidget *parent):
	QMainWindow(parent),
	ui(new Ui::SpriteSheetEditorWindow)
{
	ui->setupUi(this);

	setWindowIcon(QIcon("resources/sprite_sheet_editor_icon.ico"));
	setWindowTitle(SpriteSheetEditorTITLE);

	ui->btnPrevious->setIcon(QIcon("resources/materials/icons/previous.png"));
	ui->btnPausePlay->setIcon(QIcon("resources/materials/icons/pause.png"));
	ui->btnNext->setIcon(QIcon("resources/materials/icons/next.png"));

	//ui->openGLWidget->openSprite("C:/Users/ProgSys/Desktop/Disgaea/PC/IMY/LAHARL.SH");
	//files
	connect(ui->actionOpen_File, SIGNAL(triggered()), this, SLOT(open()));

	//tools
	connect(ui->actionDump, SIGNAL(triggered()), this, SLOT(dump()));
	connect(ui->actionExport_sprites_as_PNG, &QAction::triggered, this, [this]{exportSprites("PNG");} );
	connect(ui->actionExport_sprites_as_TGA, &QAction::triggered, this, [this]{exportSprites("TGA");} );

	//View
	connect(ui->action_Pick_color, SIGNAL(triggered()), this, SLOT(pickBackgroundColor()));
	connect(ui->actionDisplay_external_references, SIGNAL(triggered(bool)), ui->openGLWidget, SLOT(displayExternalReferences(bool)));
	connect(ui->actionDisplay_ground, SIGNAL(triggered(bool)), ui->openGLWidget, SLOT(displayGround(bool)));
	connect(ui->actionDisplay_shadow, SIGNAL(triggered(bool)), ui->openGLWidget, SLOT(displayShadow(bool)));
	connect(ui->actionTimeline, SIGNAL(triggered(bool)), ui->dockTimeline, SLOT(setVisible(bool)));
	connect(ui->dockTimeline, SIGNAL(visibilityChanged(bool)), ui->actionTimeline, SLOT(setChecked(bool)));

	//About
	connect(ui->actionAbout, &QAction::triggered, this, [this]{
		about();
	});
	connect(ui->actionAbout_Qt, &QAction::triggered, this, [this]{
		 QMessageBox::aboutQt(this);
	} );

	//player functions
	connect(ui->btnPrevious, SIGNAL(clicked()), ui->openGLWidget, SLOT(previousFrame()));
	connect(ui->btnPausePlay, SIGNAL(clicked()), this, SLOT(clickPlayPause()));
	connect(ui->btnNext, SIGNAL(clicked()), ui->openGLWidget, SLOT(nextFrame()));

	connect(this, SIGNAL(openSprite(const PG::FILE::SpriteAnimation* spriteSheet)), ui->openGLWidget, SLOT(open(const PG::FILE::SpriteAnimation* spriteSheet)));
	connect(this, SIGNAL(dumpSprite( const QString& )), ui->openGLWidget, SLOT(dump( const QString& )));
	connect(this, SIGNAL( exportSprites( const QString& , const QString& ) ), ui->openGLWidget, SLOT( exportSprites( const QString& , const QString& ) ) );
	connect(ui->comboBox, SIGNAL(currentIndexChanged( int )), ui->openGLWidget, SLOT(setAnimation( int )));

	connect(this, SIGNAL(backgroundColorSelected(  const QColor& )), ui->openGLWidget, SLOT( setBackgroundColor( const QColor& )));

	ui->openGLWidget->setUpConnections(this);

	//timeline
	m_tline = new Timeline(this);
    qmlRegisterType<Timeline>("MyTimeLine",0,1, "Timeline");
    qmlRegisterType<Keyframe>("MyKeyframe",0,1, "Keyframe");
    ui->timelineQML->setSource(QUrl::fromLocalFile("QML/Timeline.qml"));
    ui->timelineQML->rootContext()->setContextProperty("timeline", m_tline);
}

void SpriteSheetEditor::open(){
    QFileDialog openDialog(this);
    openDialog.setNameFilter(tr("SPRITE (*.sh);;DATA (*.dat)"));

    QStringList fileNames;
	if (openDialog.exec()){
		fileNames = openDialog.selectedFiles();
		if(fileNames.size() > 0){
			open(fileNames[0]);
		}
	}
}

void SpriteSheetEditor::open(const QString& file){
	ui->comboBox->clear();

	if(file.isEmpty() || m_spriteSheet.importSH(file.toStdString()) || !(emit openSprite(&m_spriteSheet))){
		ui->statusbar->showMessage(QString("Failed to opened %1.").arg(file));
		setTitel();

		ui->actionDump->setEnabled(false);
		ui->actionExport_sprites_as_PNG->setEnabled(false);
		ui->actionExport_sprites_as_TGA->setEnabled(false);
	}else{
		ui->statusbar->showMessage(QString("Opened %1.").arg(file));
		setTitel(file);

		ui->actionDump->setEnabled(true);
		ui->actionExport_sprites_as_PNG->setEnabled(true);
		ui->actionExport_sprites_as_TGA->setEnabled(true);
	}

}

void SpriteSheetEditor::addAnimation(const QString& animation){
	ui->comboBox->addItem(animation);
}

void SpriteSheetEditor::addAnimations(const QStringList& animations){
	ui->comboBox->addItems(animations);
}

void SpriteSheetEditor::dump(){
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Dump"),
										"dump",
	                                   tr("txt (*.txt)"));

	if (fileName.isEmpty()) return;

	if(emit dumpSprite(fileName)){
		ui->statusbar->showMessage(QString("Dumped to %1.").arg(fileName));
	}else{
		ui->statusbar->showMessage(QString("Failed to dump file to %1.").arg(fileName));
	}
}

void SpriteSheetEditor::exportSprites(const QString& filetype){
	QString dir = QFileDialog::getExistingDirectory(this, tr("Extract Directory"), "/home",QFileDialog::ShowDirsOnly
	                                             | QFileDialog::DontResolveSymlinks);

	if(dir.isEmpty() || dir.isNull()){
		ui->statusbar->showMessage("Invalid extract directory.");
		return;
	}

	if(emit exportSprites(dir, filetype)){
		ui->statusbar->showMessage(QString("Saved sprites to %1.").arg(dir));
	}else{
		ui->statusbar->showMessage(QString("Failed to export sprites to %1.").arg(dir));
	}
}

void SpriteSheetEditor::setCurrentFrame(unsigned int currFrame){
	ui->labelCurentFrame->setText(QString::number(currFrame+1));
}

void SpriteSheetEditor::setTotalFrames(unsigned int totalFrames){
	ui->labelTotalFrames->setText(QString::number(totalFrames));
}

void SpriteSheetEditor::setTitel(){
	setWindowTitle(SpriteSheetEditorTITLE);
}

void SpriteSheetEditor::setTitel(const QString& filename){
	 if(filename.isEmpty()){
		 setTitel();
		 return;
	 }
	 QString title(SpriteSheetEditorTITLE);
	 title.append(" - ");
	 title.append(filename);
	 setWindowTitle(title);
}

void SpriteSheetEditor::clickPlayPause(){
	if(ui->openGLWidget->isPlaying()){
		ui->openGLWidget->pause();
		ui->btnPausePlay->setIcon(QIcon("resources/materials/icons/play.png"));
	}else{
		ui->openGLWidget->play();
		ui->btnPausePlay->setIcon(QIcon("resources/materials/icons/pause.png"));
	}
}

void SpriteSheetEditor::pickBackgroundColor(){
	QColor color = QColorDialog::getColor(QColor(5,79,121), this, "Pick a background color!");
	emit backgroundColorSelected(color);
}

SpriteSheetEditor::~SpriteSheetEditor() {
	delete m_tline;
	delete ui;
}

