
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

#ifndef _LAYER_TREE_H_
#define _LAYER_TREE_H_

#include <QAbstractItemModel>

#include <set>

class PaneStack;
class View;
class Pane;
class Layer;
class PropertyContainer;
class Model;

class ModelMetadataModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ModelMetadataModel(PaneStack *stack, bool waveModelsOnly, QObject *parent = 0);
    virtual ~ModelMetadataModel();

    QVariant data(const QModelIndex &index, int role) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;

    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

protected slots:
    void paneAdded();
    void paneDeleted();
    void propertyContainerAdded(PropertyContainer *);
    void propertyContainerRemoved(PropertyContainer *);
    void propertyContainerSelected(PropertyContainer *);
    void propertyContainerPropertyChanged(PropertyContainer *);
    void playParametersAudibilityChanged(bool);
    void paneLayerModelChanged();
    void rebuildModelSet();

protected:
    PaneStack *m_stack;
    bool m_waveModelsOnly;
    int m_modelTypeColumn;
    int m_modelNameColumn;
    int m_modelMakerColumn;
    int m_modelSourceColumn;
    int m_columnCount;

    std::set<Model *> m_models;
};

class LayerTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    LayerTreeModel(PaneStack *stack, QObject *parent = 0);
    virtual ~LayerTreeModel();

    QVariant data(const QModelIndex &index, int role) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;

    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

protected slots:
    void paneAdded();
    void paneAboutToBeDeleted(Pane *);
    void propertyContainerAdded(PropertyContainer *);
    void propertyContainerRemoved(PropertyContainer *);
    void propertyContainerSelected(PropertyContainer *);
    void propertyContainerPropertyChanged(PropertyContainer *);
    void paneLayerModelChanged();
    void playParametersAudibilityChanged(bool);

protected:
    PaneStack *m_stack;
    std::set<QObject *> m_deletedPanes;
    int m_layerNameColumn;
    int m_layerVisibleColumn;
    int m_layerPlayedColumn;
    int m_modelNameColumn;
    int m_columnCount;
};

#endif
