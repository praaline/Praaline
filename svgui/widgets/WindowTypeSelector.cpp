/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "WindowTypeSelector.h"

#include "WindowShapePreview.h"

#include <QVBoxLayout>
#include <QComboBox>

#include "base/Preferences.h"

WindowTypeSelector::WindowTypeSelector(WindowType defaultType)
{
    init(defaultType);
}

WindowTypeSelector::WindowTypeSelector()
{
    Preferences *prefs = Preferences::getInstance();
    int min = 0, max = 0, deflt = 0;
    WindowType type =
        WindowType(prefs->getPropertyRangeAndValue("Window Type", &min, &max,
                                                   &deflt));
    init(type);
}

void
WindowTypeSelector::init(WindowType defaultType)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    // The WindowType enum is in rather a ragbag order -- reorder it here
    // in a more sensible order
    m_windows = new WindowType[9];
    m_windows[0] = HanningWindow;
    m_windows[1] = HammingWindow;
    m_windows[2] = BlackmanWindow;
    m_windows[3] = BlackmanHarrisWindow;
    m_windows[4] = NuttallWindow;
    m_windows[5] = GaussianWindow;
    m_windows[6] = ParzenWindow;
    m_windows[7] = BartlettWindow;
    m_windows[8] = RectangularWindow;

    Preferences *prefs = Preferences::getInstance();

    m_windowShape = new WindowShapePreview;

    m_windowCombo = new QComboBox;
    int window = int(defaultType);
    int index = 0;
    
    for (int i = 0; i <= 8; ++i) {
        m_windowCombo->addItem(prefs->getPropertyValueLabel("Window Type",
                                                            m_windows[i]));
        if (m_windows[i] == window) index = i;
    }

    m_windowCombo->setCurrentIndex(index);

    layout->addWidget(m_windowShape);
    layout->addWidget(m_windowCombo);

    connect(m_windowCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(windowIndexChanged(int)));
    windowIndexChanged(index);
}

WindowTypeSelector::~WindowTypeSelector()
{
    delete[] m_windows;
}

WindowType
WindowTypeSelector::getWindowType() const
{
    return m_windowType;
}

void
WindowTypeSelector::setWindowType(WindowType type)
{
    if (type == m_windowType) return;
    int index;
    for (index = 0; index <= 8; ++index) {
        if (m_windows[index] == type) break;
    }
    if (index <= 8) m_windowCombo->setCurrentIndex(index);
    m_windowType = type;
    m_windowShape->setWindowType(m_windowType);
}

void
WindowTypeSelector::windowIndexChanged(int index)
{
    WindowType type = m_windows[index];
    if (type == m_windowType) return;
    m_windowType = type;
    m_windowShape->setWindowType(m_windowType);
    emit windowTypeChanged(type);
}

