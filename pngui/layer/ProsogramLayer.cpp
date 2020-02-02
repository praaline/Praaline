#include "ProsogramLayer.h"

#include "PraalineCore/Base/RealTime.h"
#include "PraalineCore/Language/PhoneticAlphabet.h"
using namespace Praaline::Core;

#include "svcore/data/model/Model.h"
#include "svcore/base/Profiler.h"
#include "svgui/layer/ColourDatabase.h"
#include "svgui/layer/LinearNumericalScale.h"
#include "svgui/layer/SingleColourLayer.h"
#include "svgui/layer/VerticalScaleLayer.h"
#include "svgui/layer/TimeValueLayer.h"
#include "svgui/view/View.h"

#include "pngui/model/visualiser/ProsogramModel.h"

#include <QPainter>
#include <QMouseEvent>
#include <QInputDialog>
#include <QTextStream>
#include <QMessageBox>

#include <iostream>
#include <cmath>

ProsogramLayer::ProsogramLayer() :
    SingleColourLayer(),
    m_model(0), m_layerIntensity(new TimeValueLayer), m_layerPitch(new TimeValueLayer),
    m_semitonesSetAsideForTiers(20.0),
    m_showPitch(true), m_showPitchRange(true), m_showIntensity(true), m_showVerticalLines(true),
    m_showPhoneTier(true), m_showSyllTier(true), m_showTonalAnnotationTier(true)
{
    m_layerIntensity->setBaseColour(ColourDatabase::getInstance()->getColourIndex(QString("Green")));
    m_layerIntensity->setPlotStyle(TimeValueLayer::PlotCurve);
    m_layerPitch->setBaseColour(ColourDatabase::getInstance()->getColourIndex(QString("Blue")));
    m_layerPitch->setPlotStyle(TimeValueLayer::PlotDiscreteCurves);
}

ProsogramLayer::~ProsogramLayer()
{
    delete m_layerIntensity;
    delete m_layerPitch;
}

bool ProsogramLayer::trySetModel(Model *model)
{
    if (trySetModelHelper<ProsogramLayer, ProsogramModel>(this, model))
        return true;
    return false;
}

void
ProsogramLayer::setModel(ProsogramModel *model)
{
    if (m_model == model) return;
    m_model = model;
    connectSignals(m_model);
    m_layerIntensity->setModel(model->intensityModel());
    m_layerPitch->setModel(model->pitchModel());
    // cerr << "ProsogramLayer::setModel(" << model << ")" << endl;
    emit modelReplaced();
}

// ====================================================================================================================
// Propeties
// ====================================================================================================================

Layer::PropertyList ProsogramLayer::getProperties() const
{
    PropertyList list = SingleColourLayer::getProperties();
    list.push_back("Show Pitch");
    list.push_back("Show Pitch Range");
    list.push_back("Show Intensity");
    list.push_back("Show Vertical Lines");
    return list;
}

QString ProsogramLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Show Pitch")           return tr("Show Pitch");
    if (name == "Show Pitch Range")     return tr("Show Pitch Range");
    if (name == "Show Intensity")       return tr("Show Intensity");
    if (name == "Show Vertical Lines")  return tr("Show Vertical Lines");
    return SingleColourLayer::getPropertyLabel(name);
}

Layer::PropertyType ProsogramLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Show Pitch")           return ToggleProperty;
    if (name == "Show Pitch Range")     return ToggleProperty;
    if (name == "Show Intensity")       return ToggleProperty;
    if (name == "Show Vertical Lines")  return ToggleProperty;
    return SingleColourLayer::getPropertyType(name);
}

int ProsogramLayer::getPropertyRangeAndValue(const PropertyName &name, int *min, int *max, int *deflt) const
{
    int garbage0(0), garbage1(0), garbage2(0);
    if (!min) min = &garbage0;
    if (!max) max = &garbage1;
    if (!deflt) deflt = &garbage2;
    if (name == "Show Pitch")           return m_showPitch ? 1 : 0;
    if (name == "Show Pitch Range")     return m_showPitchRange ? 1 : 0;
    if (name == "Show Intensity")       return m_showIntensity ? 1 : 0;
    if (name == "Show Vertical Lines")  return m_showVerticalLines ? 1 : 0;
    return SingleColourLayer::getPropertyRangeAndValue(name, min, max, deflt);
}

QString ProsogramLayer::getPropertyValueLabel(const PropertyName &name, int value) const
{
    return SingleColourLayer::getPropertyValueLabel(name, value);
}

void ProsogramLayer::setProperty(const PropertyName &name, int value)
{
    if      (name == "Show Pitch")           setShowPitch(value ? true : false);
    else if (name == "Show Pitch Range")     setShowPitchRange(value ? true : false);
    else if (name == "Show Intensity")       setShowIntensity(value ? true : false);
    else if (name == "Show Vertical Lines")  setShowVerticalLines(value ? true : false);
    else SingleColourLayer::setProperty(name, value);
}

void ProsogramLayer::setProperties(const QXmlAttributes &attributes)
{
    SingleColourLayer::setProperties(attributes);
    bool showPitch = (attributes.value("showPitch").trimmed() == "true");
    setShowPitch(showPitch);
    bool showPitchRange = (attributes.value("showPitchRange").trimmed() == "true");
    setShowPitchRange(showPitchRange);
    bool showIntensity = (attributes.value("showIntensity").trimmed() == "true");
    setShowIntensity(showIntensity);
    bool showVerticalLines = (attributes.value("showVerticalLines").trimmed() == "true");
    setShowVerticalLines(showVerticalLines);
}

void ProsogramLayer::setShowPitch(bool show)
{
    if (m_showPitch == show) return;
    m_showPitch = show;
    emit layerParametersChanged();
}

void ProsogramLayer::setShowPitchRange(bool show)
{
    if (m_showPitchRange == show) return;
    m_showPitchRange = show;
    emit layerParametersChanged();
}

void ProsogramLayer::setShowIntensity(bool show)
{
    if (m_showIntensity == show) return;
    m_showIntensity = show;
    emit layerParametersChanged();
}

void ProsogramLayer::setShowVerticalLines(bool show)
{
    if (m_showVerticalLines == show) return;
    m_showVerticalLines = show;
    emit layerParametersChanged();
}

// ====================================================================================================================
// Vertical scale
// ====================================================================================================================
int ProsogramLayer::getVerticalScaleWidth(View *v, bool, QPainter &paint) const
{
    if (!m_model) return 0;
    return LinearNumericalScale().getWidth(v, paint);
}

void ProsogramLayer::paintVerticalScale(View *v, bool, QPainter &paint, QRect rect) const
{
    if (!m_model) return;
    double min, max;
    bool logarithmic;
    // int w = getVerticalScaleWidth(v, false, paint);
    // int h = v->height();
    getScaleExtents(v, min, max, logarithmic);
    int n = (max - min) / 10.0; // ticks every 10 ST
    LinearNumericalScale().paintVertical(v, this, paint, 0, min, max, n);
    paint.drawText(5, 5 + paint.fontMetrics().ascent(), "ST");
}

void ProsogramLayer::getScaleExtents(View *v, double &min, double &max, bool &log) const
{
    min = 0.0;
    max = 0.0;
    log = false;
    bool autoAlign = true;
    if (autoAlign) {
        if (!v->getValueExtents(getScaleUnits(), min, max, log)) {
            // sensible defaults for ST scale if the model is not available
            min = 70.0 - m_semitonesSetAsideForTiers;
            max = 110.0;
        }
    }
    else {
        getDisplayExtents(min, max);
    }
}

bool ProsogramLayer::getValueExtents(double &min, double &max, bool &logarithmic, QString &unit) const
{
    if (!m_model) return false;
    min = m_model->segmentModel()->getf0MinimumST();
    max = m_model->segmentModel()->getf0MaximumST();
    // round up to the decade
    min = floor((min - 5.0) / 10.0) * 10.0;
    max = ceil((max + 5.0) / 10.0) * 10.0;
    min = min - m_semitonesSetAsideForTiers;
    if (max == min) {
        max = max + 10.0;
        min = min - 10.0;
    }
    unit = getScaleUnits();
    logarithmic = false;
    return true;
}

bool ProsogramLayer::getDisplayExtents(double &min, double &max) const
{
    return false;
}

bool ProsogramLayer::setDisplayExtents(double min, double max)
{
    return false;
}

int ProsogramLayer::getYForValue(View *v, double value) const
{
    double min = 0.0, max = 0.0;
    bool logarithmic = false;
    int h = v->height();
    getScaleExtents(v, min, max, logarithmic);
    return int(h - ((value - min) * h) / (max - min));
}

double ProsogramLayer::getValueForY(View *, int y) const
{

}

QString ProsogramLayer::getScaleUnits() const
{
    return "ST"; // semitones re 1 Hz
}


// ====================================================================================================================

bool ProsogramLayer::isLayerScrollable(const View *v) const
{
    QPoint discard;
    return !v->shouldIlluminateLocalFeatures(this, discard);
}

ProsogramTonalSegmentModel::PointList ProsogramLayer::getLocalTonalSegments(View *v, int x) const
{
    if (!m_model || !m_model->segmentModel()) return ProsogramTonalSegmentModel::PointList();

    sv_frame_t frame = v->getFrameForX(x);

    ProsogramTonalSegmentModel::PointList onPoints = m_model->segmentModel()->getPoints(frame);
    if (!onPoints.empty()) return onPoints;

    ProsogramTonalSegmentModel::PointList prevPoints = m_model->segmentModel()->getPreviousPoints(frame);
    return prevPoints;
}

QString ProsogramLayer::getFeatureDescription(View *v, QPoint &pos) const
{
    int x = pos.x();
    if (!m_model || !m_model->getSampleRate() || !m_model->segmentModel()) return "";

    ProsogramTonalSegmentModel::PointList tonalSegments = getLocalTonalSegments(v, x);

    if (tonalSegments.empty()) {
        if (!m_model->isReady()) {
            return tr("In progress");
        } else {
            return "";
        }
    }

    ProsogramTonalSegmentModel::Point segment = *(tonalSegments.begin());

    RealTime rt = RealTime::frame2RealTime(segment.frame, m_model->getSampleRate());
    RealTime rd = RealTime::frame2RealTime(segment.duration, m_model->getSampleRate());

    QString text = QString(tr("Nucleus at Time:\t%1\tDuration:\t%2\nF0 start:\t%3 ST (%4 Hz)\nF0 end:\t%5 ST (%6 Hz)"))
            .arg(rt.toText(true).c_str()).arg(rd.toText(true).c_str())
            .arg(segment.f0StartST()).arg(segment.f0StartHz)
            .arg(segment.f0EndST()).arg(segment.f0EndHz);

    pos = QPoint(v->getXForFrame(segment.frame), pos.y());
    return text;
}


bool ProsogramLayer::snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    if (!m_model  || !m_model->segmentModel()) {
        return Layer::snapToFeatureFrame(v, frame, resolution, snap, y);
    }

    resolution = m_model->segmentModel()->getResolution();
    ProsogramTonalSegmentModel::PointList points;

    if (snap == SnapNeighbouring) {
        points = getLocalTonalSegments(v, v->getXForFrame(frame));
        if (points.empty()) return false;
        frame = points.begin()->frame;
        return true;
    }

    points = m_model->segmentModel()->getPoints(frame, frame);
    sv_frame_t snapped = frame;
    bool found = false;

    for (ProsogramTonalSegmentModel::PointList::const_iterator i = points.begin(); i != points.end(); ++i) {
        if (snap == SnapRight) {
            // The best frame to snap to is the end frame of whichever feature we would have snapped to the start frame of if
            // we had been snapping left.
            if (i->frame <= frame) {
                if (i->frame + i->duration > frame) {
                    snapped = i->frame + i->duration;
                    found = true; // don't break, as the next may be better
                }
            } else {
                if (!found) {
                    snapped = i->frame;
                    found = true;
                }
                break;
            }
        } else if (snap == SnapLeft) {
            if (i->frame <= frame) {
                snapped = i->frame;
                found = true; // don't break, as the next may be better
            } else {
                break;
            }
        } else { // nearest
            ProsogramTonalSegmentModel::PointList::const_iterator j = i;
            ++j;
            if (j == points.end()) {
                snapped = i->frame;
                found = true;
                break;
            } else if (j->frame >= frame) {
                if (j->frame - frame < frame - i->frame) {
                    snapped = j->frame;
                } else {
                    snapped = i->frame;
                }
                found = true;
                break;
            }
        }
    }

    frame = snapped;
    return found;
}

// ====================================================================================================================

void ProsogramLayer::paintAnnotationTier(View *v, QPainter &paint, sv_frame_t frame0, sv_frame_t frame1,
                                         int tier_y0, int tier_y1, Qt::PenStyle verticalLinePenstyle,
                                         QPointer<AnnotationGridPointModel> model, bool IPA) const
{
    paint.setBrush(Qt::black);
    if (!model) return;

    AnnotationGridPointModel::PointList intervals = model->getPoints(frame0, frame1);
    for (AnnotationGridPointModel::PointList::const_iterator i = intervals.begin(); i != intervals.end(); ++i) {
        const AnnotationGridPointModel::Point &p(*i);
        // Find end frame
        sv_frame_t nextFrame;
        AnnotationGridPointModel::PointList::const_iterator next(i);
        next++;
        if (next != intervals.end()) nextFrame = (*next).frame; else nextFrame = model->getEndFrame();
        // Find x-coordinate for point
        int x = v->getXForFrame(p.frame);
        paint.setRenderHint(QPainter::Antialiasing, false);
        // Boundary line inside tier
        paint.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        paint.drawLine(x, tier_y0, x, tier_y1);
        // Vertical lines (optionally)
        if (m_showVerticalLines) {
            paint.setPen(QPen(Qt::black, 1, verticalLinePenstyle));
            paint.drawLine(x, 0, x, tier_y0);
        }
        // Label (convert SAMPA to IPA)
        if (p.label.isEmpty()) continue;
        QString label = (IPA) ? PhoneticAlphabet::convertSAMPAtoIPAUnicode(p.label) : p.label;
        int boxMaxWidth = v->getXForFrame(nextFrame) - x - 2;
        int boxMaxHeight = tier_y1 - tier_y0 - 2;
        QRect textRect = QRect(x + 1, tier_y0 + 1, boxMaxWidth, boxMaxHeight);
        QRect boundingRect = paint.fontMetrics().boundingRect(QRect(0, 0, boxMaxWidth, boxMaxHeight),
                                                              Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                                                              label);
        paint.setRenderHint(QPainter::Antialiasing, true);
        if (boundingRect.width() > textRect.width())
            paint.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, label);
        else
            paint.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap, label);
    }
}

void ProsogramLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    if (!m_model || !m_model->isOK()) return;

    sv_samplerate_t sampleRate = m_model->getSampleRate();
    if (!sampleRate) return;

    int xStart = rect.left();
    int xEnd = rect.right();

    int x0 = rect.left(), x1 = rect.right();
    sv_frame_t frame0 = v->getFrameForX(x0);
    sv_frame_t frame1 = v->getFrameForX(x1);

    QPointer<ProsogramTonalSegmentModel> segmentModel = m_model->segmentModel();
    if (!segmentModel) return;

    // Illuminate tonal segments where appropriate
    // ----------------------------------------------------------------------------------------------------------------
    QPoint localPos;
    long illuminateFrame = -1;
    if (v->shouldIlluminateLocalFeatures(this, localPos)) {
        ProsogramTonalSegmentModel::PointList localPoints = getLocalTonalSegments(v, localPos.x());
        if (!localPoints.empty()) illuminateFrame = localPoints.begin()->frame;
    }

    paint.save();

    // Draw horizontal scale
    // ----------------------------------------------------------------------------------------------------------------
    paint.setRenderHint(QPainter::Antialiasing, false);
    double min, max; bool logarithmic;
    getScaleExtents(v, min, max, logarithmic);
    int minR = (int) min; int maxR = (int) max;
    for (int ST = minR; ST < maxR; ST += 2) {
        int y = getYForValue(v, ST);
        if (y >= getYForValue(v, min + m_semitonesSetAsideForTiers)) continue;
        paint.setPen(QPen(Qt::gray, 1, Qt::DotLine));
        paint.drawLine(xStart, y, xEnd, y);
    }

    // Draw pitch range
    // ----------------------------------------------------------------------------------------------------------------
    if (m_showPitchRange) {
        QList<double> values;
        values << m_model->pitchRangeBottomST() << m_model->pitchRangeMedianST() << m_model->pitchRangeTopST();
        foreach (double ST, values) {
            if (ST > 0) {
                int y = getYForValue(v, ST);
                paint.setPen(QPen(Qt::magenta, 1, Qt::DashLine));
                paint.drawLine(xStart, y, xEnd, y);
            }
        }
    }

    // Draw intensity and pitch
    // ----------------------------------------------------------------------------------------------------------------
    if (m_showIntensity) {
        double intensityMin(0.0), intensityMinValues(0.0), intensityMax(0.0);
        m_layerIntensity->getDisplayExtents(intensityMin, intensityMax);
        intensityMinValues = intensityMin;
        int y0_intensity = m_layerIntensity->getYForValue(v, intensityMin);
        while ((y0_intensity > getYForValue(v, min + m_semitonesSetAsideForTiers)) && (intensityMin > 0.0)) {
            intensityMin = intensityMin - (intensityMax - intensityMin) * 0.05;
            m_layerIntensity->setDisplayExtents(intensityMin, intensityMax);
            y0_intensity = m_layerIntensity->getYForValue(v, intensityMinValues);
        }
        m_layerIntensity->paint(v, paint, rect);
    }
    if (m_showPitch) {
        m_layerPitch->setDisplayExtents(min, max);
        m_layerPitch->paint(v, paint, rect);
    }

    // Draw phones + syllables
    // ----------------------------------------------------------------------------------------------------------------
    QPointer<AnnotationGridPointModel> phoneModel = m_model->phoneModel();
    QPointer<AnnotationGridPointModel> syllModel = m_model->syllModel();
    QPointer<AnnotationGridPointModel> tonalModel = m_model->tonalModel();
    // 20 ST (or a custom value) of space is reserved for tier annotations
    int countTiers(0);
    if (m_showPhoneTier) countTiers++;
    if (m_showSyllTier) countTiers++;
    if (m_showTonalAnnotationTier) countTiers++;
    int tier_y0 = getYForValue(v, min + m_semitonesSetAsideForTiers);
    int tier_height = (v->height() - tier_y0 - countTiers) / countTiers;
    int tier_y1 = tier_y0 + tier_height;
    if (m_showPhoneTier) {
        paint.setRenderHint(QPainter::Antialiasing, false);
        paint.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        paint.drawLine(xStart, tier_y0, xEnd, tier_y0);
        paintAnnotationTier(v, paint, frame0, frame1, tier_y0, tier_y1, Qt::DotLine, phoneModel, true);
        tier_y0 = tier_y1 + 1;
        tier_y1 = tier_y0 + tier_height;
    }
    if (m_showSyllTier) {
        paint.setRenderHint(QPainter::Antialiasing, false);
        paint.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        paint.drawLine(xStart, tier_y0, xEnd, tier_y0);
        paintAnnotationTier(v, paint, frame0, frame1, tier_y0, tier_y1, Qt::DashLine, syllModel, true);
        tier_y0 = tier_y1 + 1;
        tier_y1 = tier_y0 + tier_height;
    }
    if (m_showSyllTier) {
        paint.setRenderHint(QPainter::Antialiasing, false);
        paint.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        paint.drawLine(xStart, tier_y0, xEnd, tier_y0);
        paintAnnotationTier(v, paint, frame0, frame1, tier_y0, tier_y1, Qt::DashLine, tonalModel, false);
    }

    // Draw voiced-unvoiced regions
    // ----------------------------------------------------------------------------------------------------------------
    QBrush brushVoiced(Qt::darkGray, Qt::DiagCrossPattern);
    QPointer<AnnotationGridPointModel> vuvRegionsModel = m_model->vuvRegionModel();
    AnnotationGridPointModel::PointList vuvRegions = vuvRegionsModel->getPoints(frame0, frame1);
    for (AnnotationGridPointModel::PointList::const_iterator i = vuvRegions.begin(); i != vuvRegions.end(); ++i) {
        const AnnotationGridPointModel::Point &p(*i);
        sv_frame_t nextFrame;
        AnnotationGridPointModel::PointList::const_iterator next(i);
        next++;
        if (next != vuvRegions.end()) nextFrame = (*next).frame; else nextFrame = vuvRegionsModel->getEndFrame();
        int x = v->getXForFrame(p.frame);
        int w = v->getXForFrame(nextFrame) - x;
        if (p.label != "V") continue;
        paint.setRenderHint(QPainter::Antialiasing, false);
        paint.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        paint.setBrush(brushVoiced);
        paint.drawRect(x, 2, w, 5);
        // paint.fillRect(x, 2, w, 4, brushVoiced);
    }

    // Draw tonal segments
    // ----------------------------------------------------------------------------------------------------------------
    QPen penTonalSegment(Qt::black, 3, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
    QPen penTonalSegmentIlluminated(Qt::magenta, 3, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
    QPen penTonalSegmentRising(Qt::darkRed, 3, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
    QPen penTonalSegmentFalling(Qt::darkGreen, 3, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);

    ProsogramTonalSegmentModel::PointList segments = segmentModel->getPoints(frame0, frame1);
    for (ProsogramTonalSegmentModel::PointList::const_iterator i = segments.begin(); i != segments.end(); ++i) {
        const ProsogramTonalSegmentModel::Point &p(*i);

        int x0 = v->getXForFrame(p.frame);
        int x1 = v->getXForFrame(p.frame + p.duration);
        int y0 = getYForValue(v, p.f0StartST());
        int y1 = getYForValue(v, p.f0EndST());

        // Draw tonal segment
        paint.setRenderHint(QPainter::Antialiasing, true);
        if (p.frame == illuminateFrame) {
            paint.setPen(penTonalSegmentIlluminated);
        } else {
            if      (p.rising(3.0))  paint.setPen(penTonalSegmentRising);
            else if (p.falling(3.0)) paint.setPen(penTonalSegmentFalling);
            else                     paint.setPen(penTonalSegment);
        }
        paint.setBrush(getBaseQColor());
        paint.drawLine(x0, y0, x1, y1);

        // Indicate stylised nucleus on voiced-unvoiced regions
        paint.setRenderHint(QPainter::Antialiasing, false);
        paint.setPen(QPen(Qt::red, 1, Qt::SolidLine));
        paint.setBrush(Qt::NoBrush);
        paint.drawRect(x0, 1, x1 - x0, 7);
    }

    paint.restore();
}

// ====================================================================================================================

int ProsogramLayer::getDefaultColourHint(bool darkbg, bool &impose)
{
    impose = false;
    return ColourDatabase::getInstance()->getColourIndex
            (QString(darkbg ? "White" : "Black"));
}

void ProsogramLayer::toXml(QTextStream &stream,
                           QString indent, QString extraAttributes) const
{
    SingleColourLayer::toXml(stream, indent, extraAttributes);
}

