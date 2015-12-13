/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "LevelPanToolButton.h"
#include "LevelPanWidget.h"

#include <QMenu>
#include <QWidgetAction>
#include <QImage>
#include <QStylePainter>
#include <QStyleOptionToolButton>

#include <iostream>
using std::cerr;
using std::endl;

LevelPanToolButton::LevelPanToolButton(QWidget *parent) :
    QToolButton(parent),
    m_pixels(32),
    m_pixelsBig(32 * 3),
    m_muted(false),
    m_savedLevel(1.f)
{
    m_lpw = new LevelPanWidget();

    connect(m_lpw, SIGNAL(levelChanged(float)), this, SIGNAL(levelChanged(float)));
    connect(m_lpw, SIGNAL(levelChanged(float)), this, SLOT(selfLevelChanged(float)));

    connect(m_lpw, SIGNAL(panChanged(float)), this, SIGNAL(panChanged(float)));
    connect(m_lpw, SIGNAL(panChanged(float)), this, SLOT(update()));

    connect(this, SIGNAL(clicked(bool)), this, SLOT(selfClicked()));
    
    QMenu *menu = new QMenu();
    QWidgetAction *wa = new QWidgetAction(menu);
    wa->setDefaultWidget(m_lpw);
    menu->addAction(wa);

    setPopupMode(InstantPopup);
    setMenu(menu);

    setImageSize(m_pixels);
    setBigImageSize(m_pixelsBig);
}

LevelPanToolButton::~LevelPanToolButton()
{
}

float
LevelPanToolButton::getLevel() const
{
    return m_lpw->getLevel();
}

float
LevelPanToolButton::getPan() const
{
    return m_lpw->getPan();
}

bool
LevelPanToolButton::includesMute() const
{
    return m_lpw->includesMute();
}

void
LevelPanToolButton::setImageSize(int pixels)
{
    m_pixels = pixels;

    QPixmap px(m_pixels, m_pixels);
    px.fill(Qt::transparent);
    setIcon(px);
}

void
LevelPanToolButton::setBigImageSize(int pixels)
{
    m_pixelsBig = pixels;

    m_lpw->setFixedWidth(m_pixelsBig);
    m_lpw->setFixedHeight(m_pixelsBig);
}

void
LevelPanToolButton::setLevel(float level)
{
    m_lpw->setLevel(level);
    update();
}

void
LevelPanToolButton::setPan(float pan)
{
    m_lpw->setPan(pan);
    update();
}

void
LevelPanToolButton::setIncludeMute(bool include)
{
    m_lpw->setIncludeMute(include);
    update();
}

void
LevelPanToolButton::setEnabled(bool enabled)
{
    m_lpw->setEnabled(enabled);
    QToolButton::setEnabled(enabled);
}

void
LevelPanToolButton::selfLevelChanged(float level)
{
    if (level > 0.f) {
	m_muted = false;
    } else {
	m_muted = true;
	m_savedLevel = 1.f;
    }
    update();
}

void
LevelPanToolButton::selfClicked()
{
    cerr << "selfClicked" << endl;
    
    if (m_muted) {
	m_muted = false;
	m_lpw->setLevel(m_savedLevel);
	emit levelChanged(m_savedLevel);
    } else {
	m_savedLevel = m_lpw->getLevel();
	m_muted = true;
	m_lpw->setLevel(0.f);
	emit levelChanged(0.f);
    }
    update();
}

void
LevelPanToolButton::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    opt.features &= (~QStyleOptionToolButton::HasMenu);
    p.drawComplexControl(QStyle::CC_ToolButton, opt);
    
    if (m_pixels >= height()) {
        setImageSize(height()-1);
    }
    
    double margin = (double(height()) - m_pixels) / 2.0;
    m_lpw->renderTo(this, QRectF(margin, margin, m_pixels, m_pixels), false);
}


