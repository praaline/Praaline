#include "AnnotationGridLayer.h"

#include "base/RealTime.h"
#include "svcore/data/model/Model.h"
#include "svcore/base/Profiler.h"
#include "svgui/layer/ColourDatabase.h"
#include "svgui/view/View.h"

#include "pngui/model/visualiser/AnnotationGridModel.h"

#include <QPainter>
#include <QMouseEvent>
#include <QInputDialog>
#include <QTextStream>
#include <QMessageBox>

#include <iostream>
#include <cmath>

AnnotationGridLayer::AnnotationGridLayer() :
    SingleColourLayer(),
    m_model(0),
    m_plotStyle(PlotSpeakersThenLevelAttributes),
    m_boundaryEditing(false),
    m_boundaryEditingPoint(AnnotationGridPointModel::Point(0)),
    m_boundaryEditingCommand(0),
    m_textEditing(false),
    m_textEditor(0),
    m_textEditorPoint(AnnotationGridPointModel::Point(0))
{
    m_textEditor = new QLineEdit();
    m_textEditor->setVisible(false);
    m_textEditor->setFrame(false);
    m_textEditor->setAlignment(Qt::AlignHCenter);
    connect(m_textEditor, SIGNAL(editingFinished()), this, SLOT(textEditingFinished()));
}

AnnotationGridLayer::~AnnotationGridLayer()
{
    textEditingFinished();
    if (m_textEditor) delete m_textEditor;
}

bool AnnotationGridLayer::trySetModel(Model *model)
{
    textEditingFinished();
    if (trySetModelHelper<AnnotationGridLayer, AnnotationGridModel>(this, model))
        return true;
    return false;
}

void AnnotationGridLayer::setModel(AnnotationGridModel *model)
{
    if (m_model == model) return;
    textEditingFinished();
    m_model = model;
    connectSignals(m_model);
    if (m_plotStyle == PlotBlendedSpeakers || m_plotStyle == PlotLevelAttributesThenSpeakers)
        m_tierTuples = m_model->tierTuples(AnnotationGridModel::LayoutLevelAttributesThenSpeakers);
    else
        m_tierTuples = m_model->tierTuples(AnnotationGridModel::LayoutSpeakersThenLevelAttributes);
    // cerr << "AnnotationGridLayer::setModel(" << model << ")" << endl;
    emit modelReplaced();
}

// ==============================================================================================================================
// Properties
// ==============================================================================================================================

Layer::PropertyList AnnotationGridLayer::getProperties() const
{
    PropertyList list = SingleColourLayer::getProperties();
    list.push_back("Structure");
    return list;
}

QString AnnotationGridLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Structure") return tr("Structure");
    return SingleColourLayer::getPropertyLabel(name);
}

Layer::PropertyType AnnotationGridLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Structure") return ValueProperty;
    return SingleColourLayer::getPropertyType(name);
}

int AnnotationGridLayer::getPropertyRangeAndValue(const PropertyName &name, int *min, int *max, int *deflt) const
{
    int val = 0;
    if (name == "Structure") {
        if (min) *min = 0;
        if (max) *max = 2;
        if (deflt) *deflt = int(PlotBlendedSpeakers);
        val = int(m_plotStyle);
    } else {
        val = SingleColourLayer::getPropertyRangeAndValue(name, min, max, deflt);
    }
    return val;
}

QString AnnotationGridLayer::getPropertyValueLabel(const PropertyName &name, int value) const
{
    if (name == "Structure") {
        switch (value) {
        default:
        case 0: return tr("Blend speakers together");
        case 1: return tr("Speaker, then Level/Attribute");
        case 2: return tr("Level/Attribute, then Speaker");
        }
    }
    return SingleColourLayer::getPropertyValueLabel(name, value);
}

void AnnotationGridLayer::setProperty(const PropertyName &name, int value)
{
    if (name == "Structure") {
        setPlotStyle(PlotStyle(value));
    } else {
        SingleColourLayer::setProperty(name, value);
    }
}

bool AnnotationGridLayer::getValueExtents(double &, double &, bool &, QString &) const
{
    return false;
}

// ==============================================================================================================================

bool
AnnotationGridLayer::isLayerScrollable(const View *v) const
{
    QPoint discard;
    bool illuminate = v->shouldIlluminateLocalFeatures(this, discard);
    return ((!m_textEditing) && (!illuminate));
}

void AnnotationGridLayer::setPlotStyle(PlotStyle style)
{
    if (m_plotStyle == style) return;
    textEditingFinished();
    m_plotStyle = style;
    if (m_plotStyle == PlotLevelAttributesThenSpeakers)
        m_tierTuples = m_model->tierTuples(AnnotationGridModel::LayoutLevelAttributesThenSpeakers);
    else // including PlotBlendedSpeakers
        m_tierTuples = m_model->tierTuples(AnnotationGridModel::LayoutSpeakersThenLevelAttributes);
    emit layerParametersChanged();
}

int AnnotationGridLayer::getYForTierIndex(View *v, int tierIndex) const
{
    int h = v->height();
    if ((m_plotStyle == PlotBlendedSpeakers) && m_model) {
        int i = m_tierTuples.at(tierIndex).indexLevelAttributePair;
        int c = m_model->countLevelsAttributes();
        // qDebug() << "Tier index " << tierIndex << " i " << i << " c " << c <<  " height " << int(i * h / c);
        return int(i * h / c);
    }
    // qDebug() << "Tier index " << tierIndex << " c " << m_tierTuples.count() <<  " height " << int(tierIndex * h / m_tierTuples.count());
    return int(tierIndex * h / m_tierTuples.count());
}

int AnnotationGridLayer::getTierIndexForY(View *v, int y) const
{
    int h = v->height();
    int tierIndex(0);
    if ((m_plotStyle == PlotBlendedSpeakers) && m_model)
        tierIndex = int(y * m_model->countLevelsAttributes() / h);
    else
        tierIndex = (y * m_tierTuples.count() / h);
    if (tierIndex < 0) tierIndex = 0;
    if (tierIndex >= m_tierTuples.count()) tierIndex = m_tierTuples.count() - 1;
    return tierIndex;
}

AnnotationGridPointModel::PointList AnnotationGridLayer::getLocalPoints(View *v, int x, int y) const
{
    if (!m_model) return AnnotationGridPointModel::PointList();
    // Info for the tier at the y-coordinate
    int tierIndex = getTierIndexForY(v, y);
    if (tierIndex < 0 || tierIndex >= m_tierTuples.count()) return AnnotationGridPointModel::PointList();
    QString levelID = m_tierTuples.at(tierIndex).levelID;
    // Frame from the x-coordinate
    sv_frame_t frame = v->getFrameForX(x);
    // Points for this tier
    QPointer<AnnotationGridPointModel> boundaryModel = m_model->boundariesForLevel(levelID);
    if (!boundaryModel) return AnnotationGridPointModel::PointList();
    // Points of this tier, that cover the given frame number
    AnnotationGridPointModel::PointList onPoints = boundaryModel->getPoints(frame);
    if (!onPoints.empty()) return onPoints;

    AnnotationGridPointModel::PointList prevPoints = boundaryModel->getPreviousPoints(frame);
    return prevPoints;
}

bool AnnotationGridLayer::getPointToDrag(View *v, int x, int y, AnnotationGridPointModel::Point &p) const
{
    if (!m_model) return false;
    // Info for the tier at the y-coordinate
    int tierIndex = getTierIndexForY(v, y);
    if (tierIndex < 0 || tierIndex >= m_tierTuples.count()) return false;
    QString levelID = m_tierTuples.at(tierIndex).levelID;
    // Frames in the vicinit of the x-coordinate
    sv_frame_t a = v->getFrameForX(x - 120);
    sv_frame_t b = v->getFrameForX(x + 10);
    // Points for this tier
    QPointer<AnnotationGridPointModel> boundaryModel = m_model->boundariesForLevel(levelID);
    if (!boundaryModel) return false;
    // Points of this tier between a and b
    AnnotationGridPointModel::PointList onPoints = boundaryModel->getPoints(a, b);
    if (onPoints.empty()) return false;
    // Find the nearest point to the click coordinates
    double nearestDistance = -1;
    for (AnnotationGridPointModel::PointList::const_iterator i = onPoints.begin(); i != onPoints.end(); ++i) {
        double distance = v->getXForFrame((*i).frame) - x;
        if (nearestDistance == -1 || distance < nearestDistance) {
            nearestDistance = distance;
            p = *i;
        }
    }
    return true;
}

QString AnnotationGridLayer::getFeatureDescription(View *v, QPoint &pos) const
{    
    if (!m_model || !m_model->getSampleRate()) return "";

    int x = pos.x();
    int y = pos.y();
    AnnotationGridPointModel::PointList points = getLocalPoints(v, x, y);

    if (points.empty()) {
        if (!m_model->isReady()) {
            return tr("In progress");
        } else {
            return "";
        }
    }

    sv_frame_t frame = points.begin()->frame;
    RealTime rt = RealTime::frame2RealTime(frame, m_model->getSampleRate());
    RealTime rd = m_model->elementDuration(*(points.begin()));

    QString text = QString(tr("Time:\t%1\nDuration:\t%2"))
            .arg(rt.toText(true).c_str()).arg(rd.toText(true).c_str());

    pos = QPoint(v->getXForFrame(frame), y);
    return text;
}


bool AnnotationGridLayer::snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    if (!m_model) return Layer::snapToFeatureFrame(v, frame, resolution, snap, y);

    int tierIndex = getTierIndexForY(v, y);
    if (tierIndex < 0 || tierIndex >= m_tierTuples.count())
        return Layer::snapToFeatureFrame(v, frame, resolution, snap, y);

    QString levelID = m_tierTuples.at(tierIndex).levelID;
    QPointer<AnnotationGridPointModel> boundaryModel = m_model->boundariesForLevel(levelID);
    if (!boundaryModel) return Layer::snapToFeatureFrame(v, frame, resolution, snap, y);

    resolution = boundaryModel->getResolution();

    AnnotationGridPointModel::PointList points;

    if (snap == SnapNeighbouring) {
        points = getLocalPoints(v, v->getXForFrame(frame), y);
        if (points.empty()) return false;
        frame = points.begin()->frame;
        return true;
    }

    points = boundaryModel->getPoints(frame, frame);
    sv_frame_t snapped = frame;
    bool found = false;

    for (AnnotationGridPointModel::PointList::const_iterator i = points.begin(); i != points.end(); ++i) {
        sv_frame_t duration = RealTime::realTime2Frame(m_model->elementDuration(*i), m_model->getSampleRate());
        if (snap == SnapRight) {
            // The best frame to snap to is the end frame of whichever feature we would have snapped to the start frame of if
            // we had been snapping left.
            if (i->frame <= frame) {
                if (i->frame + duration > frame) {
                    snapped = i->frame + duration;
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
            AnnotationGridPointModel::PointList::const_iterator j = i;
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

void AnnotationGridLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    if (!m_model || !m_model->isOK()) return;

    sv_samplerate_t sampleRate = m_model->getSampleRate();
    if (!sampleRate) return;

    int x0 = rect.left(), x1 = rect.right();
    sv_frame_t frame0 = v->getFrameForX(x0);
    sv_frame_t frame1 = v->getFrameForX(x1);

    int tierCount = m_tierTuples.count();

    // Draw horizontal tier lines
    // ----------------------------------------------------------------------------------------------------------------
    QColor penColour;
    penColour = v->getForeground();
    paint.setPen(penColour);
    paint.setRenderHint(QPainter::Antialiasing, false);
    for (int tierIndex = 1; tierIndex < tierCount; ++ tierIndex) {
        int y = getYForTierIndex(v, tierIndex);
        paint.drawLine(rect.left(), y, rect.right(), y);
    }

    QPoint localPos;
    AnnotationGridPointModel::Point pointToIlluminate(0);
    if (v->shouldIlluminateLocalFeatures(this, localPos)) {
        AnnotationGridPointModel::PointList localPoints = getLocalPoints(v, localPos.x(), localPos.y());
        if (!localPoints.empty()) pointToIlluminate = *(localPoints.begin());
    }

    // Draw boundaries and data
    // ----------------------------------------------------------------------------------------------------------------
    for (int tierIndex = 0; tierIndex < tierCount; ++ tierIndex) {
        // Top and bottom for boundary lines
        int y0 = getYForTierIndex(v, tierIndex) + 1;
        int y1; // bottom boundary, calculate as follows
        if (m_plotStyle == PlotBlendedSpeakers) {
            // attention, in the Blended Speakers plot mode, the tiers wrap around
            y1 = ((tierIndex + 1) % m_model->countLevelsAttributes()) ? getYForTierIndex(v, tierIndex + 1) : rect.bottom();
        } else {
            y1 = (tierIndex + 1 < tierCount) ? getYForTierIndex(v, tierIndex + 1) : rect.bottom();
        }

        int boundaryHeight = y1 - y0;

        // Info for this tier
        QString speakerID = m_tierTuples.at(tierIndex).speakerID;
        QString levelID = m_tierTuples.at(tierIndex).levelID;
        QString attributeID = m_tierTuples.at(tierIndex).attributeID;

        // Points for this tier
        QPointer<AnnotationGridPointModel> boundaryModel = m_model->boundariesForLevel(levelID);
        if (!boundaryModel) continue;
        AnnotationGridPointModel::PointList boundaries = boundaryModel->getPoints(frame0, frame1);

        QColor brushColour(getBaseQColor());
        brushColour.setAlpha(100);

        for (AnnotationGridPointModel::PointList::const_iterator i = boundaries.begin(); i != boundaries.end(); ++i) {
            const AnnotationGridPointModel::Point &p(*i);
            if (p.speakerID != speakerID) continue;
            // Find the next point by the same speaker
            AnnotationGridPointModel::PointList::const_iterator j = i;
            while ((j != boundaries.end()) && (j->speakerID == speakerID)) ++j;
            // Calculate bounds
            int x = v->getXForFrame(p.frame);
            int boundaryWidth = v->getXForFrame(p.frame + 1) - x;
            if (boundaryWidth < 2) {
                if (boundaryWidth < 1) {
                    boundaryWidth = 2;
                    if (j != boundaries.end()) {
                        int nx = v->getXForFrame(j->frame);
                        if (nx < x + 3) boundaryWidth = 1;
                    }
                } else {
                    boundaryWidth = 2;
                }
            }
            // Illuminate goes here
            bool shouldIlluminate =  (!AnnotationGridPointModel::Point::Comparator()(pointToIlluminate, p) &&
                                      !AnnotationGridPointModel::Point::Comparator()(p, pointToIlluminate));
//            if (AnnotationGridPointModel::Point::Comparator()(pointToIlluminate, p) || AnnotationGridPointModel::Point::Comparator()(p, pointToIlluminate)) {
                paint.setPen(penColour);
                paint.setBrush(brushColour);
//            } else {
//                paint.setBrush(penColour);
//                paint.setPen(v->getBackground());
//            }
//            if (AnnotationGridPointModel::Point::Comparator(pointToIlluminate, p)) {
//                paint.setPen(getForegroundQColor(v));
//                paint.setBrush(getForegroundQColor(v));
//            } else {
//                paint.setPen(getBaseQColor());
//                paint.setBrush(brushColour);
//            }
            // Draw boundary
            if (boundaryWidth > 1) {
                paint.drawRect(x, y0, boundaryWidth - 1, boundaryHeight);
            } else {
                paint.drawLine(x, y0, x, boundaryHeight);
            }
            // Contents rectangle
            int boxMaxWidth = v->getXForFrame(p.frame + RealTime::realTime2Frame(m_model->elementDuration(p), m_model->getSampleRate())) - x - 6;
            int boxMaxHeight = y1 - y0 - 4;
            QRect textRect = QRect(x + 3, y0 + 2, boxMaxWidth, boxMaxHeight);
//            if (shouldIlluminate) {
//                paint.setBrush(Qt::yellow);
//                paint.drawRect(textRect);
//            }
            // Label
            QString label = m_model->data(speakerID, levelID, p.frame, attributeID).toString();
            if (label.isEmpty()) continue;
            QRect boundingRect = paint.fontMetrics().boundingRect(QRect(0, 0, boxMaxWidth, boxMaxHeight),
                                                                  Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                                                                  label);
            paint.setPen(v->getForeground());
            paint.setRenderHint(QPainter::Antialiasing, true);
            if (boundingRect.width() > textRect.width())
                paint.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, label);
            else
                paint.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap, label);
            paint.setRenderHint(QPainter::Antialiasing, false);
            // qDebug() << y0 << y1 << label;
            // cerr << tierIndex << " " << speakerID << " " << levelID << " " << attributeID << " " << y0 << " " << y1 << " " << label << endl;
        }
    }

    // Text editor
    if (m_textEditing) textEditorReposition(v);
}

// Draw: add boundaries on tiers
// ====================================================================================================================

void AnnotationGridLayer::drawStart(View *v, QMouseEvent *e)
{
    //    cerr << "AnnotationGridLayer::drawStart(" << e->x() << "," << e->y() << ")" << endl;
    if (!m_model) return;
    // Get tier info based on y coordinate
    int tierIndex = getTierIndexForY(v, e->y());
    QString speakerID = m_tierTuples.at(tierIndex).speakerID;
    QString levelID = m_tierTuples.at(tierIndex).levelID;
    // Get frame on which to add boundary
    sv_frame_t frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    // New boundary
    m_boundaryEditingPoint = AnnotationGridPointModel::Point(frame, speakerID, levelID);
    // Finalise previously pending command
    if (m_boundaryEditingCommand) finish(m_boundaryEditingCommand);
    // Start command to add boundary
    m_boundaryEditingCommand = new AnnotationGridModel::EditBoundaryCommand(m_model, tr("Draw Point"));
    m_boundaryEditingCommand->addBoundary(m_boundaryEditingPoint);
    m_boundaryEditing = true;
}

void AnnotationGridLayer::drawDrag(View *v, QMouseEvent *e)
{
    //    cerr << "AnnotationGridLayer::drawDrag(" << e->x() << "," << e->y() << ")" << endl;
    if (!m_model || !m_boundaryEditing) return;
    sv_frame_t frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    // m_boundaryEditingCommand->moveBoundary(m_boundaryEditingPoint, frame);
}

void AnnotationGridLayer::drawEnd(View *v, QMouseEvent *e)
{
    Q_UNUSED(v)
    Q_UNUSED(e)
    //    cerr << "AnnotationGridLayer::drawEnd(" << e->x() << "," << e->y() << ")" << endl;
    if (!m_model || !m_boundaryEditing) return;
    QString newName = tr("Add Boundary at %1 s").arg(RealTime::frame2RealTime(m_boundaryEditingPoint.frame,
                                                                              m_model->getSampleRate()).toText(false).c_str());
    m_boundaryEditingCommand->setName(newName);
    finish(m_boundaryEditingCommand);
    m_boundaryEditingCommand = 0;
    m_boundaryEditing = false;
}

// Erase: remove boundaries from tiers
// ====================================================================================================================

void AnnotationGridLayer::eraseStart(View *v, QMouseEvent *e)
{
    if (!m_model) return;
    AnnotationGridPointModel::PointList points = getLocalPoints(v, e->x(), e->y());
    if (points.empty()) return;
    m_boundaryEditingPoint = *points.begin();
    if (m_boundaryEditingCommand) {
        finish(m_boundaryEditingCommand);
        m_boundaryEditingCommand = 0;
    }
    m_boundaryEditing = true;
}

void AnnotationGridLayer::eraseDrag(View *, QMouseEvent *)
{
}

void AnnotationGridLayer::eraseEnd(View *v, QMouseEvent *e)
{
    if (!m_model || !m_boundaryEditing) return;
    m_boundaryEditing = false;
    AnnotationGridPointModel::PointList points = getLocalPoints(v, e->x(), e->y());
    if (points.empty()) return;
    if (points.begin()->frame != m_boundaryEditingPoint.frame) return;
    m_boundaryEditingCommand = new AnnotationGridModel::EditBoundaryCommand(m_model, tr("Erase Boundary"));
    m_boundaryEditingCommand->deleteBoundary(m_boundaryEditingPoint);
    finish(m_boundaryEditingCommand);
    m_boundaryEditingCommand = 0;
    m_boundaryEditing = false;
}

// Edit: move boundary position on tiers
// ====================================================================================================================

void AnnotationGridLayer::editStart(View *v, QMouseEvent *e)
{
    cerr << "AnnotationGridLayer::editStart(" << e->x() << "," << e->y() << ")" << endl;
    if (!m_model) return;
    AnnotationGridPointModel::PointList points = getLocalPoints(v, e->x(), e->y());
    if (points.empty()) return;
    m_boundaryEditingPoint = *points.begin();
    if (m_boundaryEditingCommand) {
        finish(m_boundaryEditingCommand);
        m_boundaryEditingCommand = 0;
    }
    m_boundaryEditing = true;
}

void AnnotationGridLayer::editDrag(View *v, QMouseEvent *e)
{
    if (!m_model || !m_boundaryEditing) return;
    sv_frame_t frame = v->getFrameForX(e->x());
    // Checks
    if (frame < 0) frame = 0;
    // if (frame <= m_model->elementMoveLimitFrameLeft(m_boundaryEditingPoint)) return;
    // if (frame >= m_model->elementMoveLimitFrameRight(m_boundaryEditingPoint)) return;
    // Update editing command
    if (!m_boundaryEditingCommand) {
        m_boundaryEditingCommand = new AnnotationGridModel::EditBoundaryCommand(m_model, tr("Drag Boundary"));
    }
    m_boundaryEditingCommand->moveBoundary(m_boundaryEditingPoint, frame);
}

void AnnotationGridLayer::editEnd(View *, QMouseEvent *)
{
//  cerr << "AnnotationGridLayer::editEnd(" << e->x() << "," << e->y() << ")" << endl;
    if (!m_model || !m_boundaryEditing) return;
    if (m_boundaryEditingCommand) {
        QString newName = tr("Move Boundary on Level %1 to %2 s")
                .arg(m_boundaryEditingPoint.levelID)
                .arg(RealTime::frame2RealTime(m_boundaryEditingPoint.frame, m_model->getSampleRate()).toText(false).c_str());
        m_boundaryEditingCommand->setName(newName);
        finish(m_boundaryEditingCommand);
    }
    m_boundaryEditingCommand = 0;
    m_boundaryEditing = false;
}

// Edit open: edit the label of an interval
// ====================================================================================================================

bool AnnotationGridLayer::editOpen(View *v, QMouseEvent *e)
{
    if (!m_model) return false;

    int tierIndex = getTierIndexForY(v, e->y());
    sv_frame_t frame = v->getFrameForX(e->x());

    // Info for this tier
    QString speakerID = m_tierTuples.at(tierIndex).speakerID;
    QString levelID = m_tierTuples.at(tierIndex).levelID;
    QString attributeID = m_tierTuples.at(tierIndex).attributeID;

    // Points for this tier
    QPointer<AnnotationGridPointModel> boundaryModel = m_model->boundariesForLevel(levelID);
    if (!boundaryModel) return false;
    AnnotationGridPointModel::PointList prevPoints = boundaryModel->getPreviousPoints(frame);

    if (!prevPoints.empty()) {
        m_textEditorPoint = *(prevPoints.begin());
        m_textEditorTierIndex = tierIndex;

        QString editText = m_model->data(speakerID, levelID, prevPoints.begin()->frame, attributeID).toString();
        m_textEditor->setText(editText);
        textEditorReposition(v);
        m_textEditor->show();
        m_textEditing = true;
        m_textEditorSpeakerID = speakerID;
        m_textEditorLevelID = levelID;
        m_textEditorAttributeID = attributeID;
    }

//    AnnotationGridModel::Point text(0);
//    if (!getPointToDrag(v, e->x(), e->y(), text)) return false;

//    QString label = text.label;

//    bool ok = false;
//    label = QInputDialog::getText(v, tr("Enter label"),
//                                  tr("Please enter a new label:"),
//                                  QLineEdit::Normal, label, &ok);
//    if (ok && label != text.label) {
//        AnnotationGridModel::RelabelCommand *command =
//                new AnnotationGridModel::RelabelCommand(m_model, text, label);
//        CommandHistory::getInstance()->addCommand(command);
//    }

    return true;
}

void AnnotationGridLayer::textEditorReposition(View *v) const
{
    if (!m_model) return;
    int x_left = v->getXForFrame(m_textEditorPoint.frame);
    int y_top = getYForTierIndex(v, m_textEditorTierIndex);
    sv_frame_t duration = RealTime::realTime2Frame(m_model->elementDuration(m_textEditorPoint), m_model->getSampleRate());
    int width = v->getXForFrame(m_textEditorPoint.frame + duration) - x_left;
    if (width < 50) width = 50;
    int height(50); if (m_tierTuples.count() > 0) height = v->height() / m_tierTuples.count();
    m_textEditor->setParent(v);
    m_textEditor->move(x_left + 3, y_top + 2);
    m_textEditor->resize(width - 4, height - 2);
    // qDebug() << "Text editor move to" << x_left << y_top;
}

void AnnotationGridLayer::textEditingFinished()
{
    m_textEditor->hide();
    if (m_textEditing) {
        m_model->setData(m_textEditorSpeakerID, m_textEditorLevelID, m_textEditorPoint.frame,
                         m_textEditorAttributeID, m_textEditor->text());
    }
    m_textEditorSpeakerID.clear(); m_textEditorLevelID.clear(); m_textEditorAttributeID.clear();
    m_textEditing = false;
}

void AnnotationGridLayer::moveSelection(Selection s, sv_frame_t newStartFrame)
{
    if (!m_model) return;

//    AnnotationGridModel::EditCommand *command =
//            new AnnotationGridModel::EditCommand(m_model, tr("Drag Selection"));

//    AnnotationGridModel::PointList points =
//            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

//    for (AnnotationGridModel::PointList::iterator i = points.begin();
//         i != points.end(); ++i) {

//        if (s.contains(i->frame)) {
//            AnnotationGridModel::Point newPoint(*i);
//            newPoint.frame = i->frame + newStartFrame - s.getStartFrame();
//            command->deletePoint(*i);
//            command->addPoint(newPoint);
//        }
//    }

//    finish(command);
}

void AnnotationGridLayer::resizeSelection(Selection s, Selection newSize)
{
    if (!m_model) return;

//    AnnotationGridModel::EditCommand *command =
//            new AnnotationGridModel::EditCommand(m_model, tr("Resize Selection"));

//    AnnotationGridModel::PointList points =
//            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

//    double ratio =
//            double(newSize.getEndFrame() - newSize.getStartFrame()) /
//            double(s.getEndFrame() - s.getStartFrame());

//    for (AnnotationGridModel::PointList::iterator i = points.begin();
//         i != points.end(); ++i) {

//        if (s.contains(i->frame)) {

//            double target = double(i->frame);
//            target = double(newSize.getStartFrame()) +
//                    target - double(s.getStartFrame()) * ratio;

//            AnnotationGridModel::Point newPoint(*i);
//            newPoint.frame = lrint(target);
//            command->deletePoint(*i);
//            command->addPoint(newPoint);
//        }
//    }

//    finish(command);
}

void AnnotationGridLayer::deleteSelection(Selection s)
{
    if (!m_model) return;

//    AnnotationGridModel::EditCommand *command =
//            new AnnotationGridModel::EditCommand(m_model, tr("Delete Selection"));

//    AnnotationGridModel::PointList points =
//            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

//    for (AnnotationGridModel::PointList::iterator i = points.begin();
//         i != points.end(); ++i) {
//        if (s.contains(i->frame)) command->deletePoint(*i);
//    }

//    finish(command);
}

void AnnotationGridLayer::copy(View *v, Selection s, Clipboard &to)
{
    if (!m_model) return;

//    AnnotationGridModel::PointList points =
//            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

//    for (AnnotationGridModel::PointList::iterator i = points.begin();
//         i != points.end(); ++i) {
//        if (s.contains(i->frame)) {
//            Clipboard::Point point(i->frame, i->height, i->label);
//            point.setReferenceFrame(alignToReference(v, i->frame));
//            to.addPoint(point);
//        }
//    }
}

bool AnnotationGridLayer::paste(View *v, const Clipboard &from, sv_frame_t /* frameOffset */, bool /* interactive */)
{
//    if (!m_model) return false;

//    const Clipboard::PointList &points = from.getPoints();

//    bool realign = false;

//    if (clipboardHasDifferentAlignment(v, from)) {

//        QMessageBox::StandardButton button =
//                QMessageBox::question(v, tr("Re-align pasted items?"),
//                                      tr("The items you are pasting came from a layer with different source material from this one.  Do you want to re-align them in time, to match the source material for this layer?"),
//                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
//                                      QMessageBox::Yes);

//        if (button == QMessageBox::Cancel) {
//            return false;
//        }

//        if (button == QMessageBox::Yes) {
//            realign = true;
//        }
//    }

//    AnnotationGridModel::EditCommand *command =
//            new AnnotationGridModel::EditCommand(m_model, tr("Paste"));

//    double valueMin = 0.0, valueMax = 1.0;
//    for (Clipboard::PointList::const_iterator i = points.begin();
//         i != points.end(); ++i) {
//        if (i->haveValue()) {
//            if (i->getValue() < valueMin) valueMin = i->getValue();
//            if (i->getValue() > valueMax) valueMax = i->getValue();
//        }
//    }
//    if (valueMax < valueMin + 1.0) valueMax = valueMin + 1.0;

//    for (Clipboard::PointList::const_iterator i = points.begin();
//         i != points.end(); ++i) {

//        if (!i->haveFrame()) continue;
//        sv_frame_t frame = 0;

//        if (!realign) {

//            frame = i->getFrame();

//        } else {

//            if (i->haveReferenceFrame()) {
//                frame = i->getReferenceFrame();
//                frame = alignFromReference(v, frame);
//            } else {
//                frame = i->getFrame();
//            }
//        }

//        AnnotationGridModel::Point newPoint(frame);

//        if (i->haveValue()) {
//            newPoint.height = float((i->getValue() - valueMin) / (valueMax - valueMin));
//        } else {
//            newPoint.height = 0.5f;
//        }

//        if (i->haveLabel()) {
//            newPoint.label = i->getLabel();
//        } else if (i->haveValue()) {
//            newPoint.label = QString("%1").arg(i->getValue());
//        } else {
//            newPoint.label = tr("New Point");
//        }

//        command->addPoint(newPoint);
//    }

//    finish(command);
//    return true;
    return false;
}

int AnnotationGridLayer::getDefaultColourHint(bool darkbg, bool &impose)
{
    impose = false;
    return ColourDatabase::getInstance()->getColourIndex
            (QString(darkbg ? "Bright Orange" : "Orange"));
}

void AnnotationGridLayer::toXml(QTextStream &stream,
                           QString indent, QString extraAttributes) const
{
    SingleColourLayer::toXml(stream, indent, extraAttributes);
}

void AnnotationGridLayer::setProperties(const QXmlAttributes &attributes)
{
    SingleColourLayer::setProperties(attributes);
}

