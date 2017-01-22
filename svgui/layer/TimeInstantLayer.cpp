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

#include "TimeInstantLayer.h"

#include "data/model/Model.h"
#include "base/RealTime.h"
#include "view/View.h"
#include "base/Profiler.h"
#include "base/Clipboard.h"
#include "ColourDatabase.h"
#include "LayerFactory.h"

#include "data/model/SparseOneDimensionalModel.h"

#include "widgets/ItemEditDialog.h"
#include "widgets/ListInputDialog.h"

#include <QPainter>
#include <QMouseEvent>
#include <QTextStream>
#include <QMessageBox>

#include <iostream>
#include <cmath>

//#define DEBUG_TIME_INSTANT_LAYER 1

TimeInstantLayer::TimeInstantLayer() :
    SingleColourLayer(),
    m_model(0),
    m_editing(false),
    m_editingPoint(0, tr("New Point")),
    m_editingCommand(0),
    m_plotStyle(PlotInstants)
{
}

TimeInstantLayer::~TimeInstantLayer()
{
}

bool TimeInstantLayer::trySetModel(Model *model)
{
    if (trySetModelHelper<TimeInstantLayer, SparseOneDimensionalModel>(this, model))
        return true;
    return false;
}

void
TimeInstantLayer::setModel(SparseOneDimensionalModel *model)
{
    if (m_model == model) return;
    m_model = model;

    connectSignals(m_model);

#ifdef DEBUG_TIME_INSTANT_LAYER
    cerr << "TimeInstantLayer::setModel(" << model << ")" << endl;
#endif

    if (m_model && m_model->getRDFTypeURI().endsWith("Segment")) {
        setPlotStyle(PlotSegmentation);
    }

    emit modelReplaced();
}

// static
Model *TimeInstantLayer::createEmptyModel(Model *baseModel)
{
    return new SparseOneDimensionalModel(baseModel->getSampleRate(), 1);
}

Layer::PropertyList
TimeInstantLayer::getProperties() const
{
    PropertyList list = SingleColourLayer::getProperties();
    list.push_back("Plot Type");
    return list;
}

QString
TimeInstantLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Plot Type") return tr("Plot Type");
    return SingleColourLayer::getPropertyLabel(name);
}

Layer::PropertyType
TimeInstantLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Plot Type") return ValueProperty;
    return SingleColourLayer::getPropertyType(name);
}

int
TimeInstantLayer::getPropertyRangeAndValue(const PropertyName &name,
                                           int *min, int *max, int *deflt) const
{
    int val = 0;

    if (name == "Plot Type") {

        if (min) *min = 0;
        if (max) *max = 1;
        if (deflt) *deflt = 0;

        val = int(m_plotStyle);

    } else {

        val = SingleColourLayer::getPropertyRangeAndValue(name, min, max, deflt);
    }

    return val;
}

QString
TimeInstantLayer::getPropertyValueLabel(const PropertyName &name,
                                        int value) const
{
    if (name == "Plot Type") {
        switch (value) {
        default:
        case 0: return tr("Instants");
        case 1: return tr("Segmentation");
        }
    }
    return SingleColourLayer::getPropertyValueLabel(name, value);
}

void
TimeInstantLayer::setProperty(const PropertyName &name, int value)
{
    if (name == "Plot Type") {
        setPlotStyle(PlotStyle(value));
    } else {
        SingleColourLayer::setProperty(name, value);
    }
}

void
TimeInstantLayer::setPlotStyle(PlotStyle style)
{
    if (m_plotStyle == style) return;
    m_plotStyle = style;
    emit layerParametersChanged();
}

bool
TimeInstantLayer::isLayerScrollable(const View *v) const
{
    QPoint discard;
    return !v->shouldIlluminateLocalFeatures(this, discard);
}

SparseOneDimensionalModel::PointList
TimeInstantLayer::getLocalPoints(View *v, int x) const
{
    // Return a set of points that all have the same frame number, the
    // nearest to the given x coordinate, and that are within a
    // certain fuzz distance of that x coordinate.

    if (!m_model) return SparseOneDimensionalModel::PointList();

    long frame = v->getFrameForX(x);

    SparseOneDimensionalModel::PointList onPoints =
            m_model->getPoints(frame);

    if (!onPoints.empty()) {
        return onPoints;
    }

    SparseOneDimensionalModel::PointList prevPoints =
            m_model->getPreviousPoints(frame);
    SparseOneDimensionalModel::PointList nextPoints =
            m_model->getNextPoints(frame);

    SparseOneDimensionalModel::PointList usePoints = prevPoints;

    if (prevPoints.empty()) {
        usePoints = nextPoints;
    } else if (long(prevPoints.begin()->frame) < v->getStartFrame() &&
               !(nextPoints.begin()->frame > v->getEndFrame())) {
        usePoints = nextPoints;
    } else if (nextPoints.begin()->frame - frame <
               frame - prevPoints.begin()->frame) {
        usePoints = nextPoints;
    }

    if (!usePoints.empty()) {
        int fuzz = 2;
        int px = v->getXForFrame(usePoints.begin()->frame);
        if ((px > x && px - x > fuzz) ||
                (px < x && x - px > fuzz + 1)) {
            usePoints.clear();
        }
    }

    return usePoints;
}

QString
TimeInstantLayer::getLabelPreceding(sv_frame_t frame) const
{
    if (!m_model) return "";
    SparseOneDimensionalModel::PointList points = m_model->getPreviousPoints(frame);
    for (SparseOneDimensionalModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {
        if (i->label != "") return i->label;
    }
    return "";
}

QString
TimeInstantLayer::getFeatureDescription(View *v, QPoint &pos) const
{
    int x = pos.x();

    if (!m_model || !m_model->getSampleRate()) return "";

    SparseOneDimensionalModel::PointList points = getLocalPoints(v, x);

    if (points.empty()) {
        if (!m_model->isReady()) {
            return tr("In progress");
        } else {
            return tr("No local points");
        }
    }

    long useFrame = points.begin()->frame;

    RealTime rt = RealTime::frame2RealTime(useFrame, m_model->getSampleRate());
    
    QString text;

    if (points.begin()->label == "") {
        text = QString(tr("Time:\t%1\nNo label"))
                .arg(rt.toText(true).c_str());
    } else {
        text = QString(tr("Time:\t%1\nLabel:\t%2"))
                .arg(rt.toText(true).c_str())
                .arg(points.begin()->label);
    }

    pos = QPoint(v->getXForFrame(useFrame), pos.y());
    return text;
}

bool
TimeInstantLayer::snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    if (!m_model) {
        return Layer::snapToFeatureFrame(v, frame, resolution, snap, y);
    }

    resolution = m_model->getResolution();
    SparseOneDimensionalModel::PointList points;

    if (snap == SnapNeighbouring) {
        points = getLocalPoints(v, v->getXForFrame(frame));
        if (points.empty()) return false;
        frame = points.begin()->frame;
        return true;
    }

    points = m_model->getPoints(frame, frame);
    sv_frame_t snapped = frame;
    bool found = false;

    for (SparseOneDimensionalModel::PointList::const_iterator i = points.begin(); i != points.end(); ++i) {
        if (snap == SnapRight) {
            if (i->frame >= frame) {
                snapped = i->frame;
                found = true;
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
            SparseOneDimensionalModel::PointList::const_iterator j = i;
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

void
TimeInstantLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    if (!m_model || !m_model->isOK()) return;

    //    Profiler profiler("TimeInstantLayer::paint", true);

    int x0 = rect.left(), x1 = rect.right();

    long frame0 = v->getFrameForX(x0);
    long frame1 = v->getFrameForX(x1);

    SparseOneDimensionalModel::PointList points(m_model->getPoints(frame0, frame1));

    bool odd = false;
    if (m_plotStyle == PlotSegmentation && !points.empty()) {
        int index = m_model->getIndexOf(*points.begin());
        odd = ((index % 2) == 1);
    }

    paint.setPen(getBaseQColor());

    QColor brushColour(getBaseQColor());
    brushColour.setAlpha(100);
    paint.setBrush(brushColour);

    QColor oddBrushColour(brushColour);
    if (m_plotStyle == PlotSegmentation) {
        if (getBaseQColor() == Qt::black) {
            oddBrushColour = Qt::gray;
        } else if (getBaseQColor() == Qt::darkRed) {
            oddBrushColour = Qt::red;
        } else if (getBaseQColor() == Qt::darkBlue) {
            oddBrushColour = Qt::blue;
        } else if (getBaseQColor() == Qt::darkGreen) {
            oddBrushColour = Qt::green;
        } else {
            oddBrushColour = oddBrushColour.light(150);
        }
        oddBrushColour.setAlpha(100);
    }

    //    cerr << "TimeInstantLayer::paint: resolution is "
    //	      << m_model->getResolution() << " frames" << endl;

    QPoint localPos;
    long illuminateFrame = -1;

    if (v->shouldIlluminateLocalFeatures(this, localPos)) {
        SparseOneDimensionalModel::PointList localPoints =
                getLocalPoints(v, localPos.x());
        if (!localPoints.empty()) illuminateFrame = localPoints.begin()->frame;
    }

    int prevX = -1;
    int textY = v->getTextLabelHeight(this, paint);
    
    for (SparseOneDimensionalModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {

        const SparseOneDimensionalModel::Point &p(*i);
        SparseOneDimensionalModel::PointList::const_iterator j = i;
        ++j;

        int x = v->getXForFrame(p.frame);
        if (x == prevX && m_plotStyle == PlotInstants &&
                p.frame != illuminateFrame) continue;

        int iw = v->getXForFrame(p.frame + m_model->getResolution()) - x;
        if (iw < 2) {
            if (iw < 1) {
                iw = 2;
                if (j != points.end()) {
                    int nx = v->getXForFrame(j->frame);
                    if (nx < x + 3) iw = 1;
                }
            } else {
                iw = 2;
            }
        }

        if (p.frame == illuminateFrame) {
            paint.setPen(getForegroundQColor(v));
        } else {
            paint.setPen(brushColour);
        }

        if (m_plotStyle == PlotInstants) {
            if (iw > 1) {
                paint.drawRect(x, 0, iw - 1, v->height() - 1);
            } else {
                paint.drawLine(x, 0, x, v->height() - 1);
            }
        } else {

            if (odd) paint.setBrush(oddBrushColour);
            else paint.setBrush(brushColour);

            int nx;

            if (j != points.end()) {
                const SparseOneDimensionalModel::Point &q(*j);
                nx = v->getXForFrame(q.frame);
            } else {
                nx = v->getXForFrame(m_model->getEndFrame());
            }

            if (nx >= x) {

                if (illuminateFrame != p.frame &&
                        (nx < x + 5 || x >= v->width() - 1)) {
                    paint.setPen(Qt::NoPen);
                }

                paint.drawRect(x, -1, nx - x, v->height() + 1);
            }

            odd = !odd;
        }

        paint.setPen(getBaseQColor());

        if (p.label != "") {

            // only draw if there's enough room from here to the next point

            int lw = paint.fontMetrics().width(p.label);
            bool good = true;

            if (j != points.end()) {
                int nx = v->getXForFrame(j->frame);
                if (nx >= x && nx - x - iw - 3 <= lw) good = false;
            }

            if (good) {
                v->drawVisibleText(paint, x + iw + 2, textY, p.label, View::OutlinedText);
                //		paint.drawText(x + iw + 2, textY, p.label);
            }
        }

        prevX = x;
    }
}

void
TimeInstantLayer::drawStart(View *v, QMouseEvent *e)
{
#ifdef DEBUG_TIME_INSTANT_LAYER
    cerr << "TimeInstantLayer::drawStart(" << e->x() << ")" << endl;
#endif

    if (!m_model) return;

    long frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    m_editingPoint = SparseOneDimensionalModel::Point(frame, tr("New Point"));

    if (m_editingCommand) finish(m_editingCommand);
    m_editingCommand = new SparseOneDimensionalModel::EditCommand(m_model,
                                                                  tr("Draw Point"));
    m_editingCommand->addPoint(m_editingPoint);

    m_editing = true;
}

void
TimeInstantLayer::drawDrag(View *v, QMouseEvent *e)
{
#ifdef DEBUG_TIME_INSTANT_LAYER
    cerr << "TimeInstantLayer::drawDrag(" << e->x() << ")" << endl;
#endif

    if (!m_model || !m_editing) return;

    long frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();
    m_editingCommand->deletePoint(m_editingPoint);
    m_editingPoint.frame = frame;
    m_editingCommand->addPoint(m_editingPoint);
}

void
TimeInstantLayer::drawEnd(View *, QMouseEvent *)
{
#ifdef DEBUG_TIME_INSTANT_LAYER
    cerr << "TimeInstantLayer::drawEnd(" << e->x() << ")" << endl;
#endif
    if (!m_model || !m_editing) return;
    QString newName = tr("Add Point at %1 s")
            .arg(RealTime::frame2RealTime(m_editingPoint.frame,
                                          m_model->getSampleRate())
                 .toText(false).c_str());
    m_editingCommand->setName(newName);
    finish(m_editingCommand);
    m_editingCommand = 0;
    m_editing = false;
}

void
TimeInstantLayer::eraseStart(View *v, QMouseEvent *e)
{
    if (!m_model) return;

    SparseOneDimensionalModel::PointList points = getLocalPoints(v, e->x());
    if (points.empty()) return;

    m_editingPoint = *points.begin();

    if (m_editingCommand) {
        finish(m_editingCommand);
        m_editingCommand = 0;
    }

    m_editing = true;
}

void
TimeInstantLayer::eraseDrag(View *, QMouseEvent *)
{
}

void
TimeInstantLayer::eraseEnd(View *v, QMouseEvent *e)
{
    if (!m_model || !m_editing) return;

    m_editing = false;

    SparseOneDimensionalModel::PointList points = getLocalPoints(v, e->x());
    if (points.empty()) return;
    if (points.begin()->frame != m_editingPoint.frame) return;

    m_editingCommand = new SparseOneDimensionalModel::EditCommand
            (m_model, tr("Erase Point"));

    m_editingCommand->deletePoint(m_editingPoint);

    finish(m_editingCommand);
    m_editingCommand = 0;
    m_editing = false;
}

void
TimeInstantLayer::editStart(View *v, QMouseEvent *e)
{
#ifdef DEBUG_TIME_INSTANT_LAYER
    cerr << "TimeInstantLayer::editStart(" << e->x() << ")" << endl;
#endif

    if (!m_model) return;

    SparseOneDimensionalModel::PointList points = getLocalPoints(v, e->x());
    if (points.empty()) return;

    m_editingPoint = *points.begin();

    if (m_editingCommand) {
        finish(m_editingCommand);
        m_editingCommand = 0;
    }

    m_editing = true;
}

void
TimeInstantLayer::editDrag(View *v, QMouseEvent *e)
{
#ifdef DEBUG_TIME_INSTANT_LAYER
    cerr << "TimeInstantLayer::editDrag(" << e->x() << ")" << endl;
#endif

    if (!m_model || !m_editing) return;

    long frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    if (!m_editingCommand) {
        m_editingCommand = new SparseOneDimensionalModel::EditCommand(m_model,
                                                                      tr("Drag Point"));
    }

    m_editingCommand->deletePoint(m_editingPoint);
    m_editingPoint.frame = frame;
    m_editingCommand->addPoint(m_editingPoint);
}

void
TimeInstantLayer::editEnd(View *, QMouseEvent *)
{
#ifdef DEBUG_TIME_INSTANT_LAYER
    cerr << "TimeInstantLayer::editEnd(" << e->x() << ")" << endl;
#endif
    if (!m_model || !m_editing) return;
    if (m_editingCommand) {
        QString newName = tr("Move Point to %1 s")
                .arg(RealTime::frame2RealTime(m_editingPoint.frame,
                                              m_model->getSampleRate())
                     .toText(false).c_str());
        m_editingCommand->setName(newName);
        finish(m_editingCommand);
    }
    m_editingCommand = 0;
    m_editing = false;
}

bool
TimeInstantLayer::editOpen(View *v, QMouseEvent *e)
{
    if (!m_model) return false;

    SparseOneDimensionalModel::PointList points = getLocalPoints(v, e->x());
    if (points.empty()) return false;

    SparseOneDimensionalModel::Point point = *points.begin();

    ItemEditDialog *dialog = new ItemEditDialog
            (m_model->getSampleRate(),
             ItemEditDialog::ShowTime |
             ItemEditDialog::ShowText);

    dialog->setFrameTime(point.frame);
    dialog->setText(point.label);

    if (dialog->exec() == QDialog::Accepted) {

        SparseOneDimensionalModel::Point newPoint = point;
        newPoint.frame = dialog->getFrameTime();
        newPoint.label = dialog->getText();
        
        SparseOneDimensionalModel::EditCommand *command =
                new SparseOneDimensionalModel::EditCommand(m_model, tr("Edit Point"));
        command->deletePoint(point);
        command->addPoint(newPoint);
        finish(command);
    }

    delete dialog;
    return true;
}

void
TimeInstantLayer::moveSelection(Selection s, sv_frame_t newStartFrame)
{
    if (!m_model) return;

    SparseOneDimensionalModel::EditCommand *command =
            new SparseOneDimensionalModel::EditCommand(m_model,
                                                       tr("Drag Selection"));

    SparseOneDimensionalModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (SparseOneDimensionalModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {
            SparseOneDimensionalModel::Point newPoint(*i);
            newPoint.frame = i->frame + newStartFrame - s.getStartFrame();
            command->deletePoint(*i);
            command->addPoint(newPoint);
        }
    }

    finish(command);
}

void
TimeInstantLayer::resizeSelection(Selection s, Selection newSize)
{
    if (!m_model) return;

    SparseOneDimensionalModel::EditCommand *command =
            new SparseOneDimensionalModel::EditCommand(m_model,
                                                       tr("Resize Selection"));

    SparseOneDimensionalModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    double ratio =
            double(newSize.getEndFrame() - newSize.getStartFrame()) /
            double(s.getEndFrame() - s.getStartFrame());

    for (SparseOneDimensionalModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {

            double target = double(i->frame);
            target = double(newSize.getStartFrame()) +
                    target - double(s.getStartFrame()) * ratio;

            SparseOneDimensionalModel::Point newPoint(*i);
            newPoint.frame = lrint(target);
            command->deletePoint(*i);
            command->addPoint(newPoint);
        }
    }

    finish(command);
}

void
TimeInstantLayer::deleteSelection(Selection s)
{
    if (!m_model) return;

    SparseOneDimensionalModel::EditCommand *command =
            new SparseOneDimensionalModel::EditCommand(m_model,
                                                       tr("Delete Selection"));

    SparseOneDimensionalModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (SparseOneDimensionalModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {
        if (s.contains(i->frame)) command->deletePoint(*i);
    }

    finish(command);
}

void
TimeInstantLayer::copy(View *v, Selection s, Clipboard &to)
{
    if (!m_model) return;

    SparseOneDimensionalModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (SparseOneDimensionalModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {
        if (s.contains(i->frame)) {
            Clipboard::Point point(i->frame, i->label);
            point.setReferenceFrame(alignToReference(v, i->frame));
            to.addPoint(point);
        }
    }
}

bool
TimeInstantLayer::paste(View *v, const Clipboard &from, sv_frame_t frameOffset, bool)
{
    if (!m_model) return false;

    const Clipboard::PointList &points = from.getPoints();

    bool realign = false;

    if (clipboardHasDifferentAlignment(v, from)) {

        QMessageBox::StandardButton button =
                QMessageBox::question(v, tr("Re-align pasted instants?"),
                                      tr("The instants you are pasting came from a layer with different source material from this one.  Do you want to re-align them in time, to match the source material for this layer?"),
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                      QMessageBox::Yes);

        if (button == QMessageBox::Cancel) {
            return false;
        }

        if (button == QMessageBox::Yes) {
            realign = true;
        }
    }

    SparseOneDimensionalModel::EditCommand *command =
            new SparseOneDimensionalModel::EditCommand(m_model, tr("Paste"));

    for (Clipboard::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {
        
        if (!i->haveFrame()) continue;

        sv_frame_t frame = 0;

        if (!realign) {
            
            frame = i->getFrame();

        } else {

            if (i->haveReferenceFrame()) {
                frame = i->getReferenceFrame();
                frame = alignFromReference(v, frame);
            } else {
                frame = i->getFrame();
            }
        }

        if (frameOffset > 0) frame += frameOffset;
        else if (frameOffset < 0) {
            if (frame > -frameOffset) frame += frameOffset;
            else frame = 0;
        }

        SparseOneDimensionalModel::Point newPoint(frame);
        if (i->haveLabel()) {
            newPoint.label = i->getLabel();
        } else if (i->haveValue()) {
            newPoint.label = QString("%1").arg(i->getValue());
        }
        
        command->addPoint(newPoint);
    }

    finish(command);
    return true;
}

int
TimeInstantLayer::getDefaultColourHint(bool darkbg, bool &impose)
{
    impose = false;
    return ColourDatabase::getInstance()->getColourIndex
            (QString(darkbg ? "Bright Purple" : "Purple"));
}

void
TimeInstantLayer::toXml(QTextStream &stream,
                        QString indent, QString extraAttributes) const
{
    SingleColourLayer::toXml(stream, indent,
                             extraAttributes +
                             QString(" plotStyle=\"%1\"")
                             .arg(m_plotStyle));
}

void
TimeInstantLayer::setProperties(const QXmlAttributes &attributes)
{
    SingleColourLayer::setProperties(attributes);

    bool ok;
    PlotStyle style = (PlotStyle)
            attributes.value("plotStyle").toInt(&ok);
    if (ok) setPlotStyle(style);
}

