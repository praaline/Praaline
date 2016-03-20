#include "ProsogramLayer.h"

#include "base/RealTime.h"
#include "svcore/data/model/Model.h"
#include "svcore/base/Profiler.h"
#include "svgui/layer/ColourDatabase.h"
#include "svgui/layer/LinearNumericalScale.h"
#include "svgui/layer/SingleColourLayer.h"
#include "svgui/layer/VerticalScaleLayer.h"
#include "svgui/layer/TimeValueLayer.h"
#include "svgui/view/View.h"

#include "pngui/model/annotation/ProsogramModel.h"

#include <QPainter>
#include <QMouseEvent>
#include <QInputDialog>
#include <QTextStream>
#include <QMessageBox>

#include <iostream>
#include <cmath>

ProsogramLayer::ProsogramLayer() :
    SingleColourLayer(),
    m_model(0), m_layerIntensity(new TimeValueLayer), m_layerPitch(new TimeValueLayer)
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
    return list;
}

QString ProsogramLayer::getPropertyLabel(const PropertyName &name) const
{
    return SingleColourLayer::getPropertyLabel(name);
}

Layer::PropertyType ProsogramLayer::getPropertyType(const PropertyName &name) const
{
    return SingleColourLayer::getPropertyType(name);
}

int ProsogramLayer::getPropertyRangeAndValue(const PropertyName &name,
                                              int *min, int *max, int *deflt) const
{
    return SingleColourLayer::getPropertyRangeAndValue(name, min, max, deflt);
}

QString ProsogramLayer::getPropertyValueLabel(const PropertyName &name,
                                           int value) const
{
    return SingleColourLayer::getPropertyValueLabel(name, value);
}

void ProsogramLayer::setProperty(const PropertyName &name, int value)
{
    SingleColourLayer::setProperty(name, value);
}

void ProsogramLayer::setProperties(const QXmlAttributes &attributes)
{
    SingleColourLayer::setProperties(attributes);
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
    int w = getVerticalScaleWidth(v, false, paint);
    int h = v->height();
    getScaleExtents(v, min, max, logarithmic);
    LinearNumericalScale().paintVertical(v, this, paint, 0, min, max);
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
            min = m_model->segmentModel()->getf0MinimumHz();
            max = m_model->segmentModel()->getf0MaximumST();
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
    unit = getScaleUnits();
    if (max == min) {
        max = max + 5;
        min = min - 5;
    } else {
        double margin = (max - min) / 10.0;
        if (margin < 5.0) margin = 5.0;
        max = max + margin;
        min = min - margin;
    }
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
    int h = v->height() * 0.9;
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

//ProsogramModel::PointList
//ProsogramLayer::getLocalPoints(View *v, int x, int y) const
//{
//    if (!m_model) return ProsogramModel::PointList();

//    sv_frame_t frame0 = v->getFrameForX(-150);
//    sv_frame_t frame1 = v->getFrameForX(v->width() + 150);

//    ProsogramModel::PointList points(m_model->getPoints(frame0, frame1));

//    ProsogramModel::PointList rv;
//    QFontMetrics metrics = QFontMetrics(QFont());

//    for (ProsogramModel::PointList::iterator i = points.begin();
//         i != points.end(); ++i) {

//        const ProsogramModel::Point &p(*i);

//        int px = v->getXForFrame(p.frame);
//        int py = getYForHeight(v, p.height);

//        QString label = p.label;
//        if (label == "") {
//            label = tr("<no text>");
//        }

//        QRect rect = metrics.boundingRect
//                (QRect(0, 0, 150, 200),
//                 Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, label);

//        if (py + rect.height() > v->height()) {
//            if (rect.height() > v->height()) py = 0;
//            else py = v->height() - rect.height() - 1;
//        }

//        if (x >= px && x < px + rect.width() &&
//                y >= py && y < py + rect.height()) {
//            rv.insert(p);
//        }
//    }

//    return rv;
//}

QString ProsogramLayer::getFeatureDescription(View *v, QPoint &pos) const
{
    return "";
//    int x = pos.x();

//    if (!m_model || !m_model->getSampleRate()) return "";

//    ProsogramModel::PointList points = getLocalPoints(v, x, pos.y());

//    if (points.empty()) {
//        if (!m_model->isReady()) {
//            return tr("In progress");
//        } else {
//            return "";
//        }
//    }

//    sv_frame_t useFrame = points.begin()->frame;

//    RealTime rt = RealTime::frame2RealTime(useFrame, m_model->getSampleRate());

//    QString text;

//    if (points.begin()->label == "") {
//        text = QString(tr("Time:\t%1\nHeight:\t%2\nLabel:\t%3"))
//                .arg(rt.toText(true).c_str())
//                .arg(points.begin()->height)
//                .arg(points.begin()->label);
//    }

//    pos = QPoint(v->getXForFrame(useFrame),
//                 getYForHeight(v, points.begin()->height));
//    return text;
}


bool ProsogramLayer::snapToFeatureFrame(View *v, sv_frame_t &frame,
                                        int &resolution,
                                        SnapType snap) const
{
    if (!m_model) {
        return Layer::snapToFeatureFrame(v, frame, resolution, snap);
    }

//    resolution = m_model->getResolution();
//    ProsogramModel::PointList points;

//    if (snap == SnapNeighbouring) {

//        points = getLocalPoints(v, v->getXForFrame(frame), -1);
//        if (points.empty()) return false;
//        frame = points.begin()->frame;
//        return true;
//    }

//    points = m_model->getPoints(frame, frame);
//    sv_frame_t snapped = frame;
//    bool found = false;

//    for (ProsogramModel::PointList::const_iterator i = points.begin();
//         i != points.end(); ++i) {

//        if (snap == SnapRight) {

//            if (i->frame > frame) {
//                snapped = i->frame;
//                found = true;
//                break;
//            }

//        } else if (snap == SnapLeft) {

//            if (i->frame <= frame) {
//                snapped = i->frame;
//                found = true; // don't break, as the next may be better
//            } else {
//                break;
//            }

//        } else { // nearest

//            ProsogramModel::PointList::const_iterator j = i;
//            ++j;

//            if (j == points.end()) {

//                snapped = i->frame;
//                found = true;
//                break;

//            } else if (j->frame >= frame) {

//                if (j->frame - frame < frame - i->frame) {
//                    snapped = j->frame;
//                } else {
//                    snapped = i->frame;
//                }
//                found = true;
//                break;
//            }
//        }
//    }

//    frame = snapped;
//    return found;
    return false;
}

// ====================================================================================================================

void ProsogramLayer::paintAnnotationTier(View *v, QPainter &paint, sv_frame_t frame0, sv_frame_t frame1,
                                         int tier_y0, int tier_y1, Qt::PenStyle verticalLinePenstyle,
                                         QPointer<AnnotationGridPointModel> model) const
{
    paint.setBrush(Qt::black);
    if (!model) return;

    AnnotationGridPointModel::PointList intervals = model->getPoints(frame0, frame1);
    for (AnnotationGridPointModel::PointList::const_iterator i = intervals.begin(); i != intervals.end(); ++i) {
        const AnnotationGridPointModel::Point &p(*i);
        int x = v->getXForFrame(p.frame);
        paint.setRenderHint(QPainter::Antialiasing, false);
        paint.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        paint.drawLine(x, tier_y0, x, tier_y1);
        paint.setPen(QPen(Qt::black, 1, verticalLinePenstyle));
        paint.drawLine(x, 0, x, tier_y0);

        if (p.label.isEmpty()) continue;
        int boxMaxWidth = v->getXForFrame(p.frame + p.duration) - x - 2;
        int boxMaxHeight = tier_y1 - tier_y0 - 2;
        QRect textRect = QRect(x + 1, tier_y0 + 1, boxMaxWidth, boxMaxHeight);
        QRect boundingRect = paint.fontMetrics().boundingRect(QRect(0, 0, boxMaxWidth, boxMaxHeight),
                                                              Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                                                              p.label);
        paint.setRenderHint(QPainter::Antialiasing, true);
        if (boundingRect.width() > textRect.width())
            paint.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, p.label);
        else
            paint.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap, p.label);
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

    paint.save();

    // Draw horizontal scale
    // ----------------------------------------------------------------------------------------------------------------
    paint.setRenderHint(QPainter::Antialiasing, false);
    double min, max; bool logarithmic;
    getScaleExtents(v, min, max, logarithmic);
    int minR = (int) min; int maxR = (int) max;
    for (int ST = minR; ST < maxR; ST += 2) {
        int y = getYForValue(v, ST);
        if (y >= getYForValue(v, segmentModel->getf0MinimumST())) continue;
        paint.setPen(QPen(Qt::gray, 1, Qt::DotLine));
        paint.drawLine(xStart, y, xEnd, y);
    }

    // Draw intensity and pitch
    // ----------------------------------------------------------------------------------------------------------------
    m_layerIntensity->paint(v, paint, rect);
    m_layerPitch->setDisplayExtents(min, max);
    m_layerPitch->paint(v, paint, rect);

    // Draw phones + syllables
    // ----------------------------------------------------------------------------------------------------------------
    QPointer<AnnotationGridPointModel> phoneModel = m_model->phoneModel();
    QPointer<AnnotationGridPointModel> syllModel = m_model->syllModel();
    int phone_y0 = getYForValue(v, segmentModel->getf0MinimumST());
    int tiersHeight = (v->height() - phone_y0 - 2) / 2;
    int phone_y1 = phone_y0 + tiersHeight;
    int syll_y0 = phone_y1 + 1;
    int syll_y1 = v->height();
    paint.setRenderHint(QPainter::Antialiasing, false);
    paint.setPen(QPen(Qt::black, 1, Qt::SolidLine));
    paint.drawLine(xStart, phone_y0, xEnd, phone_y0);
    paintAnnotationTier(v, paint, frame0, frame1, phone_y0, phone_y1, Qt::DotLine, phoneModel);
    paint.setRenderHint(QPainter::Antialiasing, false);
    paint.setPen(QPen(Qt::black, 1, Qt::SolidLine));
    paint.drawLine(xStart, syll_y0, xEnd, syll_y0);
    paintAnnotationTier(v, paint, frame0, frame1, syll_y0, syll_y1, Qt::DashLine, syllModel);

    // Draw voiced-unvoiced regions
    // ----------------------------------------------------------------------------------------------------------------
    QBrush brushVoiced(Qt::darkGray, Qt::DiagCrossPattern);
    QPointer<AnnotationGridPointModel> vuvRegionsModel = m_model->vuvRegionModel();
    AnnotationGridPointModel::PointList vuvRegions = vuvRegionsModel->getPoints(frame0, frame1);
    for (AnnotationGridPointModel::PointList::const_iterator i = vuvRegions.begin(); i != vuvRegions.end(); ++i) {
        const AnnotationGridPointModel::Point &p(*i);
        int x = v->getXForFrame(p.frame);
        int w = v->getXForFrame(p.frame + p.duration) - x;
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
    ProsogramTonalSegmentModel::PointList segments = segmentModel->getPoints(frame0, frame1);
    for (ProsogramTonalSegmentModel::PointList::const_iterator i = segments.begin(); i != segments.end(); ++i) {
        const ProsogramTonalSegmentModel::Point &p(*i);

        int x0 = v->getXForFrame(p.frame);
        int x1 = v->getXForFrame(p.frame + p.duration);
        int y0 = getYForValue(v, p.f0StartST());
        int y1 = getYForValue(v, p.f0EndST());

        // Draw tonal segment
        paint.setRenderHint(QPainter::Antialiasing, true);
        paint.setPen(penTonalSegment);
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

