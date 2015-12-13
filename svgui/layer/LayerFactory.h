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

#ifndef _LAYER_FACTORY_H_
#define _LAYER_FACTORY_H_

#include <QString>
#include <QStringList>
#include <set>
#include <map>
#include <string>
#include <functional>

class Layer;
class Model;
class Clipboard;

class LayerFactory
{
public:
    typedef std::string LayerType;
    typedef std::set<LayerType> LayerTypeSet;

    static LayerType Type(const QString &s) { return s.toStdString(); }

    struct LayerConfiguration {
        LayerConfiguration() {}
        LayerConfiguration(const QString &name, const char *presentationName,
                           const QString &iconName, bool isValidEmpty,
                           const QStringList &_acceptableModelTypes,
                           std::function<Layer *(LayerType)> constructor,
                           std::function<Model *(Model*)> createEmptyModel) :
            name(name), presentationName(presentationName), iconName(iconName),
            isValidEmpty(isValidEmpty), constructor(constructor), createEmptyModel(createEmptyModel)
        {
            foreach (QString modelType, _acceptableModelTypes) {
                acceptableModelTypes.insert(modelType.toStdString());
            }
        }

        QString name;
        const char *presentationName;
        QString iconName;
        bool isSliceable;
        bool isValidEmpty;
        std::set<std::string> acceptableModelTypes;
        std::function<Layer *(LayerType)> constructor;
        std::function<Model *(Model *)> createEmptyModel;
    };

    static LayerFactory& getInstance();
    virtual ~LayerFactory();

    void registerLayerType(const LayerType &type, const LayerConfiguration &config);

    LayerTypeSet getValidLayerTypes(Model *model);
    LayerTypeSet getValidEmptyLayerTypes();

    Layer *createLayer(LayerType type);

    QString getLayerPresentationName(LayerType type);
    bool isLayerSliceable(const Layer *);
    QString getLayerIconName(LayerType);
    QString getLayerTypeName(LayerType);
    LayerType getLayerTypeForName(QString);

    LayerType getLayerType(const Layer *);

    void setLayerDefaultProperties(LayerType type, Layer *layer);

    void setModel(Layer *layer, Model *model);
    Model *createEmptyModel(LayerType type, Model *baseModel);

    int getChannel(Layer *layer);
    void setChannel(Layer *layer, int channel);

    LayerType getLayerTypeForClipboardContents(const Clipboard &);

private:
    std::map<LayerType, LayerConfiguration> m_configurations;
};

#endif

