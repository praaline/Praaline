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

QString convertSAMPAtoIPAUnicode(const QString &sampa)
{
    QString ipa(sampa);
    // Vowels
    ipa.replace("A", "\u0251");    //    A 	65  script a        0251 	593 	open back unrounded, Cardinal 5, Eng. start
    ipa.replace("{", "\u00E6");    //    { 	123 ae ligature 	00E6 	230 	near-open front unrounded, Eng. trap
    ipa.replace("6", "\u0250");    //    6 	54 	turned a        0250 	592 	open schwa, Ger. besser
    ipa.replace("Q", "\u0252");    //    Q 	81 	turned script a 0252 	594 	open back rounded, Eng. lot
    ipa.replace("E", "\u025B");    //    E 	69 	epsilon         025B 	603 	open-mid front unrounded, C3, Fr. même
    ipa.replace("@", "\u0259");    //    @ 	64 	turned e        0259 	601 	schwa, Eng. banana
    ipa.replace("3", "\u025C");    //    3 	51 	rev. epsilon 	025C 	604 	long mid central, Eng. nurse
    ipa.replace("I", "\u026A");    //    I 	73 	small cap I 	026A 	618 	lax close front unrounded, Eng. kit
    ipa.replace("O", "\u0254");    //    O 	79 	turned c        0254 	596 	open-mid back rounded, Eng. thought
    ipa.replace("2", "\u00F8");    //    2 	50 	o-slash         00F8 	248 	close-mid front rounded, Fr. deux
    ipa.replace("9", "\u0153");    //    9 	57 	oe ligature 	0153 	339 	open-mid front rounded, Fr. neuf
    ipa.replace("&", "\u0276");    //    & 	38 	s.c. OE lig. 	0276 	630 	open front rounded
    ipa.replace("U", "\u028A");    //    U 	85 	upsilon         028A	650 	lax close back rounded, Eng. foot
    ipa.replace("}", "\u0289");    //    } 	125 barred u        0289 	649 	close central rounded, Swedish sju
    ipa.replace("V", "\u028C");    //    V 	86 	turned v        028C 	652 	open-mid back unrounded, Eng. strut
    ipa.replace("Y", "\u028F");    //    Y 	89 	small cap Y 	028F 	655 	lax [y], Ger. hübsch
    // Consonants
    ipa.replace("B", "\u03B2");    //    B 	66 	beta            03B2 	946 	voiced bilabial fricative, Sp. cabo
    ipa.replace("C", "\u00E7");    //    C 	67 	c-cedilla       00E7 	231 	voiceless palatal fricative, Ger. ich
    ipa.replace("D", "\u00F0");    //    D 	68 	eth             00F0 	240 	voiced dental fricative, Eng. then
    ipa.replace("G", "\u0263");    //    G 	71 	gamma           0263 	611 	voiced velar fricative, Sp. fuego
    ipa.replace("L", "\u028E");    //    L 	76 	turned y        028E 	654 	palatal lateral, It. famiglia
    ipa.replace("J", "\u0272");    //    J 	74 	left-tail n 	0272 	626 	palatal nasal, Sp. año
    ipa.replace("N", "\u014B");    //    N 	78 	eng             014B 	331 	velar nasal, Eng. thing
    ipa.replace("R", "\u0281");    //    R 	82 	inv. s.c. R 	0281 	641 	vd. uvular fric. or trill, Fr. roi
    ipa.replace("S", "\u0283");    //    S 	83 	esh             0283 	643 	voiceless palatoalveolar fricative, Eng. ship
    ipa.replace("T", "\u03B8");    //    T 	84 	theta           03B8 	952 	voiceless dental fricative, Eng. thin
    ipa.replace("H", "\u0265");    //    H 	72 	turned h        0265 	613 	labial-palatal semivowel, Fr. huit
    ipa.replace("Z", "\u0292");    //    Z 	90 	ezh (yogh)      0292 	658 	vd. palatoalveolar fric., Eng. measure
    ipa.replace("?", "\u0294");    //    ? 	63 	dotless ?       0294 	660 	glottal stop, Ger. Verein, also Danish stød
    // Length, stress and tone marks
    ipa.replace(":", "\u02D0");    //    : 	58 	length mark 	02D0 	720 	length mark
    ipa.replace("\"", "\u02C8");   //    " 	34 	vertical stroke 02C8 	712 	primary stress *
    ipa.replace("%", "\u02CC");    //    % 	37	low vert. str. 	02CC 	716 	secondary stress
    // Diacritics
    ipa.replace("~", "\u0303");    //    O~ 126 sup. tilde      0303 	771 	nasalization, Fr. bon
    return ipa;
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
        QString label = convertSAMPAtoIPAUnicode(p.label);
        int boxMaxWidth = v->getXForFrame(p.frame + p.duration) - x - 2;
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
        if (y >= getYForValue(v, segmentModel->getf0MinimumST())) continue;
        paint.setPen(QPen(Qt::gray, 1, Qt::DotLine));
        paint.drawLine(xStart, y, xEnd, y);
    }

    // Draw pitch range
    // ----------------------------------------------------------------------------------------------------------------


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
    QPen penTonalSegmentIlluminated(Qt::red, 3, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);

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
            paint.setPen(penTonalSegment);
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

