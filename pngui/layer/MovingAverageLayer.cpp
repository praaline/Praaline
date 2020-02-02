#include "PraalineCore/Base/RealTime.h"
#include "svcore/data/model/Model.h"
#include "svcore/base/Profiler.h"
#include "svgui/layer/ColourDatabase.h"
#include "svgui/layer/LinearNumericalScale.h"
#include "svgui/layer/SingleColourLayer.h"
#include "svgui/layer/VerticalScaleLayer.h"
#include "svgui/layer/TimeValueLayer.h"
#include "svgui/widgets/TextAbbrev.h"
#include "svgui/view/View.h"

#include "pngui/model/visualiser/MovingAverageModel.h"

#include <QPainter>
#include <QMouseEvent>
#include <QInputDialog>
#include <QTextStream>
#include <QMessageBox>

#include <iostream>
#include <cmath>

#include "MovingAverageLayer.h"

MovingAverageLayer::MovingAverageLayer() :
    SingleColourLayer(), m_model(0)
{
}

MovingAverageLayer::~MovingAverageLayer()
{
    qDeleteAll(m_sublayers);
}

// ====================================================================================================================
// Model
// ====================================================================================================================

bool MovingAverageLayer::trySetModel(Model *model)
{
    if (trySetModelHelper<MovingAverageLayer, MovingAverageModel>(this, model))
        return true;
    return false;
}

void MovingAverageLayer::setModel(MovingAverageModel *model)
{
    if (m_model == model) return;
    m_model = model;
    connectSignals(m_model);
    updateSublayers();
    // cerr << "MovingAvergaeLayer::setModel(" << model << ")" << endl;
    emit modelReplaced();
}

void MovingAverageLayer::updateSublayers()
{
    // Delete and clear previous sub-layers
    qDeleteAll(m_sublayers);
    m_sublayers.clear();
    // Create new sub-layers
    foreach (QString speakerID, m_model->speakers()) {
        m_sublayers.insert(speakerID, new TimeValueLayer());
        m_sublayers[speakerID]->setModel(m_model->smoothModel(speakerID));
        m_sublayers[speakerID]->setBaseColour(this->getBaseColour());
        m_sublayers[speakerID]->setPlotStyle(TimeValueLayer::PlotCurve);
    }
}

// ====================================================================================================================
// Propeties
// ====================================================================================================================

Layer::PropertyList MovingAverageLayer::getProperties() const
{
    PropertyList list = SingleColourLayer::getProperties();
    list.push_back("Annotation Level");
    list.push_back("Step (msec)");
    list.push_back("Window left (msec)");
    list.push_back("Window right (msec)");
    return list;
}

QString MovingAverageLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Annotation Level")     return tr("Annotation Level");
    if (name == "Step (msec)")          return tr("Step (msec)");
    if (name == "Window left (msec)")   return tr("Window left (msec)");
    if (name == "Window right (msec)")  return tr("Window right (msec)");
    return SingleColourLayer::getPropertyLabel(name);
}

Layer::PropertyType MovingAverageLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Annotation Level")     return ValueProperty;
    if (name == "Step (msec)")          return RangeProperty;
    if (name == "Window left (msec)")   return RangeProperty;
    if (name == "Window right (msec)")  return RangeProperty;
    return SingleColourLayer::getPropertyType(name);
}

int MovingAverageLayer::getPropertyRangeAndValue(const PropertyName &name, int *min, int *max, int *deflt) const
{
    if (name == "Step (msec)") {
        if (min) *min = 0;
        if (max) *max = 10000;
        if (deflt) *deflt = 250;
        return (m_model) ? m_model->smoothingStepMsec() : 0;
    }
    else if (name == "Window left (msec)") {
        if (min) *min = 0;
        if (max) *max = 10000;
        if (deflt) *deflt = 1000;
        return (m_model) ? m_model->windowLeftMsec() : 0;
    }
    else if (name == "Window right (msec)") {
        if (min) *min = 0;
        if (max) *max = 10000;
        if (deflt) *deflt = 1000;
        return (m_model) ? m_model->windowRightMsec() : 0;
    }
    return SingleColourLayer::getPropertyRangeAndValue(name, min, max, deflt);
}

QString MovingAverageLayer::getPropertyValueLabel(const PropertyName &name, int value) const
{
    return SingleColourLayer::getPropertyValueLabel(name, value);
}

void MovingAverageLayer::setProperty(const PropertyName &name, int value)
{
    if (name == "Colour") {
        foreach (QString speakerID, m_sublayers.keys()) {
            m_sublayers.value(speakerID)->setBaseColour(value);
        }
        setBaseColour(value);
    }
    else if (name == "Step (msec)")          setSmoothingStepMsec(value);
    else if (name == "Window left (msec)")   setWindowLeftMsec(value);
    else if (name == "Window right (msec)")  setWindowRightMsec(value);
    else SingleColourLayer::setProperty(name, value);
}

void MovingAverageLayer::setProperties(const QXmlAttributes &attributes)
{
    SingleColourLayer::setProperties(attributes);
}

void MovingAverageLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    foreach (QString speakerID, m_sublayers.keys()) {
        m_sublayers.value(speakerID)->paint(v, paint, rect);
    }
}

int MovingAverageLayer::getVerticalScaleWidth(View *v, bool, QPainter &paint) const
{
    if (!m_model) return 0;
    return LinearNumericalScale().getWidth(v, paint);
}

void MovingAverageLayer::paintVerticalScale(View *v, bool detailed, QPainter &paint, QRect rect) const
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

QString MovingAverageLayer::getFeatureDescription(View *v, QPoint &p) const
{
    QString ret;
    foreach (QString speakerID, m_sublayers.keys()) {
        QString s = m_sublayers.value(speakerID)->getFeatureDescription(v, p);
        if (!s.isEmpty())
            ret.append(QString("%1: %2").arg(speakerID).arg(QString(s).replace("\n", " "))).append("\n");
    }
    return ret.trimmed();
}

bool MovingAverageLayer::snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    return false;
}

bool MovingAverageLayer::isLayerScrollable(const View *v) const
{
    QPoint discard;
    return !v->shouldIlluminateLocalFeatures(this, discard);
}

bool MovingAverageLayer::getValueExtents(double &min, double &max, bool &, QString &) const
{
    return false;
}

bool MovingAverageLayer::getDisplayExtents(double &min, double &max) const
{
    return false;
}

bool MovingAverageLayer::setDisplayExtents(double min, double max)
{
    return false;
}

void MovingAverageLayer::toXml(QTextStream &stream, QString indent, QString extraAttributes) const
{

}

// ====================================================================================================================
// Properties
// ====================================================================================================================

QString MovingAverageLayer::getLevelID() const
{
    if (!m_model) return QString();
    return m_model->levelID();
}

void MovingAverageLayer::setLevelID(QString levelID)
{
    if (!m_model) return;
    m_model->setLevelID(levelID);
    updateSublayers();
    emit layerParametersChanged();
}

int MovingAverageLayer::getSmoothingStepMsec() const
{
    if (!m_model) return 0;
    return m_model->smoothingStepMsec();
}

void MovingAverageLayer::setSmoothingStepMsec(int step)
{
    if (!m_model) return;
    m_model->setSmoothingStepMsec(step);
    updateSublayers();
    emit layerParametersChanged();
}

int MovingAverageLayer::getWindowLeftMsec() const
{
    if (!m_model) return 0;
    return m_model->windowLeftMsec();
}

void MovingAverageLayer::setWindowLeftMsec(int left)
{
    if (!m_model) return;
    m_model->setWindowLeftMsec(left);
    updateSublayers();
    emit layerParametersChanged();
}

int MovingAverageLayer::getWindowRightMsec() const
{
    if (!m_model) return 0;
    return m_model->windowRightMsec();
}

void MovingAverageLayer::setWindowRightMsec(int right)
{
    if (!m_model) return;
    m_model->setWindowRightMsec(right);
    updateSublayers();
    emit layerParametersChanged();
}

int MovingAverageLayer::getYForValue(View *, double value) const
{
    return 0;
}

double MovingAverageLayer::getValueForY(View *, int y) const
{
    return 0.0;
}

QString MovingAverageLayer::getScaleUnits() const
{
    if (!m_model) return QString();
    return QString("%1/s").arg(m_model->levelID());
}

void MovingAverageLayer::getScaleExtents(View *, double &min, double &max, bool &log) const
{
}

int MovingAverageLayer::getDefaultColourHint(bool dark, bool &impose)
{
    return 0;
}
