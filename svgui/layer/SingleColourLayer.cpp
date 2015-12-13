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

#include "SingleColourLayer.h"
#include "ColourDatabase.h"
#include "view/View.h"

#include <iostream>

#include <QTextStream>
#include <QApplication>

//#define DEBUG_COLOUR_SELECTION 1

SingleColourLayer::ColourRefCount 
SingleColourLayer::m_colourRefCount;

SingleColourLayer::SingleColourLayer() :
    m_colour(0),
    m_colourExplicitlySet(false),
    m_defaultColourSet(false)
{
    // Reference current colour because setDefaulColourFor
    // will unreference it before (possibly) changing it.
    refColor();
    setDefaultColourFor(0);
}

SingleColourLayer::~SingleColourLayer()
{
    unrefColor();
}

QPixmap
SingleColourLayer::getLayerPresentationPixmap(QSize size) const
{
    return ColourDatabase::getInstance()->getExamplePixmap(m_colour, size);
}

bool
SingleColourLayer::hasLightBackground() const
{
    bool dark = ColourDatabase::getInstance()->useDarkBackground(m_colour);
    return !dark;
}

Layer::PropertyList
SingleColourLayer::getProperties() const
{
    PropertyList list = Layer::getProperties();
    list.push_back("Colour");
    return list;
}

QString
SingleColourLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Colour") return tr("Colour");
    return "";
}

Layer::PropertyType
SingleColourLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Colour") return ColourProperty;
    return InvalidProperty;
}

QString
SingleColourLayer::getPropertyGroupName(const PropertyName &) const
{
    return QString();
}

int
SingleColourLayer::getPropertyRangeAndValue(const PropertyName &name,
                                        int *min, int *max, int *deflt) const
{
    int val = 0;

    int garbage0, garbage1, garbage2;
    if (!min) min = &garbage0;
    if (!max) max = &garbage1;
    if (!deflt) deflt = &garbage2;

    if (name == "Colour") {

        ColourDatabase::getInstance()->getColourPropertyRange(min, max);
        *deflt = 0; //!!!

        val = m_colour;

    } else {
	val = Layer::getPropertyRangeAndValue(name, min, max, deflt);
    }

    return val;
}

QString
SingleColourLayer::getPropertyValueLabel(const PropertyName &name,
				    int value) const
{
    if (name == "Colour") {
        ColourDatabase *db = ColourDatabase::getInstance();
        if (value >= 0 && value < db->getColourCount()) {
            return db->getColourName(value);
        }
    }
    return tr("<unknown>");
}

RangeMapper *
SingleColourLayer::getNewPropertyRangeMapper(const PropertyName &) const
{
    return 0;
}

void
SingleColourLayer::setProperty(const PropertyName &name, int value)
{
    if (name == "Colour") {
        setBaseColour(value);
    }
}

void
SingleColourLayer::setDefaultColourFor(View *v)
{
#ifdef DEBUG_COLOUR_SELECTION
    cerr << "SingleColourLayer::setDefaultColourFor: m_colourExplicitlySet = " << m_colourExplicitlySet << ", m_defaultColourSet " << m_defaultColourSet << endl;
#endif

    if (m_colourExplicitlySet || m_defaultColourSet) return;

    if (v) m_defaultColourSet = true; // v==0 case doesn't really count

    bool dark = false;
    if (v) {
        dark = !v->hasLightBackground();
    } else {
        QColor bg = QApplication::palette().color(QPalette::Window);
        if (bg.red() + bg.green() + bg.blue() < 384) dark = true;
    }

    ColourDatabase *cdb = ColourDatabase::getInstance();

    int hint = -1;
    bool impose = false;
    if (v) {
        // We don't want to call this if !v because that probably
        // means we're being called from the constructor, and this is
        // a virtual function
        hint = getDefaultColourHint(dark, impose);
#ifdef DEBUG_COLOUR_SELECTION
        cerr << "hint = " << hint << ", impose = " << impose << endl;
#endif
    } else {
#ifdef DEBUG_COLOUR_SELECTION
        cerr << "(from ctor)" << endl;
#endif
    }

    if (hint >= 0 && impose) {
        setBaseColour(hint);
        return;
    }

    unrefColor();

    int bestCount = 0, bestColour = -1;
    
    for (int i = 0; i < cdb->getColourCount(); ++i) {

        int index = i;
        if (hint > 0) index = (index + hint) % cdb->getColourCount();
        if (cdb->useDarkBackground(index) != dark) continue;

        int count = 0;
        if (m_colourRefCount.find(index) != m_colourRefCount.end()) {
            count = m_colourRefCount[index];
        }

#ifdef DEBUG_COLOUR_SELECTION
        cerr << "index = " << index << ", count = " << count;
#endif

        if (bestColour < 0 || count < bestCount) {
            bestColour = index;
            bestCount = count;
#ifdef DEBUG_COLOUR_SELECTION
            cerr << " *";
#endif
        }

#ifdef DEBUG_COLOUR_SELECTION
        cerr << endl;
#endif
    }

    if (bestColour < 0) m_colour = 0;
    else m_colour = bestColour;

    refColor();
}

void
SingleColourLayer::setBaseColour(int colour)
{
    m_colourExplicitlySet = true;

    if (m_colour == colour) return;

    refColor();
    m_colour = colour;
    unrefColor();

    flagBaseColourChanged();
    emit layerParametersChanged();
}

int
SingleColourLayer::getBaseColour() const
{
    return m_colour;
}

QColor
SingleColourLayer::getBaseQColor() const
{
    return ColourDatabase::getInstance()->getColour(m_colour);
}

QColor
SingleColourLayer::getBackgroundQColor(View *v) const
{
    return v->getBackground();
}

QColor
SingleColourLayer::getForegroundQColor(View *v) const
{
    return v->getForeground();
}

std::vector<QColor>
SingleColourLayer::getPartialShades(View *v) const
{
    std::vector<QColor> s;
    QColor base = getBaseQColor();
    QColor bg = getBackgroundQColor(v);
    for (int i = 0; i < 3; ++i) {
        int red = base.red() + ((bg.red() - base.red()) * (i + 1)) / 4;
        int green = base.green() + ((bg.green() - base.green()) * (i + 1)) / 4;
        int blue = base.blue() + ((bg.blue() - base.blue()) * (i + 1)) / 4;
        s.push_back(QColor(red, green, blue));
    }
    return s;
}

void
SingleColourLayer::toXml(QTextStream &stream,
                         QString indent, QString extraAttributes) const
{
    QString s;
    
    QString colourName, colourSpec, darkbg;
    ColourDatabase::getInstance()->getStringValues
        (m_colour, colourName, colourSpec, darkbg);

    s += QString("colourName=\"%1\" "
                 "colour=\"%2\" "
                 "darkBackground=\"%3\" ")
	.arg(colourName)
        .arg(colourSpec)
        .arg(darkbg);

    Layer::toXml(stream, indent, extraAttributes + " " + s);
}

void
SingleColourLayer::setProperties(const QXmlAttributes &attributes)
{
    QString colourName = attributes.value("colourName");
    QString colourSpec = attributes.value("colour");
    QString darkbg = attributes.value("darkBackground");

    int colour = ColourDatabase::getInstance()->putStringValues
        (colourName, colourSpec, darkbg);

    if (colour == -1)
      return;

    m_colourExplicitlySet = true;

    if (m_colour != colour) {

#ifdef DEBUG_COLOUR_SELECTION
        cerr << "SingleColourLayer::setProperties: changing colour from " << m_colour << " to " << colour << endl;
#endif

        unrefColor();
        m_colour = colour;
        refColor();

        flagBaseColourChanged();
    }
}

void SingleColourLayer::refColor()
{
    if (m_colourRefCount.find(m_colour) == m_colourRefCount.end()) {
        m_colourRefCount[m_colour] = 1;
    } else {
        m_colourRefCount[m_colour]++;
    }
}

void SingleColourLayer::unrefColor()
{
    if (m_colourRefCount.find(m_colour) != m_colourRefCount.end() &&
        m_colourRefCount[m_colour] > 0) {
        m_colourRefCount[m_colour]--;
    }
}
