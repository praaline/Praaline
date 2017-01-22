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

#include "ImageLayer.h"

#include "data/model/Model.h"
#include "base/RealTime.h"
#include "base/Profiler.h"
#include "view/View.h"

#include "data/model/ImageModel.h"
#include "data/fileio/FileSource.h"

#include "widgets/ImageDialog.h"
#include "widgets/ProgressDialog.h"

#include "LayerFactory.h"

#include <QPainter>
#include <QMouseEvent>
#include <QInputDialog>
#include <QMutexLocker>
#include <QTextStream>
#include <QMessageBox>

#include <iostream>
#include <cmath>

ImageLayer::ImageMap
ImageLayer::m_images;

QMutex
ImageLayer::m_imageMapMutex;

ImageLayer::ImageLayer() :
    Layer(),
    m_model(0),
    m_editing(false),
    m_originalPoint(0, "", ""),
    m_editingPoint(0, "", ""),
    m_editingCommand(0)
{
}

ImageLayer::~ImageLayer()
{
    for (FileSourceMap::iterator i = m_fileSources.begin();
         i != m_fileSources.end(); ++i) {
        delete i->second;
    }
}

bool ImageLayer::trySetModel(Model *model)
{
    if (trySetModelHelper<ImageLayer, ImageModel>(this, model))
        return true;
    return false;
}

void
ImageLayer::setModel(ImageModel *model)
{
    if (m_model == model) return;
    m_model = model;

    connectSignals(m_model);

    emit modelReplaced();
}

// static
Model *ImageLayer::createEmptyModel(Model *baseModel)
{
    return new ImageModel(baseModel->getSampleRate(), 1, true);
}

Layer::PropertyList
ImageLayer::getProperties() const
{
    return Layer::getProperties();
}

QString
ImageLayer::getPropertyLabel(const PropertyName &) const
{
    return "";
}

Layer::PropertyType
ImageLayer::getPropertyType(const PropertyName &name) const
{
    return Layer::getPropertyType(name);
}

int
ImageLayer::getPropertyRangeAndValue(const PropertyName &name,
                                     int *min, int *max, int *deflt) const
{
    return Layer::getPropertyRangeAndValue(name, min, max, deflt);
}

QString
ImageLayer::getPropertyValueLabel(const PropertyName &name,
                                  int value) const
{
    return Layer::getPropertyValueLabel(name, value);
}

void
ImageLayer::setProperty(const PropertyName &name, int value)
{
    Layer::setProperty(name, value);
}

bool
ImageLayer::getValueExtents(double &, double &, bool &, QString &) const
{
    return false;
}

bool
ImageLayer::isLayerScrollable(const View *) const
{
    return true;
}


ImageModel::PointList
ImageLayer::getLocalPoints(View *v, int x, int ) const
{
    if (!m_model) return ImageModel::PointList();

    //    cerr << "ImageLayer::getLocalPoints(" << x << "," << y << "):";
    const ImageModel::PointList &points(m_model->getPoints());

    ImageModel::PointList rv;

    for (ImageModel::PointList::const_iterator i = points.begin();
         i != points.end(); ) {

        const ImageModel::Point &p(*i);
        int px = v->getXForFrame(p.frame);
        if (px > x) break;

        ++i;
        if (i != points.end()) {
            int nx = v->getXForFrame((*i).frame);
            if (nx < x) {
                // as we aim not to overlap the images, if the following
                // image begins to the left of a point then the current
                // one may be assumed to end to the left of it as well.
                continue;
            }
        }

        // this image is a candidate, test it properly

        int width = 32;
        if (m_scaled[v].find(p.image) != m_scaled[v].end()) {
            width = m_scaled[v][p.image].width();
            //            cerr << "scaled width = " << width << endl;
        }

        if (x >= px && x < px + width) {
            rv.insert(p);
        }
    }

    //    cerr << rv.size() << " point(s)" << endl;

    return rv;
}

QString
ImageLayer::getFeatureDescription(View *v, QPoint &pos) const
{
    int x = pos.x();

    if (!m_model || !m_model->getSampleRate()) return "";

    ImageModel::PointList points = getLocalPoints(v, x, pos.y());

    if (points.empty()) {
        if (!m_model->isReady()) {
            return tr("In progress");
        } else {
            return "";
        }
    }

    //    int useFrame = points.begin()->frame;

    //    RealTime rt = RealTime::frame2RealTime(useFrame, m_model->getSampleRate());

    QString text;
    /*
    if (points.begin()->label == "") {
    text = QString(tr("Time:\t%1\nHeight:\t%2\nLabel:\t%3"))
        .arg(rt.toText(true).c_str())
        .arg(points.begin()->height)
        .arg(points.begin()->label);
    }

    pos = QPoint(v->getXForFrame(useFrame),
         getYForHeight(v, points.begin()->height));
*/
    return text;
}


//!!! too much overlap with TimeValueLayer/TimeInstantLayer/TextLayer

bool
ImageLayer::snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    if (!m_model) {
        return Layer::snapToFeatureFrame(v, frame, resolution, snap, y);
    }

    resolution = m_model->getResolution();
    ImageModel::PointList points;

    if (snap == SnapNeighbouring) {

        points = getLocalPoints(v, v->getXForFrame(frame), -1);
        if (points.empty()) return false;
        frame = points.begin()->frame;
        return true;
    }

    points = m_model->getPoints(frame, frame);
    sv_frame_t snapped = frame;
    bool found = false;

    for (ImageModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {

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

            ImageModel::PointList::const_iterator j = i;
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
ImageLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    if (!m_model || !m_model->isOK()) return;

    sv_samplerate_t sampleRate = m_model->getSampleRate();
    if (!sampleRate) return;

    //    Profiler profiler("ImageLayer::paint", true);

    //    int x0 = rect.left(), x1 = rect.right();
    int x0 = 0, x1 = v->width();

    sv_frame_t frame0 = v->getFrameForX(x0);
    sv_frame_t frame1 = v->getFrameForX(x1);

    ImageModel::PointList points(m_model->getPoints(frame0, frame1));
    if (points.empty()) return;

    paint.save();
    paint.setClipRect(rect.x(), 0, rect.width(), v->height());

    QColor penColour;
    penColour = v->getForeground();

    QColor brushColour;
    brushColour = v->getBackground();

    int h, s, val;
    brushColour.getHsv(&h, &s, &val);
    brushColour.setHsv(h, s, 255, 240);

    paint.setPen(penColour);
    paint.setBrush(brushColour);
    paint.setRenderHint(QPainter::Antialiasing, true);

    for (ImageModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {

        const ImageModel::Point &p(*i);

        int x = v->getXForFrame(p.frame);

        int nx = x + 2000;
        ImageModel::PointList::const_iterator j = i;
        ++j;
        if (j != points.end()) {
            int jx = v->getXForFrame(j->frame);
            if (jx < nx) nx = jx;
        }

        drawImage(v, paint, p, x, nx);
    }

    paint.setRenderHint(QPainter::Antialiasing, false);
    paint.restore();
}

void
ImageLayer::drawImage(View *v, QPainter &paint, const ImageModel::Point &p,
                      int x, int nx) const
{
    QString label = p.label;
    QString imageName = p.image;

    QImage image;
    QString additionalText;

    QSize imageSize;
    if (!getImageOriginalSize(imageName, imageSize)) {
        image = QImage(":icons/emptypage.png");
        imageSize = image.size();
        additionalText = imageName;
    }

    int topMargin = 10;
    int bottomMargin = 10;
    int spacing = 5;

    if (v->height() < 100) {
        topMargin = 5;
        bottomMargin = 5;
    }

    int maxBoxHeight = v->height() - topMargin - bottomMargin;

    int availableWidth = nx - x - 3;
    if (availableWidth < 20) availableWidth = 20;

    QRect labelRect;

    if (label != "") {

        int likelyHeight = v->height() / 4;

        int likelyWidth = // available height times image aspect
                ((maxBoxHeight - likelyHeight) * imageSize.width())
                / imageSize.height();

        if (likelyWidth > imageSize.width()) {
            likelyWidth = imageSize.width();
        }

        if (likelyWidth > availableWidth) {
            likelyWidth = availableWidth;
        }

        int singleWidth = paint.fontMetrics().width(label);
        if (singleWidth < availableWidth && singleWidth < likelyWidth * 2) {
            likelyWidth = singleWidth + 4;
        }

        labelRect = paint.fontMetrics().boundingRect
                (QRect(0, 0, likelyWidth, likelyHeight),
                 Qt::AlignCenter | Qt::TextWordWrap, label);

        labelRect.setWidth(labelRect.width() + 6);
    }

    if (image.isNull()) {
        image = getImage(v, imageName,
                         QSize(availableWidth,
                               maxBoxHeight - labelRect.height()));
    }

    int boxWidth = image.width();
    if (boxWidth < labelRect.width()) {
        boxWidth = labelRect.width();
    }

    int boxHeight = image.height();
    if (label != "") {
        boxHeight += labelRect.height() + spacing;
    }

    int division = image.height();

    if (additionalText != "") {

        paint.save();

        QFont font(paint.font());
        font.setItalic(true);
        paint.setFont(font);

        int tw = paint.fontMetrics().width(additionalText);
        if (tw > availableWidth) {
            tw = availableWidth;
        }
        if (boxWidth < tw) {
            boxWidth = tw;
        }
        boxHeight += paint.fontMetrics().height();
        division += paint.fontMetrics().height();
    }

    bottomMargin = v->height() - topMargin - boxHeight;
    if (bottomMargin > topMargin + v->height()/7) {
        topMargin += v->height()/8;
        bottomMargin -= v->height()/8;
    }

    paint.drawRect(x - 1,
                   topMargin - 1,
                   boxWidth + 2,
                   boxHeight + 2);

    int imageY;
    if (label != "") {
        imageY = topMargin + labelRect.height() + spacing;
    } else {
        imageY = topMargin;
    }

    paint.drawImage(x + (boxWidth - image.width())/2,
                    imageY,
                    image);

    if (additionalText != "") {
        paint.drawText(x,
                       imageY + image.height() + paint.fontMetrics().ascent(),
                       additionalText);
        paint.restore();
    }

    if (label != "") {
        paint.drawLine(x,
                       topMargin + labelRect.height() + spacing,
                       x + boxWidth,
                       topMargin + labelRect.height() + spacing);

        paint.drawText(QRect(x,
                             topMargin,
                             boxWidth,
                             labelRect.height()),
                       Qt::AlignCenter | Qt::TextWordWrap,
                       label);
    }
}

void
ImageLayer::setLayerDormant(const View *v, bool dormant)
{
    if (dormant) {
        // Delete the images named in the view's scaled map from the
        // general image map as well.  They can always be re-loaded
        // if it turns out another view still needs them.
        QMutexLocker locker(&m_imageMapMutex);
        for (ImageMap::iterator i = m_scaled[v].begin();
             i != m_scaled[v].end(); ++i) {
            m_images.erase(i->first);
        }
        m_scaled.erase(v);
    }
}

//!!! how to reap no-longer-used images?

bool
ImageLayer::getImageOriginalSize(QString name, QSize &size) const
{
    //    cerr << "getImageOriginalSize: \"" << name << "\"" << endl;

    QMutexLocker locker(&m_imageMapMutex);
    if (m_images.find(name) == m_images.end()) {
        //        cerr << "don't have, trying to open local" << endl;
        m_images[name] = QImage(getLocalFilename(name));
    }
    if (m_images[name].isNull()) {
        //        cerr << "null image" << endl;
        return false;
    } else {
        size = m_images[name].size();
        return true;
    }
}

QImage 
ImageLayer::getImage(View *v, QString name, QSize maxSize) const
{
    //    cerr << "ImageLayer::getImage(" << v << ", " << name << ", ("
    //              << maxSize.width() << "x" << maxSize.height() << "))" << endl;

    if (!m_scaled[v][name].isNull()  &&
            ((m_scaled[v][name].width()  == maxSize.width() &&
              m_scaled[v][name].height() <= maxSize.height()) ||
             (m_scaled[v][name].width()  <= maxSize.width() &&
              m_scaled[v][name].height() == maxSize.height()))) {
        //        cerr << "cache hit" << endl;
        return m_scaled[v][name];
    }

    QMutexLocker locker(&m_imageMapMutex);

    if (m_images.find(name) == m_images.end()) {
        m_images[name] = QImage(getLocalFilename(name));
    }

    if (m_images[name].isNull()) {
        //        cerr << "null image" << endl;
        m_scaled[v][name] = QImage();
    } else if (m_images[name].width() <= maxSize.width() &&
               m_images[name].height() <= maxSize.height()) {
        m_scaled[v][name] = m_images[name];
    } else {
        m_scaled[v][name] =
                m_images[name].scaled(maxSize,
                                      Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation);
    }

    return m_scaled[v][name];
}

void
ImageLayer::drawStart(View *v, QMouseEvent *e)
{
    //    cerr << "ImageLayer::drawStart(" << e->x() << "," << e->y() << ")" << endl;

    if (!m_model) {
        cerr << "ImageLayer::drawStart: no model" << endl;
        return;
    }

    sv_frame_t frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    m_editingPoint = ImageModel::Point(frame, "", "");
    m_originalPoint = m_editingPoint;

    if (m_editingCommand) finish(m_editingCommand);
    m_editingCommand = new ImageModel::EditCommand(m_model, "Add Image");
    m_editingCommand->addPoint(m_editingPoint);

    m_editing = true;
}

void
ImageLayer::drawDrag(View *v, QMouseEvent *e)
{
    //    cerr << "ImageLayer::drawDrag(" << e->x() << "," << e->y() << ")" << endl;

    if (!m_model || !m_editing) return;

    sv_frame_t frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    m_editingCommand->deletePoint(m_editingPoint);
    m_editingPoint.frame = frame;
    m_editingCommand->addPoint(m_editingPoint);
}

void
ImageLayer::drawEnd(View *, QMouseEvent *)
{
    //    cerr << "ImageLayer::drawEnd(" << e->x() << "," << e->y() << ")" << endl;
    if (!m_model || !m_editing) return;

    ImageDialog dialog(tr("Select image"), "", "");

    if (dialog.exec() == QDialog::Accepted) {

        checkAddSource(dialog.getImage());

        ImageModel::ChangeImageCommand *command =
                new ImageModel::ChangeImageCommand
                (m_model, m_editingPoint, dialog.getImage(), dialog.getLabel());
        m_editingCommand->addCommand(command);
    } else {
        m_editingCommand->deletePoint(m_editingPoint);
    }

    finish(m_editingCommand);
    m_editingCommand = 0;
    m_editing = false;
}

bool
ImageLayer::addImage(sv_frame_t frame, QString url)
{
    QImage image(getLocalFilename(url));
    if (image.isNull()) {
        cerr << "Failed to open image from url \"" << url << "\" (local filename \"" << getLocalFilename(url) << "\"" << endl;
        delete m_fileSources[url];
        m_fileSources.erase(url);
        return false;
    }

    ImageModel::Point point(frame, url, "");
    ImageModel::EditCommand *command =
            new ImageModel::EditCommand(m_model, "Add Image");
    command->addPoint(point);
    finish(command);
    return true;
}

void
ImageLayer::editStart(View *v, QMouseEvent *e)
{
    //    cerr << "ImageLayer::editStart(" << e->x() << "," << e->y() << ")" << endl;

    if (!m_model) return;

    ImageModel::PointList points = getLocalPoints(v, e->x(), e->y());
    if (points.empty()) return;

    m_editOrigin = e->pos();
    m_editingPoint = *points.begin();
    m_originalPoint = m_editingPoint;

    if (m_editingCommand) {
        finish(m_editingCommand);
        m_editingCommand = 0;
    }

    m_editing = true;
}

void
ImageLayer::editDrag(View *v, QMouseEvent *e)
{
    if (!m_model || !m_editing) return;

    sv_frame_t frameDiff = v->getFrameForX(e->x()) - v->getFrameForX(m_editOrigin.x());
    sv_frame_t frame = m_originalPoint.frame + frameDiff;

    if (frame < 0) frame = 0;
    frame = (frame / m_model->getResolution()) * m_model->getResolution();

    if (!m_editingCommand) {
        m_editingCommand = new ImageModel::EditCommand(m_model, tr("Move Image"));
    }

    m_editingCommand->deletePoint(m_editingPoint);
    m_editingPoint.frame = frame;
    m_editingCommand->addPoint(m_editingPoint);
}

void
ImageLayer::editEnd(View *, QMouseEvent *)
{
    //    cerr << "ImageLayer::editEnd(" << e->x() << "," << e->y() << ")" << endl;
    if (!m_model || !m_editing) return;

    if (m_editingCommand) {
        finish(m_editingCommand);
    }
    
    m_editingCommand = 0;
    m_editing = false;
}

bool
ImageLayer::editOpen(View *v, QMouseEvent *e)
{
    if (!m_model) return false;

    ImageModel::PointList points = getLocalPoints(v, e->x(), e->y());
    if (points.empty()) return false;

    QString image = points.begin()->image;
    QString label = points.begin()->label;

    ImageDialog dialog(tr("Select image"),
                       image,
                       label);

    if (dialog.exec() == QDialog::Accepted) {

        checkAddSource(dialog.getImage());

        ImageModel::ChangeImageCommand *command =
                new ImageModel::ChangeImageCommand
                (m_model, *points.begin(), dialog.getImage(), dialog.getLabel());

        CommandHistory::getInstance()->addCommand(command);
    }

    return true;
}    

void
ImageLayer::moveSelection(Selection s, sv_frame_t newStartFrame)
{
    if (!m_model) return;

    ImageModel::EditCommand *command =
            new ImageModel::EditCommand(m_model, tr("Drag Selection"));

    ImageModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (ImageModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {
            ImageModel::Point newPoint(*i);
            newPoint.frame = i->frame + newStartFrame - s.getStartFrame();
            command->deletePoint(*i);
            command->addPoint(newPoint);
        }
    }

    finish(command);
}

void
ImageLayer::resizeSelection(Selection s, Selection newSize)
{
    if (!m_model) return;

    ImageModel::EditCommand *command =
            new ImageModel::EditCommand(m_model, tr("Resize Selection"));

    ImageModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    double ratio =
            double(newSize.getEndFrame() - newSize.getStartFrame()) /
            double(s.getEndFrame() - s.getStartFrame());

    for (ImageModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {

            double target = double(i->frame);
            target = double(newSize.getStartFrame()) +
                    target - double(s.getStartFrame()) * ratio;

            ImageModel::Point newPoint(*i);
            newPoint.frame = lrint(target);
            command->deletePoint(*i);
            command->addPoint(newPoint);
        }
    }

    finish(command);
}

void
ImageLayer::deleteSelection(Selection s)
{
    if (!m_model) return;

    ImageModel::EditCommand *command =
            new ImageModel::EditCommand(m_model, tr("Delete Selection"));

    ImageModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (ImageModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {
        if (s.contains(i->frame)) command->deletePoint(*i);
    }

    finish(command);
}

void
ImageLayer::copy(View *v, Selection s, Clipboard &to)
{
    if (!m_model) return;

    ImageModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (ImageModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {
        if (s.contains(i->frame)) {
            Clipboard::Point point(i->frame, i->label);
            point.setReferenceFrame(alignToReference(v, i->frame));
            to.addPoint(point);
        }
    }
}

bool
ImageLayer::paste(View *v, const Clipboard &from, sv_frame_t /* frameOffset */, bool /* interactive */)
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

    ImageModel::EditCommand *command =
            new ImageModel::EditCommand(m_model, tr("Paste"));

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

        ImageModel::Point newPoint(frame);

        //!!! inadequate
        
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

QString
ImageLayer::getLocalFilename(QString img) const
{
    if (m_fileSources.find(img) == m_fileSources.end()) {
        checkAddSource(img);
        if (m_fileSources.find(img) == m_fileSources.end()) {
            return img;
        }
    }
    return m_fileSources[img]->getLocalFilename();
}

void
ImageLayer::checkAddSource(QString img) const
{
    cerr << "ImageLayer::checkAddSource(" << img << "): yes, trying..." << endl;

    if (m_fileSources.find(img) != m_fileSources.end()) {
        return;
    }

    ProgressDialog dialog(tr("Opening image URL..."), true, 2000);
    FileSource *rf = new FileSource(img, &dialog);
    if (rf->isOK()) {
        cerr << "ok, adding it (local filename = " << rf->getLocalFilename() << ")" << endl;
        m_fileSources[img] = rf;
        connect(rf, SIGNAL(ready()), this, SLOT(fileSourceReady()));
    } else {
        delete rf;
    }
}

void
ImageLayer::checkAddSources()
{
    const ImageModel::PointList &points(m_model->getPoints());

    for (ImageModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {
        
        checkAddSource((*i).image);
    }
}

void
ImageLayer::fileSourceReady()
{
    //    cerr << "ImageLayer::fileSourceReady" << endl;

    FileSource *rf = dynamic_cast<FileSource *>(sender());
    if (!rf) return;

    QString img;
    for (FileSourceMap::const_iterator i = m_fileSources.begin();
         i != m_fileSources.end(); ++i) {
        if (i->second == rf) {
            img = i->first;
            //            cerr << "it's image \"" << img << "\"" << endl;
            break;
        }
    }
    if (img == "") return;

    QMutexLocker locker(&m_imageMapMutex);
    m_images.erase(img);
    for (ViewImageMap::iterator i = m_scaled.begin(); i != m_scaled.end(); ++i) {
        i->second.erase(img);
        emit modelChanged();
    }
}

void
ImageLayer::toXml(QTextStream &stream,
                  QString indent, QString extraAttributes) const
{
    Layer::toXml(stream, indent, extraAttributes);
}

void
ImageLayer::setProperties(const QXmlAttributes &)
{
}

