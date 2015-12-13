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

#include <iostream>
#include <map>
#include <functional>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNamedNodeMap>
#include <QDomAttr>
#include <QSettings>
#include "svcore/base/Clipboard.h"
#include "svcore/data/model/Model.h"
#include "Layer.h"
#include "SliceableLayer.h"
#include "LayerFactory.h"

using namespace std;

//#define DEBUG_LAYER_FACTORY 1

LayerFactory& LayerFactory::getInstance()
{
    static LayerFactory instance;
    return instance;
}

LayerFactory::~LayerFactory()
{
}

// Every layer type should call this function at static initialisation to register with the factory
void LayerFactory::registerLayerType(const LayerType &type, const LayerConfiguration &config)
{
    if (m_configurations.find(type) != m_configurations.end()) {
        cerr << "Multiple registrations for layer" << type << endl;
        return;
    }
#ifdef DEBUG_LAYER_FACTORY
    cerr << "Registering layer type" << type << endl;
#endif
    m_configurations[type] = config;
}

Layer * LayerFactory::createLayer(LayerType type)
{
    Layer *layer = 0;
    auto it = m_configurations.find(type);
    if (it != m_configurations.end()) {
        function<Layer *(LayerType)> ctor = it->second.constructor;
        layer = ctor(type);
    }
    if (!layer) {
        cerr << "LayerFactory::createLayer: Unknown layer type " << type << endl;
    } else {
        layer->setObjectName(getLayerPresentationName(type));
        setLayerDefaultProperties(type, layer);
    }
    return layer;
}

QString LayerFactory::getLayerPresentationName(LayerType type)
{
    auto it = m_configurations.find(type);
    if (it != m_configurations.end()) {
        return Layer::tr(it->second.presentationName);
    }
    // else
    cerr << "WARNING: LayerFactory::getLayerPresentationName passed unknown layer" << endl;
    return Layer::tr("Unknown Layer");
}

bool LayerFactory::isLayerSliceable(const Layer *layer)
{
    if (dynamic_cast<const SliceableLayer *>(layer)) {
        if (layer->getType() == LayerFactory::Type("Spectrogram")) {
            //!!! We can create slices of spectrograms, but there's a
            // problem managing the models.  The source model for the
            // slice layer has to be one of the spectrogram's FFT
            // models -- that's fine, except that we can't store &
            // recall the slice layer with a reference to that model
            // because the model is internal to the spectrogram layer
            // and the document has no record of it.  We would need
            // some other way of managing models that are used in this
            // way.  For the moment we just don't allow slices of
            // spectrograms -- and provide a spectrum layer for this
            // instead.
            //
            // This business needs a bit more thought -- either come
            // up with a sensible way to deal with that stuff, or
            // simplify the existing slice layer logic so that it
            // doesn't have to deal with models disappearing on it at
            // all (and use the normal Document setModel mechanism to
            // set its sliceable model instead of the fancy pants
            // nonsense it's doing at the moment).

            return false;
        }
        return true;
    }
    return false;
}

LayerFactory::LayerTypeSet LayerFactory::getValidLayerTypes(Model *model)
{
    LayerTypeSet types;
    for (const auto &pair : m_configurations) {
        LayerType type = pair.first;
        auto it = pair.second.acceptableModelTypes.find(model->getType());
        if (it != pair.second.acceptableModelTypes.end()) {
            types.insert(type);
        }
    }
    return types;
    // We don't count TimeRuler here as it doesn't actually display
    // the data, although it can be backed by any model
}

LayerFactory::LayerTypeSet LayerFactory::getValidEmptyLayerTypes()
{
    LayerTypeSet types;
    for (const auto &pair : m_configurations) {
        LayerType type = pair.first;
        if (pair.second.isValidEmpty)
            types.insert(type);
    }
    return types;
}

LayerFactory::LayerType LayerFactory::getLayerType(const Layer *layer)
{
    return layer->getType();
}

QString LayerFactory::getLayerIconName(LayerType type)
{
    auto it = m_configurations.find(type);
    if (it != m_configurations.end()) {
        return it->second.iconName;
    }
    // else
    cerr << "WARNING: LayerFactory::getLayerTypeName passed unknown layer" << endl;
    return "unknown";
}

QString LayerFactory::getLayerTypeName(LayerType type)
{
    auto it = m_configurations.find(type);
    if (it != m_configurations.end()) {
        return it->second.name;
    }
    // else
    cerr << "WARNING: LayerFactory::getLayerTypeName passed unknown layer" << endl;
    return "unknown";
}

LayerFactory::LayerType LayerFactory::getLayerTypeForName(QString name)
{
    for (const auto &pair : m_configurations) {
        LayerType type = pair.first;
        if (pair.second.name == name)
            return type;
    }
    return "UnknownLayer";
}

void
LayerFactory::setModel(Layer *layer, Model *model)
{
    layer->trySetModel(model);
}

Model *LayerFactory::createEmptyModel(LayerType type, Model *baseModel)
{
    auto it = m_configurations.find(type);
    if (it != m_configurations.end()) {
        return it->second.createEmptyModel(baseModel);
    }
    // else
    return 0;
}

int LayerFactory::getChannel(Layer *layer)
{
    // It is up to the layer to override this function
    return layer->getChannel();
}

void LayerFactory::setChannel(Layer *layer, int channel)
{
    // It is up to the layer to override this function
    layer->setChannel(channel);
}

void LayerFactory::setLayerDefaultProperties(LayerType type, Layer *layer)
{
    // cerr << "LayerFactory::setLayerDefaultProperties: type " << type << " (name \"" << getLayerTypeName(type) << "\")" << endl;
    QSettings settings;
    settings.beginGroup("LayerDefaults");
    QString defaults = settings.value(getLayerTypeName(type), "").toString();
    if (defaults == "") return;
    // cerr << "defaults=\"" << defaults << "\"" << endl;
    QString xml = layer->toXmlString();
    QDomDocument docOld, docNew;
    if (docOld.setContent(xml, false) && docNew.setContent(defaults, false)) {
        QXmlAttributes attrs;
        QDomElement layerElt = docNew.firstChildElement("layer");
        QDomNamedNodeMap attrNodes = layerElt.attributes();
        for (int i = 0; i < attrNodes.length(); ++i) {
            QDomAttr attr = attrNodes.item(i).toAttr();
            if (attr.isNull()) continue;
            // cerr << "append \"" << attr.name() << "\" -> \"" << attr.value() << "\"" << endl;
            attrs.append(attr.name(), "", "", attr.value());
        }
        layerElt = docOld.firstChildElement("layer");
        attrNodes = layerElt.attributes();
        for (int i = 0; i < attrNodes.length(); ++i) {
            QDomAttr attr = attrNodes.item(i).toAttr();
            if (attr.isNull()) continue;
            if (attrs.value(attr.name()) == "") {
                // cerr << "append \"" << attr.name() << "\" -> \"" << attr.value() << "\"" << endl;
                attrs.append(attr.name(), "", "", attr.value());
            }
        }        
        layer->setProperties(attrs);
    }
    settings.endGroup();
}

LayerFactory::LayerType LayerFactory::getLayerTypeForClipboardContents(const Clipboard &clip)
{
    const Clipboard::PointList &contents = clip.getPoints();
    bool haveFrame = false;
    bool haveValue = false;
    bool haveDuration = false;
    bool haveLevel = false;
    for (Clipboard::PointList::const_iterator i = contents.begin();
         i != contents.end(); ++i) {
        if (i->haveFrame()) haveFrame = true;
        if (i->haveValue()) haveValue = true;
        if (i->haveDuration()) haveDuration = true;
        if (i->haveLevel()) haveLevel = true;
    }
    if (haveFrame && haveValue && haveDuration && haveLevel) return "Notes";
    if (haveFrame && haveValue && haveDuration) return "Regions";
    if (haveFrame && haveValue) return "TimeValues";
    return "TimeInstants";
}


