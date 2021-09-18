
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

#include "LayerTree.h"
#include "view/PaneStack.h"

#include "base/PlayParameters.h"
#include "view/Pane.h"
#include "layer/Layer.h"
#include "data/model/Model.h"
#include "data/model/WaveFileModel.h"

#include <QIcon>
#include <iostream>


ModelMetadataModel::ModelMetadataModel(PaneStack *stack, bool waveModelsOnly,
                               QObject *parent) :
    QAbstractItemModel(parent),
    m_stack(stack),
    m_waveModelsOnly(waveModelsOnly)
{
    if (m_waveModelsOnly) {
        m_modelTypeColumn = -1;
        m_modelNameColumn = 0;
        m_modelMakerColumn = 1;
        m_modelSourceColumn = 2;
        m_columnCount = 3;
    } else {
        m_modelTypeColumn = 0;
        m_modelNameColumn = 1;
        m_modelMakerColumn = 2;
        m_modelSourceColumn = 3;
        m_columnCount = 4;
    }

    connect(stack, SIGNAL(paneAdded()), this, SLOT(paneAdded()));
    connect(stack, &PaneStack::paneDeleted, this, &ModelMetadataModel::paneDeleted);

    for (int i = 0; i < stack->getPaneCount(); ++i) {
        Pane *pane = stack->getPane(i);
        if (!pane) continue;
        connect(pane, &View::propertyContainerAdded,
                this, &ModelMetadataModel::propertyContainerAdded);
        connect(pane, &View::propertyContainerRemoved,
                this, &ModelMetadataModel::propertyContainerRemoved);
        connect(pane, SIGNAL(propertyContainerSelected(PropertyContainer *)),
                this, SLOT(propertyContainerSelected(PropertyContainer *)));
        connect(pane, &View::propertyContainerPropertyChanged,
                this, &ModelMetadataModel::propertyContainerPropertyChanged);
        connect(pane, &View::propertyContainerNameChanged,
                this, &ModelMetadataModel::propertyContainerPropertyChanged);
        connect(pane, &View::layerModelChanged,
                this, &ModelMetadataModel::paneLayerModelChanged);
    }

    rebuildModelSet();
}

ModelMetadataModel::~ModelMetadataModel()
{
}

void
ModelMetadataModel::rebuildModelSet()
{
    std::set<Model *> unfound = m_models;

    for (int i = 0; i < m_stack->getPaneCount(); ++i) {

        Pane *pane = m_stack->getPane(i);
        if (!pane) continue;

        for (int j = 0; j < pane->getLayerCount(); ++j) {

            Layer *layer = pane->getLayer(j);
            if (!layer) continue;

            Model *model = layer->getModel();
            if (!model) continue;

            if (m_waveModelsOnly) {
                if (!dynamic_cast<WaveFileModel *>(model)) continue;
            }

            if (m_models.find(model) == m_models.end()) {
                connect(model, &Model::aboutToBeDeleted,
                        this, &ModelMetadataModel::rebuildModelSet);
                m_models.insert(model);
            } else {
                unfound.erase(model);
            }
        }
    }

    for (std::set<Model *>::iterator i = unfound.begin();
         i != unfound.end(); ++i) {
        m_models.erase(*i);
    }

    cerr << "ModelMetadataModel::rebuildModelSet: " << m_models.size() << " models" << endl;
}

void
ModelMetadataModel::paneAdded()
{
    rebuildModelSet();
    emit layoutChanged();
}

void
ModelMetadataModel::paneDeleted()
{
    rebuildModelSet();
    emit layoutChanged();
}

void
ModelMetadataModel::paneLayerModelChanged()
{
    rebuildModelSet();
    emit layoutChanged();
}

void
ModelMetadataModel::propertyContainerAdded(PropertyContainer *)
{
    rebuildModelSet();
    emit layoutChanged();
}

void
ModelMetadataModel::propertyContainerRemoved(PropertyContainer *)
{
    rebuildModelSet();
    emit layoutChanged();
}

void
ModelMetadataModel::propertyContainerSelected(PropertyContainer *)
{
}

void
ModelMetadataModel::propertyContainerPropertyChanged(PropertyContainer *)
{
}

void
ModelMetadataModel::playParametersAudibilityChanged(bool )
{
}

QVariant
ModelMetadataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

//    QObject *obj = static_cast<QObject *>(index.internalPointer());
    int row = index.row(), col = index.column();

    //!!! not exactly the ideal use of a std::set
    std::set<Model *>::iterator itr = m_models.begin();
    for (int i = 0; i < row && itr != m_models.end(); ++i, ++itr);
    if (itr == m_models.end()) return QVariant();

    Model *model = *itr;

    if (role != Qt::DisplayRole) {
        if (m_waveModelsOnly && col == m_modelNameColumn &&
            role == Qt::DecorationRole) {
            // There is no meaningful icon for a model, in general --
            // the icons we have represent layer types and it would be
            // misleading to use them for models.  However, if we're
            // only showing wave models, we use the waveform icon just
            // for decorative purposes.
            return QVariant(QIcon(QString(":/icons/visualiser/waveform.png")));
        }
        return QVariant();
    }
    
    if (col == m_modelTypeColumn) {
        return QVariant(model->getTypeName());
    } else if (col == m_modelNameColumn) {
        return QVariant(model->objectName());
    } else if (col == m_modelMakerColumn) {
        return QVariant(model->getMaker());
    } else if (col == m_modelSourceColumn) {
        return QVariant(model->getLocation());
    }        
    
    return QVariant();
}

bool
ModelMetadataModel::setData(const QModelIndex &, const QVariant &, int )
{
    return false;
}

Qt::ItemFlags
ModelMetadataModel::flags(const QModelIndex &) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    return flags;
}

QVariant
ModelMetadataModel::headerData(int section,
			   Qt::Orientation orientation,
			   int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
	if (section == m_modelTypeColumn) return QVariant(tr("Type"));
	else if (section == m_modelNameColumn) return QVariant(tr("Name"));
	else if (section == m_modelMakerColumn) return QVariant(tr("Maker"));
	else if (section == m_modelSourceColumn) return QVariant(tr("Source"));
    }

    return QVariant();
}

QModelIndex
ModelMetadataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (row >= (int)m_models.size()) return QModelIndex();
	return createIndex(row, column, (void *)0);
    }

    return QModelIndex();
}

QModelIndex
ModelMetadataModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int
ModelMetadataModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) return int(m_models.size());
    return 0;
}

int
ModelMetadataModel::columnCount(const QModelIndex &) const
{
    return m_columnCount;
}



LayerTreeModel::LayerTreeModel(PaneStack *stack, QObject *parent) :
    QAbstractItemModel(parent),
    m_stack(stack)
{
    m_layerNameColumn = 0;
    m_layerVisibleColumn = 1;
    m_layerPlayedColumn = 2;
    m_modelNameColumn = 3;
    m_columnCount = 4;

    connect(stack, SIGNAL(paneAdded()), this, SLOT(paneAdded()));
    connect(stack, &PaneStack::paneAboutToBeDeleted,
            this, &LayerTreeModel::paneAboutToBeDeleted);

    for (int i = 0; i < stack->getPaneCount(); ++i) {
        Pane *pane = stack->getPane(i);
        if (!pane) continue;
        connect(pane, &View::propertyContainerAdded,
                this, &LayerTreeModel::propertyContainerAdded);
        connect(pane, &View::propertyContainerRemoved,
                this, &LayerTreeModel::propertyContainerRemoved);
        connect(pane, SIGNAL(propertyContainerSelected(PropertyContainer *)),
                this, SLOT(propertyContainerSelected(PropertyContainer *)));
        connect(pane, &View::propertyContainerPropertyChanged,
                this, &LayerTreeModel::propertyContainerPropertyChanged);
        connect(pane, &View::propertyContainerNameChanged,
                this, &LayerTreeModel::propertyContainerPropertyChanged);
        connect(pane, &View::layerModelChanged,
                this, &LayerTreeModel::paneLayerModelChanged);

        for (int j = 0; j < pane->getLayerCount(); ++j) {
            Layer *layer = pane->getLayer(j);
            if (!layer) continue;
            PlayParameters *params = layer->getPlayParameters();
            if (!params) continue;
            connect(params, &PlayParameters::playAudibleChanged,
                    this, &LayerTreeModel::playParametersAudibilityChanged);
        }
    }
}

LayerTreeModel::~LayerTreeModel()
{
}

void
LayerTreeModel::paneAdded()
{
    emit layoutChanged();
}

void
LayerTreeModel::paneAboutToBeDeleted(Pane *pane)
{
    cerr << "paneDeleted: " << pane << endl;
    m_deletedPanes.insert(pane);
    emit layoutChanged();
}

void
LayerTreeModel::propertyContainerAdded(PropertyContainer *)
{
    emit layoutChanged();
}

void
LayerTreeModel::propertyContainerRemoved(PropertyContainer *)
{
    emit layoutChanged();
}

void
LayerTreeModel::propertyContainerSelected(PropertyContainer *)
{
    emit layoutChanged();
}

void
LayerTreeModel::paneLayerModelChanged()
{
    emit layoutChanged();
}

void
LayerTreeModel::propertyContainerPropertyChanged(PropertyContainer *pc)
{
    for (int i = 0; i < m_stack->getPaneCount(); ++i) {
        Pane *pane = m_stack->getPane(i);
        if (!pane) continue;
        for (int j = 0; j < pane->getLayerCount(); ++j) {
            if (pane->getLayer(j) == pc) {
                emit dataChanged(createIndex(pane->getLayerCount() - j - 1,
                                             m_layerNameColumn, pane),
                                 createIndex(pane->getLayerCount() - j - 1,
                                             m_modelNameColumn, pane));
            }
        }
    }
}

void
LayerTreeModel::playParametersAudibilityChanged(bool a)
{
    PlayParameters *params = dynamic_cast<PlayParameters *>(sender());
    if (!params) return;

    cerr << "LayerTreeModel::playParametersAudibilityChanged("
              << params << "," << a << ")" << endl;

    for (int i = 0; i < m_stack->getPaneCount(); ++i) {
        Pane *pane = m_stack->getPane(i);
        if (!pane) continue;
        for (int j = 0; j < pane->getLayerCount(); ++j) {
            Layer *layer = pane->getLayer(j);
            if (!layer) continue;
            if (layer->getPlayParameters() == params) {
                cerr << "LayerTreeModel::playParametersAudibilityChanged("
                          << params << "," << a << "): row " << pane->getLayerCount() - j - 1 << ", col " << 2 << endl;

                emit dataChanged(createIndex(pane->getLayerCount() - j - 1,
                                             m_layerPlayedColumn, pane),
                                 createIndex(pane->getLayerCount() - j - 1,
                                             m_layerPlayedColumn, pane));
            }
        }
    }
}

QVariant
LayerTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    QObject *obj = static_cast<QObject *>(index.internalPointer());
    int row = index.row(), col = index.column();

    Pane *pane = dynamic_cast<Pane *>(obj);
    if (!pane) {
        if (col == 0 && row < m_stack->getPaneCount()) {
            switch (role) {
            case Qt::DisplayRole:
                return QVariant(QString("Pane %1").arg(row + 1));
            case Qt::DecorationRole:
                return QVariant(QIcon(QString(":/icons/visualiser/pane.png")));
            default: break;
            }
        }
    }

    if (pane && pane->getLayerCount() > row) {
        Layer *layer = pane->getLayer(pane->getLayerCount() - row - 1);
        if (layer) {
            if (col == m_layerNameColumn) {
                switch (role) {
                case Qt::DisplayRole:
                    return QVariant(layer->objectName());
                case Qt::DecorationRole:
                    return QVariant
                        (QIcon(QString(":/icons/visualiser/%1.png")
                               .arg(layer->getPropertyContainerIconName())));
                default: break;
                }
            } else if (col == m_layerVisibleColumn) {
                if (role == Qt::CheckStateRole) {
                    return QVariant(layer->isLayerDormant(pane) ?
                                    Qt::Unchecked : Qt::Checked);
                } else if (role == Qt::TextAlignmentRole) {
                    return QVariant(Qt::AlignHCenter);
                }
            } else if (col == m_layerPlayedColumn) {
                if (role == Qt::CheckStateRole) {
                    PlayParameters *params = layer->getPlayParameters();
                    if (params) return QVariant(params->isPlayMuted() ?
                                                Qt::Unchecked : Qt::Checked);
                    else return QVariant();
                } else if (role == Qt::TextAlignmentRole) {
                    return QVariant(Qt::AlignHCenter);
                }
            } else if (col == m_modelNameColumn) {
                Model *model = layer->getModel();
                if (model && role == Qt::DisplayRole) {
                    return QVariant(model->objectName());
                }
            }
        }
    }

    return QVariant();
}

bool
LayerTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;

    QObject *obj = static_cast<QObject *>(index.internalPointer());
    int row = index.row(), col = index.column();

    Pane *pane = dynamic_cast<Pane *>(obj);
    if (!pane || pane->getLayerCount() <= row) return false;

    Layer *layer = pane->getLayer(pane->getLayerCount() - row - 1);
    if (!layer) return false;

    if (col == m_layerVisibleColumn) {
        if (role == Qt::CheckStateRole) {
            layer->showLayer(pane, value.toInt() == Qt::Checked);
            emit dataChanged(index, index);
            return true;
        }
    } else if (col == m_layerPlayedColumn) {
        if (role == Qt::CheckStateRole) {
            PlayParameters *params = layer->getPlayParameters();
            if (params) {
                params->setPlayMuted(value.toInt() == Qt::Unchecked);
                emit dataChanged(index, index);
                return true;
            }
        }
    }

    return false;
}

Qt::ItemFlags
LayerTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if (!index.isValid()) return flags;

    if (index.column() == m_layerVisibleColumn ||
        index.column() == m_layerPlayedColumn) {
        flags |= Qt::ItemIsUserCheckable;
    } else if (index.column() == 0) {
        flags |= Qt::ItemIsSelectable;
    }

    return flags;
}

QVariant
LayerTreeModel::headerData(int section,
			   Qt::Orientation orientation,
			   int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
	if (section == m_layerNameColumn) return QVariant(tr("Layer"));
        else if (section == m_layerVisibleColumn) return QVariant(tr("Shown"));
        else if (section == m_layerPlayedColumn) return QVariant(tr("Played"));
	else if (section == m_modelNameColumn) return QVariant(tr("Model"));
    }

    return QVariant();
}

QModelIndex
LayerTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    // cell for a pane contains row, column, pane stack
    // -> its parent is the invalid cell

    // cell for a layer contains row, column, pane
    // -> its parent is row, column, pane stack (which identify the pane)

    if (!parent.isValid()) {
        if (row >= m_stack->getPaneCount() || column > 0) return QModelIndex();
	return createIndex(row, column, m_stack);
    }

    QObject *obj = static_cast<QObject *>(parent.internalPointer());

    if (obj == m_stack) {
        Pane *pane = m_stack->getPane(parent.row());
        if (!pane || parent.column() > 0) return QModelIndex();
        return createIndex(row, column, pane);
    }

    return QModelIndex();
}

QModelIndex
LayerTreeModel::parent(const QModelIndex &index) const
{
    QObject *obj = static_cast<QObject *>(index.internalPointer());

    if (m_deletedPanes.find(obj) != m_deletedPanes.end()) {
//        m_deletedPanes.erase(obj);
        return QModelIndex();
    }

    Pane *pane = dynamic_cast<Pane *>(obj);
    if (pane) {
        int index = m_stack->getPaneIndex(pane);
        if (index >= 0) return createIndex(index, 0, m_stack);
    }

    return QModelIndex();
}

int
LayerTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) return m_stack->getPaneCount();

    QObject *obj = static_cast<QObject *>(parent.internalPointer());
    
    if (obj == m_stack) {
        Pane *pane = m_stack->getPane(parent.row());
        if (!pane || parent.column() > 0) return 0;
        return pane->getLayerCount();
    }

    return 0;
}

int
LayerTreeModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) return m_columnCount;

    QObject *obj = static_cast<QObject *>(parent.internalPointer());
    if (obj == m_stack) return m_columnCount; // row for a layer

    return 1;
}

