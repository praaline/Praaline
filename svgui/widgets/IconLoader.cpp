/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2007 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "IconLoader.h"

#include <QPixmap>
#include <QApplication>
#include <QPainter>
#include <QPalette>

static const char *autoInvertExceptions[] = {
    // These are the icons that look OK in their default colours, even
    // in a colour scheme with a black background.  (They may also be
    // icons that would look worse if we tried to auto-invert them.)
    // If we have icons that look bad when auto-inverted but that are
    // not suitable for use without being inverted, we'll need to
    // supply inverted versions -- the loader will load xx_inverse.png
    // in preference to xx.png if a dark background is found.)
    "fileclose",
    "filenew-22",
    "filenew",
    "fileopen-22",
    "fileopen",
    "fileopenaudio",
    "fileopensession",
    "filesave-22",
    "filesave",
    "filesaveas-22",
    "filesaveas",
    "help",
    "editcut",
    "editcopy",
    "editpaste",
    "editdelete",
    "exit",
    "zoom-fit",
    "zoom-in",
    "zoom-out",
    "zoom"
};

QIcon
IconLoader::load(QString name)
{
    QPixmap pmap(loadPixmap(name));
    if (pmap.isNull()) return QIcon();
    else return QIcon(pmap);
}

QPixmap
IconLoader::loadPixmap(QString name)
{
    QColor bg = QApplication::palette().window().color();
    if (bg.red() + bg.green() + bg.blue() > 384) { // light background
        QPixmap pmap(QString(":icons/visualiser/%1").arg(name));
        if (pmap.isNull()) {
            pmap = QPixmap(QString(":icons/visualiser/%1.png").arg(name));
        }
        return pmap;
    }

    QPixmap pmap(QString(":icons/visualiser/%1").arg(name));
    if (pmap.isNull()) {
        pmap = QPixmap(QString(":icons/visualiser/%1_inverse.png").arg(name));
        if (pmap.isNull()) {
            pmap = QPixmap(QString(":icons/visualiser/%1.png").arg(name));
        }
    }
    if (pmap.isNull()) return pmap;

    for (int i = 0; i < int(sizeof(autoInvertExceptions)/
                            sizeof(autoInvertExceptions[0])); ++i) {
        if (autoInvertExceptions[i] == name) {
            return pmap;
        }
    }

    // No suitable inverted icon found for black background; try to
    // auto-invert the default one

    QImage img = pmap.toImage().convertToFormat(QImage::Format_ARGB32);

    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {

            QRgb rgba = img.pixel(x, y);
            QColor colour = QColor
                (qRed(rgba), qGreen(rgba), qBlue(rgba), qAlpha(rgba));

            int alpha = colour.alpha();
            if (colour.saturation() < 5 && colour.alpha() > 10) {
                colour.setHsv(colour.hue(),
                              colour.saturation(),
                              255 - colour.value());
                colour.setAlpha(alpha);
                img.setPixel(x, y, colour.rgba());
            }
        }
    }

    pmap = QPixmap::fromImage(img);
    return pmap;
}

