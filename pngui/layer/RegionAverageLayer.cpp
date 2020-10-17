#include "PraalineCore/Base/RealTime.h"
#include "svcore/data/model/Model.h"
#include "svcore/base/Profiler.h"
#include "svgui/layer/ColourDatabase.h"
#include "svgui/layer/LinearNumericalScale.h"
#include "svgui/layer/SingleColourLayer.h"
#include "svgui/layer/VerticalScaleLayer.h"
#include "svgui/layer/RegionLayer.h"
#include "svgui/widgets/TextAbbrev.h"
#include "svgui/view/View.h"

#include "pngui/model/visualiser/RegionAverageModel.h"

#include <QPainter>
#include <QMouseEvent>
#include <QInputDialog>
#include <QTextStream>
#include <QMessageBox>

#include <iostream>
#include <cmath>

#include "RegionAverageLayer.h"

RegionAverageLayer::RegionAverageLayer() :
    SingleColourLayer(), m_model(0)
{
}

RegionAverageLayer::~RegionAverageLayer()
{
    qDeleteAll(m_sublayers);
}

// ====================================================================================================================
// Model
// ====================================================================================================================

bool RegionAverageLayer::trySetModel(Model *model)
{
    if (trySetModelHelper<RegionAverageLayer, RegionAverageModel>(this, model))
        return true;
    return false;
}

void RegionAverageLayer::setModel(RegionAverageModel *model)
{
    if (m_model == model) return;
    m_model = model;
    connectSignals(m_model);
    updateSublayers();
    // cerr << "MovingAvergaeLayer::setModel(" << model << ")" << endl;
    emit modelReplaced();
}

void RegionAverageLayer::updateSublayers()
{
    // Delete and clear previous sub-layers
    qDeleteAll(m_sublayers);
    m_sublayers.clear();
    // Create new sub-layers
    foreach (QString speakerID, m_model->speakers()) {
        m_sublayers.insert(speakerID, new RegionLayer());
        m_sublayers[speakerID]->setModel(m_model->regionModel(speakerID));
        m_sublayers[speakerID]->setBaseColour(this->getBaseColour());
        m_sublayers[speakerID]->setPlotStyle(RegionLayer::PlotLines);
    }
}

// ====================================================================================================================
// Propeties
// ====================================================================================================================

Layer::PropertyList RegionAverageLayer::getProperties() const
{
    PropertyList list = SingleColourLayer::getProperties();
    list.push_back("Annotation Level");
    list.push_back("Regions Annot. Level");
    return list;
}

QString RegionAverageLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Annotation Level")     return tr("Annotation Level");
    if (name == "Regions Annot. Level") return tr("Regions Annot. Level");
    return SingleColourLayer::getPropertyLabel(name);
}

Layer::PropertyType RegionAverageLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Annotation Level")     return ValueProperty;
    if (name == "Regions Annot. Level") return ValueProperty;
    return SingleColourLayer::getPropertyType(name);
}

int RegionAverageLayer::getPropertyRangeAndValue(const PropertyName &name, int *min, int *max, int *deflt) const
{
    return SingleColourLayer::getPropertyRangeAndValue(name, min, max, deflt);
}

QString RegionAverageLayer::getPropertyValueLabel(const PropertyName &name, int value) const
{
    return SingleColourLayer::getPropertyValueLabel(name, value);
}

void RegionAverageLayer::setProperty(const PropertyName &name, int value)
{
    if (name == "Colour") {
        foreach (QString speakerID, m_sublayers.keys()) {
            m_sublayers.value(speakerID)->setBaseColour(value);
        }
        setBaseColour(value);
    }
    else SingleColourLayer::setProperty(name, value);
}

void RegionAverageLayer::setProperties(const QXmlAttributes &attributes)
{
    SingleColourLayer::setProperties(attributes);
}

void RegionAverageLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    foreach (QString speakerID, m_sublayers.keys()) {
        m_sublayers.value(speakerID)->paint(v, paint, rect);
    }
}

int RegionAverageLayer::getVerticalScaleWidth(View *v, bool, QPainter &paint) const
{
    if (!m_model) return 0;
    return LinearNumericalScale().getWidth(v, paint);
}

void RegionAverageLayer::paintVerticalScale(View *v, bool detailed, QPainter &paint, QRect rect) const
{
    if (m_sublayers.isEmpty()) return;
    m_sublayers.first()->paintVerticalScale(v, detailed, paint, rect);
    if (getScaleUnits() != "") {
        int w = m_sublayers.first()->getVerticalScaleWidth(v, false, paint);
        int mw = w - 5;
        paint.drawText(5, 5 + paint.fontMetrics().ascent(),
                       TextAbbrev::abbreviate(getScaleUnits(), paint.fontMetrics(), mw));
    }
}

QString RegionAverageLayer::getFeatureDescription(View *v, QPoint &p) const
{
    QString ret;
    foreach (QString speakerID, m_sublayers.keys()) {
        QString s = m_sublayers.value(speakerID)->getFeatureDescription(v, p);
        if (!s.isEmpty())
            ret.append(QString("%1: %2").arg(speakerID).arg(QString(s).replace("\n", " "))).append("\n");
    }
    return ret.trimmed();
}

bool RegionAverageLayer::snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    Q_UNUSED(v)
    Q_UNUSED(frame)
    Q_UNUSED(resolution)
    Q_UNUSED(snap)
    Q_UNUSED(y)
    return false;
}

bool RegionAverageLayer::isLayerScrollable(const View *v) const
{
    QPoint discard;
    return !v->shouldIlluminateLocalFeatures(this, discard);
}

bool RegionAverageLayer::getValueExtents(double &min, double &max, bool &, QString &) const
{
    Q_UNUSED(min)
    Q_UNUSED(max)
    return false;
}

bool RegionAverageLayer::getDisplayExtents(double &min, double &max) const
{
    Q_UNUSED(min)
    Q_UNUSED(max)
    return false;
}

bool RegionAverageLayer::setDisplayExtents(double min, double max)
{
    Q_UNUSED(min)
    Q_UNUSED(max)
    return false;
}

void RegionAverageLayer::toXml(QTextStream &stream, QString indent, QString extraAttributes) const
{
    Q_UNUSED(stream)
    Q_UNUSED(indent)
    Q_UNUSED(extraAttributes)
}

// ====================================================================================================================
// Properties
// ====================================================================================================================

QString RegionAverageLayer::getLevelID() const
{
    if (!m_model) return QString();
    return m_model->levelID();
}

void RegionAverageLayer::setLevelID(QString levelID)
{
    if (!m_model) return;
    m_model->setLevelID(levelID);
    updateSublayers();
}

QString RegionAverageLayer::getGroupingLevelID() const
{
    if (!m_model) return QString();
    return m_model->groupingLevelID();
}

void RegionAverageLayer::setGroupingLevelID(QString groupingLevelID)
{
    if (!m_model) return;
    m_model->setGroupingLevelID(groupingLevelID);
    updateSublayers();
}


int RegionAverageLayer::getYForValue(View *, double value) const
{
    Q_UNUSED(value)
    return 0;
}

double RegionAverageLayer::getValueForY(View *, int y) const
{
    Q_UNUSED(y)
    return 0.0;
}

QString RegionAverageLayer::getScaleUnits() const
{
    if (!m_model) return QString();
    return QString("%1/s").arg(m_model->levelID());
}

void RegionAverageLayer::getScaleExtents(View *, double &min, double &max, bool &log) const
{
    Q_UNUSED(min)
    Q_UNUSED(max)
    Q_UNUSED(log)
}

int RegionAverageLayer::getDefaultColourHint(bool dark, bool &impose)
{
    Q_UNUSED(dark)
    Q_UNUSED(impose)
    return 0;
}
