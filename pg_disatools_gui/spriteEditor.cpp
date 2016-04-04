/*
 * spriteEditor.cpp
 *
 *  Created on: 31.03.2016
 *      Author: ProgSys
 */

#ifndef SPRITEEDITOR_CPP_
#define SPRITEEDITOR_CPP_


#include <SpriteSheetEditor.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SpriteSheetEditor w;
    w.show();
    return a.exec();
}



#endif /* SPRITEEDITOR_CPP_ */
