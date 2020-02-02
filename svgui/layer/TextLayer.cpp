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

#include "TextLayer.h"

#include "data/model/Model.h"
#include "PraalineCore/Base/RealTime.h"
#include "base/Profiler.h"
#include "ColourDatabase.h"
#include "view/View.h"
#include "LayerFactory.h"

#include "data/model/TextModel.h"

#include <QPainter>
#include <QMouseEvent>
#include <QInputDialog>
#include <QTextStream>
#include <QMessageBox>

#include <iostream>
#include <cmath>

TextLayer::TextLayer() :
    SingleColourLayer(),
    m_model(0),
    m_editing(false),
    m_originalPoint(0, 0.0, tr("Empty Label")),
    m_editingPoint(0, 0.0, tr("Empty Label")),
    m_editingCommand(0)
{
    
}

bool TextLayer::trySetModel(Model *model)
{
    if (trySetModelHelper<TextLayer, TextModel>(this, model))
        return true;
    return false;
}

void
TextLayer::setModel(TextModel *model)
{
    if (m_model == model) return;
    m_model = model;

    connectSignals(m_model);

    //    cerr << "TextLayer::setModel(" << model << ")" << endl;

    emit modelReplaced();
}

// static
Model *TextLayer::createEmptyModel(Model *baseModel)
{
    return new TextModel(baseModel->getSampleRate(), 1, true);
}

Layer::PropertyList
TextLayer::getProperties() const
{
    PropertyList list = SingleColourLayer::getProperties();
    return list;
}

QString
TextLayer::getPropertyLabel(const PropertyName &name) const
{
    return SingleColourLayer::getPropertyLabel(name);
}

Layer::PropertyType
TextLayer::getPropertyType(const PropertyName &name) const
{
    return SingleColourLayer::getPropertyType(name);
}

int
TextLayer::getPropertyRangeAndValue(const PropertyName &name,
                                    int *min, int *max, int *deflt) const
{
    return SingleColourLayer::getPropertyRangeAndValue(name, min, max, deflt);
}

QString
TextLayer::getPropertyValueLabel(const PropertyName &name,
                                 int value) const
{
    return SingleColourLayer::getPropertyValueLabel(name, value);
}

void
TextLayer::setProperty(const PropertyName &name, int value)
{
    SingleColourLayer::setProperty(name, value);
}

bool
TextLayer::getValueExtents(double &, double &, bool &, QString &) const
{
    return false;
}

bool
TextLayer::isLayerScrollable(const View *v) const
{
    QPoint discard;
    return !v->shouldIlluminateLocalFeatures(this, discard);
}


TextModel::PointList
TextLayer::getLocalPoints(View *v, int x, int y) const
{
    if (!m_model) return TextModel::PointList();

    sv_frame_t frame0 = v->getFrameForX(-150);
    sv_frame_t frame1 = v->getFrameForX(v->width() + 150);
    
    TextModel::PointList points(m_model->getPoints(frame0, frame1));

    TextModel::PointList rv;
    QFontMetrics metrics = QFontMetrics(QFont());

    for (TextModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        const TextModel::Point &p(*i);

        int px = v->getXForFrame(p.frame);
        int py = getYForHeight(v, p.height);

        QString label = p.label;
        if (label == "") {
            label = tr("<no text>");
        }

        QRect rect = metrics.boundingRect
                (QRect(0, 0, 150, 200),
                 Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, label);

        if (py + rect.height() > v->height()) {
            if (rect.height() > v->height()) py = 0;
            else py = v->height() - rect.height() - 1;
        }

        if (x >= px && x < px + rect.width() &&
                y >= py && y < py + rect.height()) {
            rv.insert(p);
        }
    }

    return rv;
}

bool
TextLayer::getPointToDrag(View *v, int x, int y, TextModel::Point &p) const
{
    if (!m_model) return false;

    sv_frame_t a = v->getFrameForX(x - 120);
    sv_frame_t b = v->getFrameForX(x + 10);
    TextModel::PointList onPoints = m_model->getPoints(a, b);
    if (onPoints.empty()) return false;

    double nearestDistance = -1;

    for (TextModel::PointList::const_iterator i = onPoints.begin();
         i != onPoints.end(); ++i) {

        double yd = getYForHeight(v, (*i).height) - y;
        double xd = v->getXForFrame((*i).frame) - x;
        double distance = sqrt(yd*yd + xd*xd);

        if (nearestDistance == -1 || distance < nearestDistance) {
            nearestDistance = distance;
            p = *i;
        }
    }

    return true;
}

QString
TextLayer::getFeatureDescription(View *v, QPoint &pos) const
{
    int x = pos.x();

    if (!m_model || !m_model->getSampleRate()) return "";

    TextModel::PointList points = getLocalPoints(v, x, pos.y());

    if (points.empty()) {
        if (!m_model->isReady()) {
            return tr("In progress");
        } else {
            return "";
        }
    }

    sv_frame_t useFrame = points.begin()->frame;

    RealTime rt = RealTime::frame2RealTime(useFrame, m_model->getSampleRate());
    
    QString text;

    if (points.begin()->label == "") {
        text = QString(tr("Time:\t%1\nHeight:\t%2\nLabel:\t%3"))
                .arg(rt.toText(true).c_str())
                .arg(points.begin()->height)
                .arg(points.begin()->label);
    }

    pos = QPoint(v->getXForFrame(useFrame),
                 getYForHeight(v, points.begin()->height));
    return text;
}


//!!! too much overlap with TimeValueLayer/TimeInstantLayer

bool
TextLayer::snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    if (!m_model) {
        return Layer::snapToFeatureFrame(v, frame, resolution, snap, y);
    }

    resolution = m_model->getResolution();
    TextModel::PointList points;

    if (snap == SnapNeighbouring) {
        points = getLocalPoints(v, v->getXForFrame(frame), -1);
        if (points.empty()) return false;
        frame = points.begin()->frame;
        return true;
    }

    points = m_model->getPoints(frame, frame);
    sv_frame_t snapped = frame;
    bool found = false;

    for (TextModel::PointList::const_iterator i = points.begin(); i != points.end(); ++i) {
        if (snap == SnapRight) {
            if (i->frame > frame) {
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
            TextModel::PointList::const_iterator j = i;
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

int
TextLayer::getYForHeight(View *v, double height) const
{
    int h = v->height();
    return h - int(height * h);
}

double
TextLayer::getHeightForY(View *v, int y) const
{
    int h = v->height();
    return double(h - y) / h;
}

void
TextLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    if (!m_model || !m_model->isOK()) return;

    sv_samplerate_t sampleRate = m_model->getSampleRate();
    if (!sampleRate) return;

    //    Profiler profiler("TextLayer::paint", true);

    int x0 = rect.left(), x1 = rect.right();
    sv_frame_t frame0 = v->getFrameForX(x0);
    sv_frame_t frame1 = v->getFrameForX(x1);

    TextModel::PointList points(m_model->getPoints(frame0, frame1));
    if (points.empty()) return;

    QColor brushColour(getBaseQColor());

    int h, s, val;
    brushColour.getHsv(&h, &s, &val);
    brushColour.setHsv(h, s, 255, 100);

    QColor penColour;
    penColour = v->getForeground();

    //    cerr << "TextLayer::paint: resolution is "
    //	      << m_model->getResolution() << " frames" << endl;

    QPoint localPos;
    TextModel::Point illuminatePoint(0);
    bool shouldIlluminate = false;

    if (v->shouldIlluminateLocalFeatures(this, localPos)) {
        shouldIlluminate = getPointToDrag(v, localPos.x(), localPos.y(),
                                          illuminatePoint);
    }

    int boxMaxWidth = 150;
    int boxMaxHeight = 200;

    paint.save();
    paint.setClipRect(rect.x(), 0, rect.width() + boxMaxWidth, v->height());
    
    for (TextModel::PointList::const_iterator i = points.begin(); i != points.end(); ++i) {

        const TextModel::Point &p(*i);

        int x = v->getXForFrame(p.frame);
        int y = getYForHeight(v, p.height);

        if (!shouldIlluminate ||
                // "illuminatePoint != p"
                TextModel::Point::Comparator()(illuminatePoint, p) ||
                TextModel::Point::Comparator()(p, illuminatePoint)) {
            paint.setPen(penColour);
            paint.setBrush(brushColour);
        } else {
            paint.setBrush(penColour);
            paint.setPen(v->getBackground());
        }

        QString label = p.label;
        if (label == "") {
            label = tr("<no text>");
        }

        QRect boxRect = paint.fontMetrics().boundingRect
                (QRect(0, 0, boxMaxWidth, boxMaxHeight),
                 Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, label);

        QRect textRect = QRect(3, 2, boxRect.width(), boxRect.height());
        boxRect = QRect(0, 0, boxRect.width() + 6, boxRect.height() + 2);

        if (y + boxRect.height() > v->height()) {
            if (boxRect.height() > v->height()) y = 0;
            else y = v->height() - boxRect.height() - 1;
        }

        boxRect = QRect(x, y, boxRect.width(), boxRect.height());
        textRect = QRect(x + 3, y + 2, textRect.width(), textRect.height());

        //	boxRect = QRect(x, y, boxRect.width(), boxRect.height());
        //	textRect = QRect(x + 3, y + 2, textRect.width(), textRect.height());

        paint.setRenderHint(QPainter::Antialiasing, false);
        paint.drawRect(boxRect);

        paint.setRenderHint(QPainter::Antialiasing, true);
        paint.drawText(textRect,
                       Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                       label);

        ///	if (p.label != "") {
        ///	    paint.drawText(x + 5, y - paint.fontMetrics().height() + paint.fontMetrics().ascent(), p.label);
        ///	}
    }

    paint.restore();

    // looks like save/restore doesn't deal with this:
    paint.setRenderHint(QPainter::Antialiasing, false);
}

void
TextLayer::drawStart(View *v, QMouseEvent *e)
{
    //    cerr << "TextLayer::drawStart(" << e->x() << "," << e->y() << ")" << endl;

    if (!m_model) {
        cerr << "TextLayer::drawStart: no model" << endl;
        return;
    }

    sv_frame_t frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    double height = getHeightForY(v, e->y());

    m_editingPoint = TextModel::Point(frame, float(height), "");
    m_originalPoint = m_editingPoint;

    if (m_editingCommand) finish(m_editingCommand);
    m_editingCommand = new TextModel::EditCommand(m_model, "Add Label");
    m_editingCommand->addPoint(m_editingPoint);

    m_editing = true;
}

void
TextLayer::drawDrag(View *v, QMouseEvent *e)
{
    //    cerr << "TextLayer::drawDrag(" << e->x() << "," << e->y() << ")" << endl;

    if (!m_model || !m_editing) return;

    sv_frame_t frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    double height = getHeightForY(v, e->y());

    m_editingCommand->deletePoint(m_editingPoint);
    m_editingPoint.frame = frame;
    m_editingPoint.height = float(height);
    m_editingCommand->addPoint(m_editingPoint);
}

void
TextLayer::drawEnd(View *v, QMouseEvent *)
{
    //    cerr << "TextLayer::drawEnd(" << e->x() << "," << e->y() << ")" << endl;
    if (!m_model || !m_editing) return;

    bool ok = false;
    QString label = QInputDialog::getText(v, tr("Enter label"),
                                          tr("Please enter a new label:"),
                                          QLineEdit::Normal, "", &ok);

    if (ok) {
        TextModel::RelabelCommand *command =
                new TextModel::RelabelCommand(m_model, m_editingPoint, label);
        m_editingCommand->addCommand(command);
    } else {
        m_editingCommand->deletePoint(m_editingPoint);
    }

    finish(m_editingCommand);
    m_editingCommand = 0;
    m_editing = false;
}

void
TextLayer::eraseStart(View *v, QMouseEvent *e)
{
    if (!m_model) return;

    if (!getPointToDrag(v, e->x(), e->y(), m_editingPoint)) return;

    if (m_editingCommand) {
        finish(m_editingCommand);
        m_editingCommand = 0;
    }

    m_editing = true;
}

void
TextLayer::eraseDrag(View *, QMouseEvent *)
{
}

void
TextLayer::eraseEnd(View *v, QMouseEvent *e)
{
    if (!m_model || !m_editing) return;

    m_editing = false;

    TextModel::Point p(0);
    if (!getPointToDrag(v, e->x(), e->y(), p)) return;
    if (p.frame != m_editingPoint.frame || p.height != m_editingPoint.height) return;

    m_editingCommand = new TextModel::EditCommand
            (m_model, tr("Erase Point"));

    m_editingCommand->deletePoint(m_editingPoint);

    finish(m_editingCommand);
    m_editingCommand = 0;
    m_editing = false;
}

void
TextLayer::editStart(View *v, QMouseEvent *e)
{
    //    cerr << "TextLayer::editStart(" << e->x() << "," << e->y() << ")" << endl;

    if (!m_model) return;

    if (!getPointToDrag(v, e->x(), e->y(), m_editingPoint)) {
        return;
    }

    m_editOrigin = e->pos();
    m_originalPoint = m_editingPoint;

    if (m_editingCommand) {
        finish(m_editingCommand);
        m_editingCommand = 0;
    }

    m_editing = true;
}

void
TextLayer::editDrag(View *v, QMouseEvent *e)
{
    if (!m_model || !m_editing) return;

    sv_frame_t frameDiff = v->getFrameForX(e->x()) - v->getFrameForX(m_editOrigin.x());
    double heightDiff = getHeightForY(v, e->y()) - getHeightForY(v, m_editOrigin.y());

    sv_frame_t frame = m_originalPoint.frame + frameDiff;
    double height = m_originalPoint.height + heightDiff;

    //    sv_frame_t frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = (frame / m_model->getResolution()) * m_model->getResolution();

    //    double height = getHeightForY(v, e->y());

    if (!m_editingCommand) {
        m_editingCommand = new TextModel::EditCommand(m_model, tr("Drag Label"));
    }

    m_editingCommand->deletePoint(m_editingPoint);
    m_editingPoint.frame = frame;
    m_editingPoint.height = float(height);
    m_editingCommand->addPoint(m_editingPoint);
}

void
TextLayer::editEnd(View *, QMouseEvent *)
{
    //    cerr << "TextLayer::editEnd(" << e->x() << "," << e->y() << ")" << endl;
    if (!m_model || !m_editing) return;

    if (m_editingCommand) {

        QString newName = m_editingCommand->getName();

        if (m_editingPoint.frame != m_originalPoint.frame) {
            if (m_editingPoint.height != m_originalPoint.height) {
                newName = tr("Move Label");
            } else {
                newName = tr("Move Label Horizontally");
            }
        } else {
            newName = tr("Move Label Vertically");
        }

        m_editingCommand->setName(newName);
        finish(m_editingCommand);
    }
    
    m_editingCommand = 0;
    m_editing = false;
}

bool
TextLayer::editOpen(View *v, QMouseEvent *e)
{
    if (!m_model) return false;

    TextModel::Point text(0);
    if (!getPointToDrag(v, e->x(), e->y(), text)) return false;

    QString label = text.label;

    bool ok = false;
    label = QInputDialog::getText(v, tr("Enter label"),
                                  tr("Please enter a new label:"),
                                  QLineEdit::Normal, label, &ok);
    if (ok && label != text.label) {
        TextModel::RelabelCommand *command =
                new TextModel::RelabelCommand(m_model, text, label);
        CommandHistory::getInstance()->addCommand(command);
    }

    return true;
}    

void
TextLayer::moveSelection(Selection s, sv_frame_t newStartFrame)
{
    if (!m_model) return;

    TextModel::EditCommand *command =
            new TextModel::EditCommand(m_model, tr("Drag Selection"));

    TextModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (TextModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {
            TextModel::Point newPoint(*i);
            newPoint.frame = i->frame + newStartFrame - s.getStartFrame();
            command->deletePoint(*i);
            command->addPoint(newPoint);
        }
    }

    finish(command);
}

void
TextLayer::resizeSelection(Selection s, Selection newSize)
{
    if (!m_model) return;

    TextModel::EditCommand *command =
            new TextModel::EditCommand(m_model, tr("Resize Selection"));

    TextModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    double ratio =
            double(newSize.getEndFrame() - newSize.getStartFrame()) /
            double(s.getEndFrame() - s.getStartFrame());

    for (TextModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {

            double target = double(i->frame);
            target = double(newSize.getStartFrame()) +
                    target - double(s.getStartFrame()) * ratio;

            TextModel::Point newPoint(*i);
            newPoint.frame = lrint(target);
            command->deletePoint(*i);
            command->addPoint(newPoint);
        }
    }

    finish(command);
}

void
TextLayer::deleteSelection(Selection s)
{
    if (!m_model) return;

    TextModel::EditCommand *command =
            new TextModel::EditCommand(m_model, tr("Delete Selection"));

    TextModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (TextModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {
        if (s.contains(i->frame)) command->deletePoint(*i);
    }

    finish(command);
}

void
TextLayer::copy(View *v, Selection s, Clipboard &to)
{
    if (!m_model) return;

    TextModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (TextModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {
        if (s.contains(i->frame)) {
            Clipboard::Point point(i->frame, i->height, i->label);
            point.setReferenceFrame(alignToReference(v, i->frame));
            to.addPoint(point);
        }
    }
}

bool
TextLayer::paste(View *v, const Clipboard &from, sv_frame_t /* frameOffset */, bool /* interactive */)
{
    if (!m_model) return false;

    const Clipboard::PointList &points = from.getPoints();

    bool realign = false;

    if (clipboardHasDifferentAlignment(v, from)) {

        QMessageBox::StandardButton button =
                QMessageBox::question(v, tr("Re-align pasted items?"),
                                      tr("The items you are pasting came from a layer with different source material from this one.  Do you want to re-align them in time, to match the source material for this layer?"),
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                      QMessageBox::Yes);

        if (button == QMessageBox::Cancel) {
            return false;
        }

        if (button == QMessageBox::Yes) {
            realign = true;
        }
    }

    TextModel::EditCommand *command =
            new TextModel::EditCommand(m_model, tr("Paste"));

    double valueMin = 0.0, valueMax = 1.0;
    for (Clipboard::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {
        if (i->haveValue()) {
            if (i->getValue() < valueMin) valueMin = i->getValue();
            if (i->getValue() > valueMax) valueMax = i->getValue();
        }
    }
    if (valueMax < valueMin + 1.0) valueMax = valueMin + 1.0;

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

        TextModel::Point newPoint(frame);

        if (i->haveValue()) {
            newPoint.height = float((i->getValue() - valueMin) / (valueMax - valueMin));
        } else {
            newPoint.height = 0.5f;
        }

        if (i->haveLabel()) {
            newPoint.label = i->getLabel();
        } else if (i->haveValue()) {
            newPoint.label = QString("%1").arg(i->getValue());
        } else {
            newPoint.label = tr("New Point");
        }
        
        command->addPoint(newPoint);
    }

    finish(command);
    return true;
}

int
TextLayer::getDefaultColourHint(bool darkbg, bool &impose)
{
    impose = false;
    return ColourDatabase::getInstance()->getColourIndex
            (QString(darkbg ? "Bright Orange" : "Orange"));
}

void
TextLayer::toXml(QTextStream &stream,
                 QString indent, QString extraAttributes) const
{
    SingleColourLayer::toXml(stream, indent, extraAttributes);
}

void
TextLayer::setProperties(const QXmlAttributes &attributes)
{
    SingleColourLayer::setProperties(attributes);
}

