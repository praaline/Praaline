/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2007 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "Pane.h"
#include "layer/Layer.h"
#include "data/model/Model.h"
#include "base/ZoomConstraint.h"
#include "base/RealTime.h"
#include "base/Profiler.h"
#include "ViewManager.h"
#include "widgets/CommandHistory.h"
#include "widgets/TextAbbrev.h"
#include "base/Preferences.h"
#include "layer/WaveformLayer.h"
#include "layer/TimeRulerLayer.h"

// GF: added so we can propagate the mouse move event to the note layer for context handling.
#include "layer/LayerFactory.h"
#include "layer/FlexiNoteLayer.h"


//!!! ugh
#include "data/model/WaveFileModel.h"

#include <QPaintEvent>
#include <QPainter>
#include <QBitmap>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QCursor>
#include <QTextStream>
#include <QMimeData>
#include <QApplication>

#include <iostream>
#include <cmath>

//!!! for HUD -- pull out into a separate class
#include <QFrame>
#include <QGridLayout>
#include <QPushButton>
#include "widgets/Thumbwheel.h"
#include "widgets/Panner.h"
#include "widgets/RangeInputDialog.h"
#include "widgets/NotifyingPushButton.h"

#include "widgets/KeyReference.h" //!!! should probably split KeyReference into a data class in base and another that shows the widget

//#define DEBUG_PANE




QCursor *Pane::m_measureCursor1 = 0;
QCursor *Pane::m_measureCursor2 = 0;

Pane::Pane(QWidget *w) :
    View(w, true),
    m_identifyFeatures(false),
    m_clickedInRange(false),
    m_shiftPressed(false),
    m_ctrlPressed(false),
    m_altPressed(false),
    m_navigating(false),
    m_resizing(false),
    m_editing(false),
    m_releasing(false),
    m_centreLineVisible(true),
    m_scaleWidth(0),
    m_pendingWheelAngle(0),
    m_headsUpDisplay(0),
    m_vpan(0),
    m_hthumb(0),
    m_vthumb(0),
    m_reset(0),
    m_mouseInWidget(false),
    m_playbackFrameMoveScheduled(false),
    m_playbackFrameMoveTo(0)
{
    setObjectName("Pane");
    setMouseTracking(true);
    setAcceptDrops(true);
    
    updateHeadsUpDisplay();

    connect(this, SIGNAL(regionOutlined(QRect)), 
            this, SLOT(zoomToRegion(QRect)));

    cerr << "Pane::Pane(" << this << ") returning" << endl;
}

void
Pane::updateHeadsUpDisplay()
{
    Profiler profiler("Pane::updateHeadsUpDisplay");

    if (!isVisible()) return;

/*
    int count = 0;
    int currentLevel = 1;
    int level = 1;
    while (true) {
        if (getZoomLevel() == level) currentLevel = count;
        int newLevel = getZoomConstraintBlockSize(level + 1,
                                                  ZoomConstraint::RoundUp);
        if (newLevel == level) break;
        if (newLevel == 131072) break; //!!! just because
        level = newLevel;
        ++count;
    }

    cerr << "Have " << count+1 << " zoom levels" << endl;
*/

    Layer *layer = 0;
    if (getLayerCount() > 0) layer = getLayer(getLayerCount() - 1);

    if (!m_headsUpDisplay) {

        m_headsUpDisplay = new QFrame(this);

        QGridLayout *layout = new QGridLayout;
        layout->setMargin(0);
        layout->setSpacing(0);
        m_headsUpDisplay->setLayout(layout);
        
        m_hthumb = new Thumbwheel(Qt::Horizontal);
        m_hthumb->setObjectName(tr("Horizontal Zoom"));
        m_hthumb->setCursor(Qt::ArrowCursor);
        layout->addWidget(m_hthumb, 1, 0, 1, 2);
        m_hthumb->setFixedWidth(70);
        m_hthumb->setFixedHeight(16);
        m_hthumb->setDefaultValue(0);
        m_hthumb->setSpeed(0.6f);
        connect(m_hthumb, SIGNAL(valueChanged(int)), this, 
                SLOT(horizontalThumbwheelMoved(int)));
        connect(m_hthumb, SIGNAL(mouseEntered()), this, SLOT(mouseEnteredWidget()));
        connect(m_hthumb, SIGNAL(mouseLeft()), this, SLOT(mouseLeftWidget()));

        m_vpan = new Panner;
        m_vpan->setCursor(Qt::ArrowCursor);
        layout->addWidget(m_vpan, 0, 1);
        m_vpan->setFixedWidth(12);
        m_vpan->setFixedHeight(70);
        m_vpan->setAlpha(80, 130);
        connect(m_vpan, SIGNAL(rectExtentsChanged(float, float, float, float)),
                this, SLOT(verticalPannerMoved(float, float, float, float)));
        connect(m_vpan, SIGNAL(doubleClicked()),
                this, SLOT(editVerticalPannerExtents()));
        connect(m_vpan, SIGNAL(mouseEntered()), this, SLOT(mouseEnteredWidget()));
        connect(m_vpan, SIGNAL(mouseLeft()), this, SLOT(mouseLeftWidget()));

        m_vthumb = new Thumbwheel(Qt::Vertical);
        m_vthumb->setObjectName(tr("Vertical Zoom"));
        m_vthumb->setCursor(Qt::ArrowCursor);
        layout->addWidget(m_vthumb, 0, 2);
        m_vthumb->setFixedWidth(16);
        m_vthumb->setFixedHeight(70);
        connect(m_vthumb, SIGNAL(valueChanged(int)), this, 
                SLOT(verticalThumbwheelMoved(int)));
        connect(m_vthumb, SIGNAL(mouseEntered()), this, SLOT(mouseEnteredWidget()));
        connect(m_vthumb, SIGNAL(mouseLeft()), this, SLOT(mouseLeftWidget()));

        if (layer) {
            RangeMapper *rm = layer->getNewVerticalZoomRangeMapper();
            if (rm) m_vthumb->setRangeMapper(rm);
        }

        m_reset = new NotifyingPushButton;
        m_reset->setFlat(true);
        m_reset->setCursor(Qt::ArrowCursor);
        m_reset->setFixedHeight(16);
        m_reset->setFixedWidth(16);
        m_reset->setIcon(QPixmap(":/icons/visualiser/zoom-reset.png"));
        m_reset->setToolTip(tr("Reset zoom to default"));
        layout->addWidget(m_reset, 1, 2);
        
        layout->setColumnStretch(0, 20);

        connect(m_reset, SIGNAL(clicked()), m_hthumb, SLOT(resetToDefault()));
        connect(m_reset, SIGNAL(clicked()), m_vthumb, SLOT(resetToDefault()));
        connect(m_reset, SIGNAL(clicked()), m_vpan, SLOT(resetToDefault()));
        connect(m_reset, SIGNAL(mouseEntered()), this, SLOT(mouseEnteredWidget()));
        connect(m_reset, SIGNAL(mouseLeft()), this, SLOT(mouseLeftWidget()));
    }

    int count = 0;
    int current = 0;
    int level = 1;

    //!!! pull out into function (presumably in View)
    bool haveConstraint = false;
    for (LayerList::const_iterator i = m_layerStack.begin(); i != m_layerStack.end();
         ++i) {
        if ((*i)->getZoomConstraint() && !(*i)->supportsOtherZoomLevels()) {
            haveConstraint = true;
            break;
        }
    }

    if (haveConstraint) {
        while (true) {
            if (getZoomLevel() == level) current = count;
            int newLevel = getZoomConstraintBlockSize(level + 1,
                                                      ZoomConstraint::RoundUp);
            if (newLevel == level) break;
            level = newLevel;
            if (++count == 50) break;
        }
    } else {
        // if we have no particular constraints, we can really spread out
        while (true) {
            if (getZoomLevel() >= level) current = count;
            int step = level / 10;
            int pwr = 0;
            while (step > 0) {
                ++pwr;
                step /= 2;
            }
            step = 1;
            while (pwr > 0) {
                step *= 2;
                --pwr;
            }
//            cerr << level << endl;
            level += step;
            if (++count == 100 || level > 262144) break;
        }
    }

//    cerr << "Have " << count << " zoom levels" << endl;

    m_hthumb->setMinimumValue(0);
    m_hthumb->setMaximumValue(count);
    m_hthumb->setValue(count - current);

//    cerr << "set value to " << count-current << endl;

//    cerr << "default value is " << m_hthumb->getDefaultValue() << endl;

    if (count != 50 && m_hthumb->getDefaultValue() == 0) {
        m_hthumb->setDefaultValue(count - current);
//        cerr << "set default value to " << m_hthumb->getDefaultValue() << endl;
    }

    bool haveVThumb = false;

    if (layer) {
        int defaultStep = 0;
        int max = layer->getVerticalZoomSteps(defaultStep);
        if (max == 0) {
            m_vthumb->hide();
        } else {
            haveVThumb = true;
            m_vthumb->show();
            m_vthumb->blockSignals(true);
            m_vthumb->setMinimumValue(0);
            m_vthumb->setMaximumValue(max);
            m_vthumb->setDefaultValue(defaultStep);
            m_vthumb->setValue(layer->getCurrentVerticalZoomStep());
            m_vthumb->blockSignals(false);

//            cerr << "Vertical thumbwheel: min 0, max " << max
//                      << ", default " << defaultStep << ", value "
//                      << m_vthumb->getValue() << endl;

        }
    }

    updateVerticalPanner();

    if (m_manager && m_manager->getZoomWheelsEnabled() &&
        width() > 120 && height() > 100) {
        if (!m_headsUpDisplay->isVisible()) {
            m_headsUpDisplay->show();
        }
        if (haveVThumb) {
            m_headsUpDisplay->setFixedHeight(m_vthumb->height() + m_hthumb->height());
            m_headsUpDisplay->move(width() - 86, height() - 86);
        } else {
            m_headsUpDisplay->setFixedHeight(m_hthumb->height());
            m_headsUpDisplay->move(width() - 86, height() - 16);
        }
    } else {
        m_headsUpDisplay->hide();
    }
}

void
Pane::updateVerticalPanner()
{
    if (!m_vpan || !m_manager || !m_manager->getZoomWheelsEnabled()) return;

    // In principle we should show or hide the panner on the basis of
    // whether the top layer has adjustable display extents, and we do
    // that below.  However, we have no basis for layout of the panner
    // if the vertical scroll wheel is not also present.  So if we
    // have no vertical scroll wheel, we should remove the panner as
    // well.  Ideally any layer that implements display extents should
    // implement vertical zoom steps as well, but they don't all at
    // the moment.

    Layer *layer = 0;
    if (getLayerCount() > 0) layer = getLayer(getLayerCount() - 1);
    int discard;
    if (layer && layer->getVerticalZoomSteps(discard) == 0) {
        m_vpan->hide();
        return;
    }

    double vmin, vmax, dmin, dmax;
    if (getTopLayerDisplayExtents(vmin, vmax, dmin, dmax) && vmax != vmin) {
        double y0 = (dmin - vmin) / (vmax - vmin);
        double y1 = (dmax - vmin) / (vmax - vmin);
        m_vpan->blockSignals(true);
        m_vpan->setRectExtents(0, float(1.0 - y1), 1, float(y1 - y0));
        m_vpan->blockSignals(false);
        m_vpan->show();
    } else {
        m_vpan->hide();
    }
}

bool
Pane::shouldIlluminateLocalFeatures(const Layer *layer, QPoint &pos) const
{
    QPoint discard;
    bool b0, b1;

    if (m_manager && m_manager->getToolModeFor(this) == ViewManager::MeasureMode) {
        return false;
    }
    
    if (m_manager && !m_manager->shouldIlluminateLocalFeatures()) {
        return false;
    }

    if (layer == getInteractionLayer() &&
        !shouldIlluminateLocalSelection(discard, b0, b1)) {

        pos = m_identifyPoint;
        return m_identifyFeatures;
    }

    return false;
}

bool
Pane::shouldIlluminateLocalSelection(QPoint &pos,
                     bool &closeToLeft,
                     bool &closeToRight) const
{
    if (m_identifyFeatures &&
        m_manager &&
        m_manager->getToolModeFor(this) == ViewManager::EditMode &&
        !m_manager->getSelections().empty() &&
        !selectionIsBeingEdited()) {

        Selection s(getSelectionAt(m_identifyPoint.x(),
                                   closeToLeft, closeToRight));

        if (!s.isEmpty()) {
            if (getInteractionLayer() && getInteractionLayer()->isLayerEditable()) {
            
                pos = m_identifyPoint;
                return true;
            }
        }
    }

    return false;
}

bool
Pane::selectionIsBeingEdited() const
{
    if (!m_editingSelection.isEmpty()) {
    if (m_mousePos != m_clickPos &&
        getFrameForX(m_mousePos.x()) != getFrameForX(m_clickPos.x())) {
        return true;
    }
    }
    return false;
}

void
Pane::setCentreLineVisible(bool visible)
{
    m_centreLineVisible = visible;
    update();
}

void
Pane::paintEvent(QPaintEvent *e)
{
//    Profiler profiler("Pane::paintEvent", true);

    QPainter paint;

    QRect r(rect());
    if (e) r = e->rect();

    View::paintEvent(e);

    paint.begin(this);
    setPaintFont(paint);

    if (e) paint.setClipRect(r);

    ViewManager::ToolMode toolMode = ViewManager::NavigateMode;
    if (m_manager) toolMode = m_manager->getToolModeFor(this);

    if (m_manager &&
        m_mouseInWidget &&
        toolMode == ViewManager::MeasureMode) {

        for (LayerList::iterator vi = m_layerStack.end(); vi != m_layerStack.begin(); ) {
            --vi;

            std::vector<QRect> crosshairExtents;

            if ((*vi)->getCrosshairExtents(this, paint, m_identifyPoint,
                                           crosshairExtents)) {
                (*vi)->paintCrosshairs(this, paint, m_identifyPoint);
                break;
            } else if ((*vi)->isLayerOpaque()) {
                break;
            }
        }
    }

    Layer *topLayer = getTopLayer();
    bool haveSomeTimeXAxis = false;

    const Model *waveformModel = 0; // just for reporting purposes
    const Model *workModel = 0;

    for (LayerList::iterator vi = m_layerStack.end(); vi != m_layerStack.begin(); ) {
        --vi;
        if (!haveSomeTimeXAxis && (*vi)->hasTimeXAxis()) {
            haveSomeTimeXAxis = true;
        }
        if (dynamic_cast<WaveformLayer *>(*vi)) {
            waveformModel = (*vi)->getModel();
            workModel = waveformModel;
        } else {
            Model *m = (*vi)->getModel();
            if (dynamic_cast<WaveFileModel *>(m)) {
                workModel = m;
            } else if (m && dynamic_cast<WaveFileModel *>(m->getSourceModel())) {
                workModel = m->getSourceModel();
            }
        }
                
        if (waveformModel && workModel && haveSomeTimeXAxis) break;
    }

    m_scaleWidth = 0;

    if (workModel && hasTopLayerTimeXAxis()) {
        drawModelTimeExtents(r, paint, workModel);
    }

    if (m_manager && m_manager->shouldShowVerticalScale() && topLayer) {
        drawVerticalScale(r, topLayer, paint);
    }

    if (m_identifyFeatures &&
        m_manager && m_manager->shouldIlluminateLocalFeatures() &&
        topLayer) {
        drawFeatureDescription(topLayer, paint);
    }
    
    sv_samplerate_t sampleRate = getModelsSampleRate();
    paint.setBrush(Qt::NoBrush);

    if (m_centreLineVisible &&
        m_manager &&
        m_manager->shouldShowCentreLine()) {
        drawCentreLine(sampleRate, paint, !haveSomeTimeXAxis);
    }
    
    paint.setPen(QColor(50, 50, 50));

    if (waveformModel &&
        sampleRate &&
        m_manager &&
        m_manager->shouldShowDuration()) {
        drawDurationAndRate(r, waveformModel, sampleRate, paint);
    }

    bool haveWorkTitle = false;

    if (workModel &&
        m_manager &&
        m_manager->shouldShowWorkTitle()) {
        drawWorkTitle(r, paint, workModel);
        haveWorkTitle = true;
    }

    if (workModel &&
        m_manager &&
        m_manager->getAlignMode()) {
        drawAlignmentStatus(r, paint, workModel, haveWorkTitle);
    }

    if (m_manager &&
        m_manager->shouldShowLayerNames()) {
        drawLayerNames(r, paint);
    }

    if (m_shiftPressed && m_clickedInRange &&
        (toolMode == ViewManager::NavigateMode || m_navigating)) {

        //!!! be nice if this looked a bit more in keeping with the
        //selection block
        
        paint.setPen(Qt::blue);
        //!!! shouldn't use clickPos -- needs to use a clicked frame
        paint.drawRect(m_clickPos.x(), m_clickPos.y(),
                       m_mousePos.x() - m_clickPos.x(),
                       m_mousePos.y() - m_clickPos.y());

    }

    if (toolMode == ViewManager::MeasureMode && topLayer) {
        bool showFocus = false;
        if (!m_manager || !m_manager->isPlaying()) showFocus = true;
        topLayer->paintMeasurementRects(this, paint, showFocus, m_identifyPoint);
    }
    
    if (selectionIsBeingEdited()) {
        drawEditingSelection(paint);
    }

    paint.end();
}

int
Pane::getVerticalScaleWidth() const
{
    if (m_scaleWidth > 0) return m_scaleWidth;
    else return 0;
}

void
Pane::drawVerticalScale(QRect r, Layer *topLayer, QPainter &paint)
{
    Layer *scaleLayer = 0;

    double min, max;
    bool log;
    QString unit;

    // If the top layer has no scale and reports no display
    // extents, but does report a unit, then the scale should be
    // drawn from any underlying layer with a scale and that unit.
    // If the top layer has no scale and no value extents at all,
    // then the scale should be drawn from any underlying layer
    // with a scale regardless of unit.

    int sw = topLayer->getVerticalScaleWidth
        (this, m_manager->shouldShowVerticalColourScale(), paint);

    if (sw > 0) {
        scaleLayer = topLayer;
        m_scaleWidth = sw;

    } else {

        bool hasDisplayExtents = topLayer->getDisplayExtents(min, max);
        bool hasValueExtents = topLayer->getValueExtents(min, max, log, unit);
            
        if (!hasDisplayExtents) {

            if (!hasValueExtents) {

                for (LayerList::iterator vi = m_layerStack.end();
                     vi != m_layerStack.begin(); ) {
                        
                    --vi;
                        
                    if ((*vi) == topLayer) continue;
                        
                    sw = (*vi)->getVerticalScaleWidth
                        (this, m_manager->shouldShowVerticalColourScale(), paint);
                        
                    if (sw > 0) {
                        scaleLayer = *vi;
                        m_scaleWidth = sw;
                        break;
                    }
                }
            } else if (unit != "") { // && hasValueExtents && !hasDisplayExtents

                QString requireUnit = unit;

                for (LayerList::iterator vi = m_layerStack.end();
                     vi != m_layerStack.begin(); ) {
                        
                    --vi;
                        
                    if ((*vi) == topLayer) continue;
                        
                    if ((*vi)->getDisplayExtents(min, max)) {
                            
                        // search no further than this: if the
                        // scale from this layer isn't suitable,
                        // we'll have to draw no scale (else we'd
                        // risk ending up with the wrong scale)
                            
                        if ((*vi)->getValueExtents(min, max, log, unit) &&
                            unit == requireUnit) {

                            sw = (*vi)->getVerticalScaleWidth
                                (this, m_manager->shouldShowVerticalColourScale(), paint);
                            if (sw > 0) {
                                scaleLayer = *vi;
                                m_scaleWidth = sw;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    if (!scaleLayer) m_scaleWidth = 0;
        
    if (m_scaleWidth > 0 && r.left() < m_scaleWidth) {

//      Profiler profiler("Pane::paintEvent - painting vertical scale", true);

//      cerr << "Pane::paintEvent: calling paint.save() in vertical scale block" << endl;
        paint.save();
            
        paint.setPen(getForeground());
        paint.setBrush(getBackground());
        paint.drawRect(0, -1, m_scaleWidth, height()+1);
        
        paint.setBrush(Qt::NoBrush);
        scaleLayer->paintVerticalScale
            (this, m_manager->shouldShowVerticalColourScale(),
             paint, QRect(0, 0, m_scaleWidth, height()));
        
        paint.restore();
    }
}
            
void
Pane::drawFeatureDescription(Layer *topLayer, QPainter &paint)
{
    QPoint pos = m_identifyPoint;
    QString desc = topLayer->getFeatureDescription(this, pos);
        
    if (desc != "") {
        
        paint.save();
        
        int tabStop =
            paint.fontMetrics().width(tr("Some lengthy prefix:"));
        
        QRect boundingRect = 
            paint.fontMetrics().boundingRect
            (rect(),
             Qt::AlignRight | Qt::AlignTop | Qt::TextExpandTabs,
             desc, tabStop);
        
        if (hasLightBackground()) {
            paint.setPen(Qt::NoPen);
            paint.setBrush(QColor(250, 250, 250, 200));
        } else {
            paint.setPen(Qt::NoPen);
            paint.setBrush(QColor(50, 50, 50, 200));
        }
        
        int extra = paint.fontMetrics().descent();
        paint.drawRect(width() - boundingRect.width() - 10 - extra,
                       10 - extra,
                       boundingRect.width() + 2 * extra,
                       boundingRect.height() + extra);
        
        if (hasLightBackground()) {
            paint.setPen(QColor(150, 20, 0));
        } else {
            paint.setPen(QColor(255, 150, 100));
        }
        
        QTextOption option;
        option.setWrapMode(QTextOption::NoWrap);
        option.setAlignment(Qt::AlignRight | Qt::AlignTop);
        option.setTabStop(tabStop);
        paint.drawText(QRectF(width() - boundingRect.width() - 10, 10,
                              boundingRect.width(),
                              boundingRect.height()),
                       desc,
                       option);
        
        paint.restore();
    }
}

void
Pane::drawCentreLine(sv_samplerate_t sampleRate, QPainter &paint, bool omitLine)
{
    int fontHeight = paint.fontMetrics().height();
    int fontAscent = paint.fontMetrics().ascent();

    QColor c = QColor(0, 0, 0);
    if (!hasLightBackground()) {
        c = QColor(240, 240, 240);
    }

    paint.setPen(c);
    int x = width() / 2;

    if (!omitLine) {
        paint.drawLine(x, 0, x, height() - 1);
        paint.drawLine(x-1, 1, x+1, 1);
        paint.drawLine(x-2, 0, x+2, 0);
        paint.drawLine(x-1, height() - 2, x+1, height() - 2);
        paint.drawLine(x-2, height() - 1, x+2, height() - 1);
    }
    
    paint.setPen(QColor(50, 50, 50));
    
    int y = height() - fontHeight + fontAscent - 6;
    
    LayerList::iterator vi = m_layerStack.end();
    
    if (vi != m_layerStack.begin()) {
        
        switch ((*--vi)->getPreferredFrameCountPosition()) {
            
        case Layer::PositionTop:
            y = fontAscent + 6;
            break;
            
        case Layer::PositionMiddle:
            y = (height() - fontHeight) / 2
                + fontAscent;
            break;
            
        case Layer::PositionBottom:
            // y already set correctly
            break;
        }
    }
    
    if (m_manager && m_manager->shouldShowFrameCount()) {
        
        if (sampleRate) {

            QString text(QString::fromStdString
                         (RealTime::frame2RealTime
                          (m_centreFrame, sampleRate)
                          .toText(true)));
            
            int tw = paint.fontMetrics().width(text);
            int x = width()/2 - 4 - tw;
            
            drawVisibleText(paint, x, y, text, OutlinedText);
        }
        
        QString text = QString("%1").arg(m_centreFrame);
        
        int x = width()/2 + 4;
        
        drawVisibleText(paint, x, y, text, OutlinedText);
    }
}

void
Pane::drawModelTimeExtents(QRect r, QPainter &paint, const Model *model)
{
    int x0 = getXForFrame(model->getStartFrame());
    int x1 = getXForFrame(model->getEndFrame());

    paint.save();

    QBrush brush;

    if (hasLightBackground()) {
        brush = QBrush(QColor("#f8f8f8"));
        paint.setPen(Qt::black);
    } else {
        brush = QBrush(QColor("#101010"));
        paint.setPen(Qt::white);
    }

    if (x0 > r.x()) {
        paint.fillRect(0, 0, x0, height(), brush);
        paint.drawLine(x0, 0, x0, height());
    }

    if (x1 < r.x() + r.width()) {
        paint.fillRect(x1, 0, width() - x1, height(), brush);
        paint.drawLine(x1, 0, x1, height());
    }

    paint.restore();
}

void
Pane::drawAlignmentStatus(QRect r, QPainter &paint, const Model *model,
                          bool down)
{
    const Model *reference = model->getAlignmentReference();
/*
    if (!reference) {
        cerr << "Pane[" << this << "]::drawAlignmentStatus: No reference" << endl;
    } else if (reference == model) {
        cerr << "Pane[" << this << "]::drawAlignmentStatus: This is the reference model" << endl;
    } else {
        cerr << "Pane[" << this << "]::drawAlignmentStatus: This is not the reference" << endl;
    }
*/
    QString text;
    int completion = 100;

    if (reference == model) {
        text = tr("Reference");
    } else if (!reference) {
        text = tr("Unaligned");
    } else {
        completion = model->getAlignmentCompletion();
        if (completion == 0) {
            text = tr("Unaligned");
        } else if (completion < 100) {
            text = tr("Aligning: %1%").arg(completion);
        } else {
            text = tr("Aligned");
        }
    }

    paint.save();
    QFont font(paint.font());
    font.setBold(true);
    paint.setFont(font);
    if (completion < 100) paint.setBrush(Qt::red);

    int y = 5;
    if (down) y += paint.fontMetrics().height();
    int w = paint.fontMetrics().width(text);
    int h = paint.fontMetrics().height();
    if (r.top() > h + y || r.left() > w + m_scaleWidth + 5) {
        paint.restore();
        return;
    }
    
    drawVisibleText(paint, m_scaleWidth + 5,
                    paint.fontMetrics().ascent() + y, text, OutlinedText);

    paint.restore();
}

void
Pane::modelAlignmentCompletionChanged()
{
    View::modelAlignmentCompletionChanged();
    update(QRect(0, 0, 300, 100));
}

void
Pane::drawWorkTitle(QRect r, QPainter &paint, const Model *model)
{
    QString title = model->getTitle();
    QString maker = model->getMaker();
//cerr << "Pane::drawWorkTitle: title=\"" << title//<< "\", maker=\"" << maker << "\"" << endl;
    if (title == "") return;

    QString text = title;
    if (maker != "") {
        text = tr("%1 - %2").arg(title).arg(maker);
    }
    
    paint.save();
    QFont font(paint.font());
    font.setItalic(true);
    paint.setFont(font);

    int y = 5;
    int w = paint.fontMetrics().width(text);
    int h = paint.fontMetrics().height();
    if (r.top() > h + y || r.left() > w + m_scaleWidth + 5) {
        paint.restore();
        return;
    }
    
    drawVisibleText(paint, m_scaleWidth + 5,
                    paint.fontMetrics().ascent() + y, text, OutlinedText);

    paint.restore();
}

void
Pane::drawLayerNames(QRect r, QPainter &paint)
{
    int fontHeight = paint.fontMetrics().height();
    int fontAscent = paint.fontMetrics().ascent();

    int lly = height() - 6;
    if (m_manager->getZoomWheelsEnabled()) {
        lly -= 20;
    }

    if (r.y() + r.height() < lly - int(m_layerStack.size()) * fontHeight) {
        return;
    }

    QStringList texts;
    std::vector<QPixmap> pixmaps;
    for (LayerList::iterator i = m_layerStack.begin(); i != m_layerStack.end(); ++i) {
        texts.push_back((*i)->getLayerPresentationName());
//        cerr << "Pane " << this << ": Layer presentation name for " << *i << ": "
//                  << texts[texts.size()-1] << endl;
        pixmaps.push_back((*i)->getLayerPresentationPixmap
                          (QSize(fontAscent, fontAscent)));
    }

    int maxTextWidth = width() / 3;
    texts = TextAbbrev::abbreviate(texts, paint.fontMetrics(), maxTextWidth);

    int llx = width() - maxTextWidth - 5;
    if (m_manager->getZoomWheelsEnabled()) {
        llx -= 36;
    }
    
    if (r.x() + r.width() >= llx - fontAscent - 3) {
    
        for (int i = 0; i < texts.size(); ++i) {

//            cerr << "Pane "<< this << ": text " << i << ": " << texts[i] << endl;
            
            if (i + 1 == texts.size()) {
                paint.setPen(getForeground());
            }
            
            drawVisibleText(paint, llx,
                            lly - fontHeight + fontAscent,
                            texts[i], OutlinedText);

            if (!pixmaps[i].isNull()) {
                paint.drawPixmap(llx - fontAscent - 3,
                                 lly - fontHeight + (fontHeight-fontAscent)/2,
                                 pixmaps[i]);
            }
            
            lly -= fontHeight;
        }
    }
}

void
Pane::drawEditingSelection(QPainter &paint)
{
    int offset = m_mousePos.x() - m_clickPos.x();

    sv_frame_t origStart = m_editingSelection.getStartFrame();

    int p0 = getXForFrame(origStart) + offset;
    int p1 = getXForFrame(m_editingSelection.getEndFrame()) + offset;

    if (m_editingSelectionEdge < 0) {
        p1 = getXForFrame(m_editingSelection.getEndFrame());
    } else if (m_editingSelectionEdge > 0) {
        p0 = getXForFrame(m_editingSelection.getStartFrame());
    }
    
    sv_frame_t newStart = getFrameForX(p0);
    sv_frame_t newEnd = getFrameForX(p1);
    
    paint.save();
    paint.setPen(QPen(getForeground(), 2));

    int fontHeight = paint.fontMetrics().height();
    int fontAscent = paint.fontMetrics().ascent();
    sv_samplerate_t sampleRate = getModelsSampleRate();
    QString startText, endText, offsetText;
    startText = QString("%1").arg(newStart);
    endText = QString("%1").arg(newEnd);
    offsetText = QString("%1").arg(newStart - origStart);
    if (newStart >= origStart) {
        offsetText = tr("+%1").arg(offsetText);
    }
    if (sampleRate) {
        startText = QString("%1 / %2")
            .arg(QString::fromStdString
                 (RealTime::frame2RealTime(newStart, sampleRate).toText()))
            .arg(startText);
        endText = QString("%1 / %2")
            .arg(QString::fromStdString
                 (RealTime::frame2RealTime(newEnd, sampleRate).toText()))
            .arg(endText);
        offsetText = QString("%1 / %2")
            .arg(QString::fromStdString
                 (RealTime::frame2RealTime(newStart - origStart, sampleRate).toText()))
            .arg(offsetText);
        if (newStart >= origStart) {
            offsetText = tr("+%1").arg(offsetText);
        }
    }
    drawVisibleText(paint, p0 + 2, fontAscent + fontHeight + 4, startText, OutlinedText);
    drawVisibleText(paint, p1 + 2, fontAscent + fontHeight + 4, endText, OutlinedText);
    drawVisibleText(paint, p0 + 2, fontAscent + fontHeight*2 + 4, offsetText, OutlinedText);
    drawVisibleText(paint, p1 + 2, fontAscent + fontHeight*2 + 4, offsetText, OutlinedText);
    
    //!!! duplicating display policy with View::drawSelections
    
    if (m_editingSelectionEdge < 0) {
        paint.drawLine(p0, 1, p1, 1);
        paint.drawLine(p0, 0, p0, height());
        paint.drawLine(p0, height() - 1, p1, height() - 1);
    } else if (m_editingSelectionEdge > 0) {
        paint.drawLine(p0, 1, p1, 1);
        paint.drawLine(p1, 0, p1, height());
        paint.drawLine(p0, height() - 1, p1, height() - 1);
    } else {
        paint.setBrush(Qt::NoBrush);
        paint.drawRect(p0, 1, p1 - p0, height() - 2);
    }
    paint.restore();
}

void
Pane::drawDurationAndRate(QRect r, const Model *waveformModel,
                          sv_samplerate_t sampleRate, QPainter &paint)
{
    int fontHeight = paint.fontMetrics().height();
    int fontAscent = paint.fontMetrics().ascent();

    if (r.y() + r.height() < height() - fontHeight - 6) return;

    sv_samplerate_t modelRate = waveformModel->getSampleRate();
    sv_samplerate_t nativeRate = waveformModel->getNativeRate();
    sv_samplerate_t playbackRate = m_manager->getPlaybackSampleRate();
    sv_samplerate_t outputRate = m_manager->getOutputSampleRate();
        
    QString srNote = "";

    // Show (R) for waveform models that have been resampled or will
    // be resampled on playback, and (X) for waveform models that will
    // be played at the wrong rate because their rate differs from the
    // current playback rate (which is not necessarily that of the
    // main model).

    if (playbackRate != 0) {
        if (modelRate == playbackRate) {
            if (modelRate != outputRate || modelRate != nativeRate) {
                srNote = " " + tr("(R)");
            }
        } else {
            srNote = " " + tr("(X)");
        }
    }

    QString desc = tr("%1 / %2Hz%3")
        .arg(RealTime::frame2RealTime(waveformModel->getEndFrame(),
                                      sampleRate)
             .toText(false).c_str())
        .arg(nativeRate)
        .arg(srNote);

    int x = m_scaleWidth + 5;
    int pbw = getProgressBarWidth();
    if (x < pbw + 5) x = pbw + 5;

    if (r.x() < x + paint.fontMetrics().width(desc)) {
        drawVisibleText(paint, x,
                        height() - fontHeight + fontAscent - 6,
                        desc, OutlinedText);
    }
}

bool
Pane::render(QPainter &paint, int xorigin, sv_frame_t f0, sv_frame_t f1)
{
    if (!View::render(paint, xorigin + m_scaleWidth, f0, f1)) {
        return false;
    }

    if (m_scaleWidth > 0) {

        Layer *layer = getTopLayer();

        if (layer) {
            
            paint.save();
            
            paint.setPen(getForeground());
            paint.setBrush(getBackground());
            paint.drawRect(xorigin, -1, m_scaleWidth, height()+1);
            
            paint.setBrush(Qt::NoBrush);
            layer->paintVerticalScale
                (this, m_manager->shouldShowVerticalColourScale(),
                 paint, QRect(xorigin, 0, m_scaleWidth, height()));
            
            paint.restore();
        }
    }

    return true;
}

QImage *
Pane::toNewImage(sv_frame_t f0, sv_frame_t f1)
{
    int x0 = int(f0 / getZoomLevel());
    int x1 = int(f1 / getZoomLevel());

    QImage *image = new QImage(x1 - x0 + m_scaleWidth,
                               height(), QImage::Format_RGB32);

    int formerScaleWidth = m_scaleWidth;
            
    if (m_manager && m_manager->shouldShowVerticalScale()) {
        Layer *layer = getTopLayer();
        if (layer) {
            QPainter paint(image);
            m_scaleWidth = layer->getVerticalScaleWidth
                (this, m_manager->shouldShowVerticalColourScale(), paint);
        }
    } else {
        m_scaleWidth = 0;
    }

    if (m_scaleWidth != formerScaleWidth) {
        delete image;
        image = new QImage(x1 - x0 + m_scaleWidth,
                           height(), QImage::Format_RGB32);
    }        

    QPainter *paint = new QPainter(image);
    if (!render(*paint, 0, f0, f1)) {
        delete paint;
        delete image;
        return 0;
    } else {
        delete paint;
        return image;
    }
}

QSize
Pane::getImageSize(sv_frame_t f0, sv_frame_t f1)
{
    QSize s = View::getImageSize(f0, f1);
    QImage *image = new QImage(100, 100, QImage::Format_RGB32);
    QPainter paint(image);

    int sw = 0;
    if (m_manager && m_manager->shouldShowVerticalScale()) {
        Layer *layer = getTopLayer();
        if (layer) {
            sw = layer->getVerticalScaleWidth
                (this, m_manager->shouldShowVerticalColourScale(), paint);
        }
    }
    
    return QSize(sw + s.width(), s.height());
}

sv_frame_t
Pane::getFirstVisibleFrame() const
{
    sv_frame_t f0 = getFrameForX(m_scaleWidth);
    sv_frame_t f = View::getFirstVisibleFrame();
    if (f0 < 0 || f0 < f) return f;
    return f0;
}

Selection
Pane::getSelectionAt(int x, bool &closeToLeftEdge, bool &closeToRightEdge) const
{
    closeToLeftEdge = closeToRightEdge = false;

    if (!m_manager) return Selection();

    sv_frame_t testFrame = getFrameForX(x - 5);
    if (testFrame < 0) {
        testFrame = getFrameForX(x);
        if (testFrame < 0) return Selection();
    }

    Selection selection = m_manager->getContainingSelection(testFrame, true);
    if (selection.isEmpty()) return selection;

    int lx = getXForFrame(selection.getStartFrame());
    int rx = getXForFrame(selection.getEndFrame());
    
    int fuzz = 2;
    if (x < lx - fuzz || x > rx + fuzz) return Selection();

    int width = rx - lx;
    fuzz = 3;
    if (width < 12) fuzz = width / 4;
    if (fuzz < 1) fuzz = 1;

    if (x < lx + fuzz) closeToLeftEdge = true;
    if (x > rx - fuzz) closeToRightEdge = true;

    return selection;
}

bool
Pane::canTopLayerMoveVertical()
{
    double vmin, vmax, dmin, dmax;
    if (!getTopLayerDisplayExtents(vmin, vmax, dmin, dmax)) return false;
    if (dmin <= vmin && dmax >= vmax) return false;
    return true;
}

bool
Pane::getTopLayerDisplayExtents(double &vmin, double &vmax,
                                double &dmin, double &dmax,
                                QString *unit) 
{
    Layer *layer = getTopLayer();
    if (!layer) return false;
    bool vlog;
    QString vunit;
    bool rv = (layer->getValueExtents(vmin, vmax, vlog, vunit) &&
               layer->getDisplayExtents(dmin, dmax));
    if (unit) *unit = vunit;
    return rv;
}

bool
Pane::setTopLayerDisplayExtents(double dmin, double dmax)
{
    Layer *layer = getTopLayer();
    if (!layer) return false;
    return layer->setDisplayExtents(dmin, dmax);
}

void
Pane::registerShortcuts(KeyReference &kr)
{
    kr.setCategory(tr("Zoom"));
    kr.registerAlternativeShortcut(tr("Zoom In"), tr("Wheel Up"));
    kr.registerAlternativeShortcut(tr("Zoom Out"), tr("Wheel Down"));

    kr.setCategory(tr("General Pane Mouse Actions"));
    
    kr.registerShortcut(tr("Zoom"), tr("Wheel"),
                        tr("Zoom in or out in time axis"));
    kr.registerShortcut(tr("Scroll"), tr("Ctrl+Wheel"),
                        tr("Scroll rapidly left or right in time axis"));
    kr.registerShortcut(tr("Zoom Vertically"), tr("Shift+Wheel"), 
                        tr("Zoom in or out in the vertical axis"));
    kr.registerShortcut(tr("Scroll Vertically"), tr("Alt+Wheel"), 
                        tr("Scroll up or down in the vertical axis"));
    kr.registerShortcut(tr("Navigate"), tr("Middle"), 
                        tr("Click middle button and drag to navigate with any tool"));
    kr.registerShortcut(tr("Relocate"), tr("Double-Click Middle"), 
                        tr("Double-click middle button to relocate with any tool"));
    kr.registerShortcut(tr("Menu"), tr("Right"),
                        tr("Show pane context menu"));
}

Layer *
Pane::getTopFlexiNoteLayer()
{
    for (int i = int(m_layerStack.size()) - 1; i >= 0; --i) {
        if (LayerFactory::getInstance().getLayerType(m_layerStack[i]) == LayerFactory::Type("FlexiNotes")) {
            return m_layerStack[i];
        }
    }
    return 0;
}

void
Pane::mousePressEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::RightButton) {
        emit contextHelpChanged("");
        emit rightButtonMenuRequested(mapToGlobal(e->pos()));
        return;
    }

//    cerr << "mousePressEvent" << endl;

    m_clickPos = e->pos();
    m_mousePos = m_clickPos;
    m_clickedInRange = true;
    m_editingSelection = Selection();
    m_editingSelectionEdge = 0;
    m_shiftPressed = (e->modifiers() & Qt::ShiftModifier);
    m_ctrlPressed = (e->modifiers() & Qt::ControlModifier);
    m_altPressed = (e->modifiers() & Qt::AltModifier);
    m_dragMode = UnresolvedDrag;

    ViewManager::ToolMode mode = ViewManager::NavigateMode;
    if (m_manager) mode = m_manager->getToolModeFor(this);

    m_navigating = false;
    m_resizing = false;
    m_editing = false;
    m_releasing = false;

    if (mode == ViewManager::NavigateMode ||
        (e->buttons() & Qt::MidButton) ||
        (mode == ViewManager::MeasureMode &&
         (e->buttons() & Qt::LeftButton) && m_shiftPressed)) {

        if (mode != ViewManager::NavigateMode) {
            setCursor(Qt::PointingHandCursor);
        }

        m_navigating = true;
        m_dragCentreFrame = m_centreFrame;
        m_dragStartMinValue = 0;
        
        double vmin, vmax, dmin, dmax;
        if (getTopLayerDisplayExtents(vmin, vmax, dmin, dmax)) {
            m_dragStartMinValue = dmin;
        }

        if (m_followPlay == PlaybackScrollPage) {
            // Schedule a play-head move to the mouse frame
            // location. This will happen only if nothing else of
            // interest happens (double-click, drag) before the
            // timeout.
            schedulePlaybackFrameMove(getFrameForX(e->x()));
        }

    } else if (mode == ViewManager::SelectMode) {

        if (!hasTopLayerTimeXAxis()) return;

        bool closeToLeft = false, closeToRight = false;
        Selection selection = getSelectionAt(e->x(), closeToLeft, closeToRight);

        if ((closeToLeft || closeToRight) && !(closeToLeft && closeToRight)) {

            m_manager->removeSelection(selection);

            if (closeToLeft) {
                m_selectionStartFrame = selection.getEndFrame();
            } else {
                m_selectionStartFrame = selection.getStartFrame();
            }
            
            m_manager->setInProgressSelection(selection, false);
            m_resizing = true;
            
        } else {
            
            sv_frame_t mouseFrame = getFrameForX(e->x());
            int resolution = 1;
            sv_frame_t snapFrame = mouseFrame;
    
            Layer *layer = getInteractionLayer();
            if (layer && !m_shiftPressed &&
                !qobject_cast<TimeRulerLayer *>(layer)) { // don't snap to secs
                layer->snapToFeatureFrame(this, snapFrame,
                                          resolution, Layer::SnapLeft);
            }
        
            if (snapFrame < 0) snapFrame = 0;
            m_selectionStartFrame = snapFrame;
            if (m_manager) {
                m_manager->setInProgressSelection
                    (Selection(alignToReference(snapFrame),
                               alignToReference(snapFrame + resolution)),
                     !m_ctrlPressed);
            }

            m_resizing = false;

            if (m_followPlay == PlaybackScrollPage) {
                // Schedule a play-head move to the mouse frame
                // location. This will happen only if nothing else of
                // interest happens (double-click, drag) before the
                // timeout.
                schedulePlaybackFrameMove(mouseFrame);
            }
        }

        update();

    } else if (mode == ViewManager::DrawMode) {

        Layer *layer = getInteractionLayer();
        if (layer && layer->isLayerEditable()) {
            layer->drawStart(this, e);
        }

    } else if (mode == ViewManager::EraseMode) {

        Layer *layer = getInteractionLayer();
        if (layer && layer->isLayerEditable()) {
            layer->eraseStart(this, e);
        }

        // GF: handle mouse press for NoteEditMode 
    } else if (mode == ViewManager::NoteEditMode) {

        std::cerr << "mouse pressed in note edit mode" << std::endl;
        Layer *layer = getTopFlexiNoteLayer();
        if (layer) {
            layer->splitStart(this, e); 
        }

    } else if (mode == ViewManager::EditMode) {

        // Do nothing here -- we'll do it in mouseMoveEvent when the
        // drag threshold has been passed

    } else if (mode == ViewManager::MeasureMode) {

        Layer *layer = getTopLayer();
        if (layer) layer->measureStart(this, e);
        update();
    }

    emit paneInteractedWith();
}

void
Pane::schedulePlaybackFrameMove(sv_frame_t frame)
{
    m_playbackFrameMoveTo = frame;
    m_playbackFrameMoveScheduled = true;
    QTimer::singleShot(QApplication::doubleClickInterval() + 10, this,
                       SLOT(playbackScheduleTimerElapsed()));
}

void
Pane::playbackScheduleTimerElapsed()
{
    if (m_playbackFrameMoveScheduled) {
        m_manager->setPlaybackFrame(m_playbackFrameMoveTo);
        m_playbackFrameMoveScheduled = false;
    }
}

void
Pane::mouseReleaseEvent(QMouseEvent *e)
{
    if (e && (e->buttons() & Qt::RightButton)) {
        return;
    }

//    cerr << "mouseReleaseEvent" << endl;

    ViewManager::ToolMode mode = ViewManager::NavigateMode;
    if (m_manager) mode = m_manager->getToolModeFor(this);

    m_releasing = true;

    if (m_clickedInRange) {
        mouseMoveEvent(e);
    }

    sv_frame_t mouseFrame = e ? getFrameForX(e->x()) : 0;
    if (mouseFrame < 0) mouseFrame = 0;

    if (m_navigating || mode == ViewManager::NavigateMode) {

        m_navigating = false;

        if (mode != ViewManager::NavigateMode) {
            // restore cursor
            toolModeChanged();
        }

        if (m_shiftPressed) {

            int x0 = std::min(m_clickPos.x(), m_mousePos.x());
            int x1 = std::max(m_clickPos.x(), m_mousePos.x());

            int y0 = std::min(m_clickPos.y(), m_mousePos.y());
            int y1 = std::max(m_clickPos.y(), m_mousePos.y());

            emit regionOutlined(QRect(x0, y0, x1 - x0, y1 - y0));
        }

    } else if (mode == ViewManager::SelectMode) {

        if (!hasTopLayerTimeXAxis()) {
            m_releasing = false;
            return;
        }

        if (m_manager && m_manager->haveInProgressSelection()) {

            //cerr << "JTEST: release with selection" << endl;
            bool exclusive;
            Selection selection = m_manager->getInProgressSelection(exclusive);
        
            if (selection.getEndFrame() < selection.getStartFrame() + 2) {
                selection = Selection();
            }
        
            m_manager->clearInProgressSelection();
        
            if (exclusive) {
                m_manager->setSelection(selection);
            } else {
                m_manager->addSelection(selection);
            }
        }
    
        update();

    } else if (mode == ViewManager::DrawMode) {

        Layer *layer = getInteractionLayer();
        if (layer && layer->isLayerEditable()) {
            layer->drawEnd(this, e);
            update();
        }

    } else if (mode == ViewManager::EraseMode) {

        Layer *layer = getInteractionLayer();
        if (layer && layer->isLayerEditable()) {
            layer->eraseEnd(this, e);
            update();
        }

    } else if (mode == ViewManager::NoteEditMode) {
    
        //GF: handle mouse release for NoteEditMode (note: works but will need to re-think this a bit later)
        Layer *layer = getTopFlexiNoteLayer();

        if (layer) {
            layer->splitEnd(this, e);
            update();

            if (m_editing) {
                if (!editSelectionEnd(e)) {
                    layer->editEnd(this, e);
                    update();
                }
            }
        } 

    } else if (mode == ViewManager::EditMode) {
        
        if (m_editing) {
            if (!editSelectionEnd(e)) {
                Layer *layer = getInteractionLayer();
                if (layer && layer->isLayerEditable()) {
                    layer->editEnd(this, e);
                    update();
                }
            }
        } 

    } else if (mode == ViewManager::MeasureMode) {

        Layer *layer = getTopLayer();
        if (layer) layer->measureEnd(this, e);
        if (m_measureCursor1) setCursor(*m_measureCursor1);
        update();
    }

    m_clickedInRange = false;
    m_releasing = false;

    emit paneInteractedWith();
}

void
Pane::mouseMoveEvent(QMouseEvent *e)
{
    if (!e || (e->buttons() & Qt::RightButton)) {
        return;
    }

//    cerr << "mouseMoveEvent" << endl;

    QPoint pos = e->pos();
    updateContextHelp(&pos);

    if (m_navigating && m_clickedInRange && !m_releasing) {

        // if no buttons pressed, and not called from
        // mouseReleaseEvent, we want to reset clicked-ness (to avoid
        // annoying continual drags when we moved the mouse outside
        // the window after pressing button first time).

        if (!(e->buttons() & Qt::LeftButton) &&
            !(e->buttons() & Qt::MidButton)) {
            m_clickedInRange = false;
            return;
        }
    }

    ViewManager::ToolMode mode = ViewManager::NavigateMode;
    if (m_manager) mode = m_manager->getToolModeFor(this);

    QPoint prevPoint = m_identifyPoint;
    m_identifyPoint = e->pos();

    if (!m_clickedInRange) {
    
        // GF: handle mouse move for context sensitive cursor switching in NoteEditMode.
        // GF: Propagate the event to FlexiNoteLayer. I somehow feel it's best handeled there rather than here, but perhaps not if this will be needed elsewhere too.
        if (mode == ViewManager::NoteEditMode) {
            FlexiNoteLayer *layer = qobject_cast<FlexiNoteLayer *>(getTopFlexiNoteLayer());
            if (layer) {
                layer->mouseMoveEvent(this, e); //!!! ew
                update();
                // return;
            }
        }   
    
        if (mode == ViewManager::SelectMode && hasTopLayerTimeXAxis()) {
            bool closeToLeft = false, closeToRight = false;
            getSelectionAt(e->x(), closeToLeft, closeToRight);
            if ((closeToLeft || closeToRight) && !(closeToLeft && closeToRight)) {
                setCursor(Qt::SizeHorCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }

        if (m_manager && !m_manager->isPlaying()) {

            bool updating = false;

            if (getInteractionLayer() &&
                m_manager->shouldIlluminateLocalFeatures()) {

                bool previouslyIdentifying = m_identifyFeatures;
                m_identifyFeatures = true;
                
                if (m_identifyFeatures != previouslyIdentifying ||
                    m_identifyPoint != prevPoint) {
                    update();
                    updating = true;
                }
            }

            if (!updating && mode == ViewManager::MeasureMode) {

                Layer *layer = getTopLayer();
                if (layer && layer->nearestMeasurementRectChanged
                    (this, prevPoint, m_identifyPoint)) {
                    update();
                }
            }
        }

        return;
    }

    if (m_navigating || mode == ViewManager::NavigateMode) {

        if (m_shiftPressed) {

            m_mousePos = e->pos();
            update();

        } else {

            dragTopLayer(e);
        }

    } else if (mode == ViewManager::SelectMode) {

        if (!hasTopLayerTimeXAxis()) return;

        dragExtendSelection(e);

    } else if (mode == ViewManager::DrawMode) {

        Layer *layer = getInteractionLayer();
        if (layer && layer->isLayerEditable()) {
            layer->drawDrag(this, e);
        }

    } else if (mode == ViewManager::EraseMode) {

        Layer *layer = getInteractionLayer();
        if (layer && layer->isLayerEditable()) {
            layer->eraseDrag(this, e);
        }

        // GF: handling NoteEditMode dragging and boundary actions for mouseMoveEvent
    } else if (mode == ViewManager::NoteEditMode) {

        bool resist = true;

        if ((e->modifiers() & Qt::ShiftModifier)) {
            m_shiftPressed = true;
        }

        if (m_shiftPressed) resist = false;

        m_dragMode = updateDragMode
            (m_dragMode,
             m_clickPos,
             e->pos(),
             true,    // can move horiz
             true,    // can move vert
             resist,  // resist horiz
             resist); // resist vert

        if (!m_editing) {

            if (m_dragMode != UnresolvedDrag) {

                m_editing = true;

                QMouseEvent clickEvent(QEvent::MouseButtonPress,
                                       m_clickPos,
                                       Qt::NoButton,
                                       e->buttons(),
                                       e->modifiers());

                if (!editSelectionStart(&clickEvent)) {
                    Layer *layer = getTopFlexiNoteLayer();
                    if (layer) {
                        std::cerr << "calling edit start" << std::endl;
                        layer->editStart(this, &clickEvent);
                    }
                }
            }

        } else {

            if (!editSelectionDrag(e)) {

                Layer *layer = getTopFlexiNoteLayer();

                if (layer) {

                    int x = e->x();
                    int y = e->y();
                    if (m_dragMode == VerticalDrag) x = m_clickPos.x();
                    else if (m_dragMode == HorizontalDrag) y = m_clickPos.y();

                    QMouseEvent moveEvent(QEvent::MouseMove,
                                          QPoint(x, y),
                                          Qt::NoButton,
                                          e->buttons(),
                                          e->modifiers());
                    std::cerr << "calling editDrag" << std::endl;
                    layer->editDrag(this, &moveEvent);
                }
            }
        }

    } else if (mode == ViewManager::EditMode) {

        bool resist = true;

        if ((e->modifiers() & Qt::ShiftModifier)) {
            m_shiftPressed = true;
            // ... but don't set it false if shift has been
            // released -- we want the state when we started
            // dragging to be used most of the time
        }

        if (m_shiftPressed) resist = false;

        m_dragMode = updateDragMode
            (m_dragMode,
             m_clickPos,
             e->pos(),
             true,    // can move horiz
             true,    // can move vert
             resist,  // resist horiz
             resist); // resist vert

        if (!m_editing) {

            if (m_dragMode != UnresolvedDrag) {

                m_editing = true;

                QMouseEvent clickEvent(QEvent::MouseButtonPress,
                                       m_clickPos,
                                       Qt::NoButton,
                                       e->buttons(),
                                       e->modifiers());

                if (!editSelectionStart(&clickEvent)) {
                    Layer *layer = getInteractionLayer();
                    if (layer && layer->isLayerEditable()) {
                        layer->editStart(this, &clickEvent);
                    }
                }
            }

        } else {

            if (!editSelectionDrag(e)) {

                Layer *layer = getInteractionLayer();

                if (layer && layer->isLayerEditable()) {

                    int x = e->x();
                    int y = e->y();
                    if (m_dragMode == VerticalDrag) x = m_clickPos.x();
                    else if (m_dragMode == HorizontalDrag) y = m_clickPos.y();

                    QMouseEvent moveEvent(QEvent::MouseMove,
                                          QPoint(x, y),
                                          Qt::NoButton,
                                          e->buttons(),
                                          e->modifiers());
                                              
                    layer->editDrag(this, &moveEvent);
                }
            }
        }

    } else if (mode == ViewManager::MeasureMode) {

        if (m_measureCursor2) setCursor(*m_measureCursor2);

        Layer *layer = getTopLayer();
        if (layer) {
            layer->measureDrag(this, e);
            if (layer->hasTimeXAxis()) edgeScrollMaybe(e->x());
        }

        update();
    }
    
    if (m_dragMode != UnresolvedDrag) {
        m_playbackFrameMoveScheduled = false;
    }
}

void
Pane::zoomToRegion(QRect r)
{
    int x0 = r.x();
    int y0 = r.y();
    int x1 = r.x() + r.width();
    int y1 = r.y() + r.height();

    int w = x1 - x0;
        
    sv_frame_t newStartFrame = getFrameForX(x0);
        
    sv_frame_t visibleFrames = getEndFrame() - getStartFrame();
    if (newStartFrame <= -visibleFrames) {
        newStartFrame  = -visibleFrames + 1;
    }
        
    if (newStartFrame >= getModelsEndFrame()) {
        newStartFrame  = getModelsEndFrame() - 1;
    }
        
    double ratio = double(w) / double(width());
//	cerr << "ratio: " << ratio << endl;
    int newZoomLevel = (int)nearbyint(m_zoomLevel * ratio);
    if (newZoomLevel < 1) newZoomLevel = 1;

//	cerr << "start: " << m_startFrame << ", level " << m_zoomLevel << endl;
    setZoomLevel(getZoomConstraintBlockSize(newZoomLevel));
    setStartFrame(newStartFrame);

    QString unit;
    double min, max;
    bool log;
    Layer *layer = 0;
    for (LayerList::const_iterator i = m_layerStack.begin();
         i != m_layerStack.end(); ++i) { 
        if ((*i)->getValueExtents(min, max, log, unit) &&
            (*i)->getDisplayExtents(min, max)) {
            layer = *i;
            break;
        }
    }
            
    if (layer) {
        if (log) {
            min = (min < 0.0) ? -log10(-min) : (min == 0.0) ? 0.0 : log10(min);
            max = (max < 0.0) ? -log10(-max) : (max == 0.0) ? 0.0 : log10(max);
        }
        double rmin = min + ((max - min) * (height() - y1)) / height();
        double rmax = min + ((max - min) * (height() - y0)) / height();
        cerr << "min: " << min << ", max: " << max << ", y0: " << y0 << ", y1: " << y1 << ", h: " << height() << ", rmin: " << rmin << ", rmax: " << rmax << endl;
        if (log) {
            rmin = pow(10, rmin);
            rmax = pow(10, rmax);
        }
        cerr << "finally: rmin: " << rmin << ", rmax: " << rmax << " " << unit << endl;

        layer->setDisplayExtents(rmin, rmax);
        updateVerticalPanner();
    }
}

void
Pane::dragTopLayer(QMouseEvent *e)
{
    // We need to avoid making it too easy to drag both
    // horizontally and vertically, in the case where the
    // mouse is moved "mostly" in horizontal or vertical axis
    // with only a small variation in the other axis.  This is
    // particularly important during playback (when we want to
    // avoid small horizontal motions) or in slow refresh
    // layers like spectrogram (when we want to avoid small
    // vertical motions).
    // 
    // To this end we have horizontal and vertical thresholds
    // and a series of states: unresolved, horizontally or
    // vertically constrained, free.
    //
    // When the mouse first moves, we're unresolved: we
    // restrict ourselves to whichever direction seems safest,
    // until the mouse has passed a small threshold distance
    // from the click point.  Then we lock in to one of the
    // constrained modes, based on which axis that distance
    // was measured in first.  Finally, if it turns out we've
    // also moved more than a certain larger distance in the
    // other direction as well, we may switch into free mode.
    // 
    // If the top layer is incapable of being dragged
    // vertically, the logic is short circuited.

    m_dragMode = updateDragMode
        (m_dragMode,
         m_clickPos,
         e->pos(),
         true, // can move horiz
         canTopLayerMoveVertical(), // can move vert
         canTopLayerMoveVertical() || (m_manager && m_manager->isPlaying()), // resist horiz
         !(m_manager && m_manager->isPlaying())); // resist vert

    if (m_dragMode == HorizontalDrag ||
        m_dragMode == FreeDrag) {

        sv_frame_t frameOff = getFrameForX(e->x()) - getFrameForX(m_clickPos.x());
        sv_frame_t newCentreFrame = m_dragCentreFrame;
        
        if (frameOff < 0) {
            newCentreFrame -= frameOff;
        } else if (newCentreFrame >= frameOff) {
            newCentreFrame -= frameOff;
        } else {
            newCentreFrame = 0;
        }

#ifdef DEBUG_PANE       
        cerr << "Pane::dragTopLayer: newCentreFrame = " << newCentreFrame <<
            ", models end frame = " << getModelsEndFrame() << endl;
#endif

        if (newCentreFrame >= getModelsEndFrame()) {
            newCentreFrame = getModelsEndFrame();
            if (newCentreFrame > 0) --newCentreFrame;
        }
                
        if (getXForFrame(m_centreFrame) != getXForFrame(newCentreFrame)) {
            setCentreFrame(newCentreFrame, !m_altPressed);
        }
    }

    if (m_dragMode == VerticalDrag ||
        m_dragMode == FreeDrag) {

        double vmin = 0.f, vmax = 0.f;
        double dmin = 0.f, dmax = 0.f;

        if (getTopLayerDisplayExtents(vmin, vmax, dmin, dmax)) {

//            cerr << "ydiff = " << ydiff << endl;

            int ydiff = e->y() - m_clickPos.y();
            double perpix = (dmax - dmin) / height();
            double valdiff = ydiff * perpix;
//            cerr << "valdiff = " << valdiff << endl;

            if (m_dragMode == UnresolvedDrag && ydiff != 0) {
                m_dragMode = VerticalDrag;
            }

            double newmin = m_dragStartMinValue + valdiff;
            double newmax = m_dragStartMinValue + (dmax - dmin) + valdiff;
            if (newmin < vmin) {
                newmax += vmin - newmin;
                newmin += vmin - newmin;
            }
            if (newmax > vmax) {
                newmin -= newmax - vmax;
                newmax -= newmax - vmax;
            }
//            cerr << "(" << dmin << ", " << dmax << ") -> ("
//                      << newmin << ", " << newmax << ") (drag start " << m_dragStartMinValue << ")" << endl;

            setTopLayerDisplayExtents(newmin, newmax);
            updateVerticalPanner();
        }
    }
}

Pane::DragMode
Pane::updateDragMode(DragMode dragMode,
                     QPoint origin,
                     QPoint point,
                     bool canMoveHorizontal,
                     bool canMoveVertical,
                     bool resistHorizontal,
                     bool resistVertical)
{
    int xdiff = point.x() - origin.x();
    int ydiff = point.y() - origin.y();

    int smallThreshold = 10, bigThreshold = 80;

    if (m_manager) {
        smallThreshold = m_manager->scalePixelSize(smallThreshold);
        bigThreshold = m_manager->scalePixelSize(bigThreshold);
    }

//    cerr << "Pane::updateDragMode: xdiff = " << xdiff << ", ydiff = "
//              << ydiff << ", canMoveVertical = " << canMoveVertical << ", drag mode = " << m_dragMode << endl;

    if (dragMode == UnresolvedDrag) {

        if (abs(ydiff) > smallThreshold &&
            abs(ydiff) > abs(xdiff) * 2 &&
            canMoveVertical) {
//            cerr << "Pane::updateDragMode: passed vertical threshold" << endl;
            dragMode = VerticalDrag;
        } else if (abs(xdiff) > smallThreshold &&
                   abs(xdiff) > abs(ydiff) * 2 &&
                   canMoveHorizontal) {
//            cerr << "Pane::updateDragMode: passed horizontal threshold" << endl;
            dragMode = HorizontalDrag;
        } else if (abs(xdiff) > smallThreshold &&
                   abs(ydiff) > smallThreshold &&
                   canMoveVertical &&
                   canMoveHorizontal) {
//            cerr << "Pane::updateDragMode: passed both thresholds" << endl;
            dragMode = FreeDrag;
        }
    }

    if (dragMode == VerticalDrag && canMoveHorizontal) {
        if (abs(xdiff) > bigThreshold) dragMode = FreeDrag;
    }

    if (dragMode == HorizontalDrag && canMoveVertical) {
        if (abs(ydiff) > bigThreshold) dragMode = FreeDrag;
    }

    if (dragMode == UnresolvedDrag) {
        if (!resistHorizontal && xdiff != 0) {
            dragMode = HorizontalDrag;
        }
        if (!resistVertical && ydiff != 0) {
            if (dragMode == HorizontalDrag) dragMode = FreeDrag;
            else dragMode = VerticalDrag;
        }
    }
    
    return dragMode;
}

void
Pane::dragExtendSelection(QMouseEvent *e)
{
    sv_frame_t mouseFrame = getFrameForX(e->x());
    int resolution = 1;
    sv_frame_t snapFrameLeft = mouseFrame;
    sv_frame_t snapFrameRight = mouseFrame;
    
    Layer *layer = getInteractionLayer();
    if (layer && !m_shiftPressed &&
        !qobject_cast<TimeRulerLayer *>(layer)) { // don't snap to secs
        layer->snapToFeatureFrame(this, snapFrameLeft,
                                  resolution, Layer::SnapLeft);
        layer->snapToFeatureFrame(this, snapFrameRight,
                                  resolution, Layer::SnapRight);
    }
	
//	cerr << "snap: frame = " << mouseFrame << ", start frame = " << m_selectionStartFrame << ", left = " << snapFrameLeft << ", right = " << snapFrameRight << endl;

    if (snapFrameLeft < 0) snapFrameLeft = 0;
    if (snapFrameRight < 0) snapFrameRight = 0;
    
    sv_frame_t min, max;
    
    if (m_selectionStartFrame > snapFrameLeft) {
        min = snapFrameLeft;
        max = m_selectionStartFrame;
    } else if (snapFrameRight > m_selectionStartFrame) {
        min = m_selectionStartFrame;
        max = snapFrameRight;
    } else {
        min = snapFrameLeft;
        max = snapFrameRight;
    }

    if (m_manager) {
        m_manager->setInProgressSelection(Selection(alignToReference(min),
                                                    alignToReference(max)),
                                          !m_resizing && !m_ctrlPressed);
    }

    edgeScrollMaybe(e->x());

    update();

    if (min != max) {
        m_playbackFrameMoveScheduled = false;
    }
}

void
Pane::edgeScrollMaybe(int x)
{
    sv_frame_t mouseFrame = getFrameForX(x);

    bool doScroll = false;
    if (!m_manager) doScroll = true;
    else if (!m_manager->isPlaying()) doScroll = true;

    if (m_followPlay != PlaybackScrollContinuous) doScroll = true;

    if (doScroll) {
        sv_frame_t offset = mouseFrame - getStartFrame();
        sv_frame_t available = getEndFrame() - getStartFrame();
        sv_frame_t move = 0;
        if (offset >= double(available) * 0.95) {
            move = sv_frame_t(double(offset - available) * 0.95) + 1;
        } else if (offset <= double(available) * 0.10) {
            move = sv_frame_t(double(available) * 0.10 - double(offset)) + 1;
            move = -move;
        }
        if (move != 0) {
            setCentreFrame(m_centreFrame + move);
            update();
        }
    }
}

void
Pane::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::RightButton) {
        return;
    }

    cerr << "mouseDoubleClickEvent" << endl;

    m_clickPos = e->pos();
    m_clickedInRange = true;
    m_shiftPressed = (e->modifiers() & Qt::ShiftModifier);
    m_ctrlPressed = (e->modifiers() & Qt::ControlModifier);
    m_altPressed = (e->modifiers() & Qt::AltModifier);

    // cancel any pending move that came from a single click
    m_playbackFrameMoveScheduled = false;

    ViewManager::ToolMode mode = ViewManager::NavigateMode;
    if (m_manager) mode = m_manager->getToolModeFor(this);

    bool relocate = (mode == ViewManager::NavigateMode ||
                     (e->buttons() & Qt::MidButton));

    if (mode == ViewManager::SelectMode) {
        m_clickedInRange = false;
        if (m_manager) m_manager->clearInProgressSelection();
        emit doubleClickSelectInvoked(getFrameForX(e->x()));
        return;
    }

    if (mode == ViewManager::NavigateMode ||
        mode == ViewManager::EditMode) {

        Layer *layer = getInteractionLayer();
        if (layer && layer->isLayerEditable()) {
            if (layer->editOpen(this, e)) relocate = false;
        }

    } else if (mode == ViewManager::MeasureMode) {

        Layer *layer = getTopLayer();
        if (layer) layer->measureDoubleClick(this, e);
        update();
    }

    if (relocate) {

        sv_frame_t f = getFrameForX(e->x());

        setCentreFrame(f);

        m_dragCentreFrame = f;
        m_dragStartMinValue = 0;
        m_dragMode = UnresolvedDrag;

        double vmin, vmax, dmin, dmax;
        if (getTopLayerDisplayExtents(vmin, vmax, dmin, dmax)) {
            m_dragStartMinValue = dmin;
        }
    }
    
    if (mode == ViewManager::NoteEditMode) {
        std::cerr << "double click in note edit mode" << std::endl;
        Layer *layer = getInteractionLayer();
        if (layer && layer->isLayerEditable()) {
            layer->addNote(this, e); 
        }
    }

    m_clickedInRange = false; // in case mouseReleaseEvent is not properly called
}

void
Pane::enterEvent(QEvent *)
{
    m_mouseInWidget = true;
}

void
Pane::leaveEvent(QEvent *)
{
    m_mouseInWidget = false;
    bool previouslyIdentifying = m_identifyFeatures;
    m_identifyFeatures = false;
    if (previouslyIdentifying) update();
    emit contextHelpChanged("");
}

void
Pane::resizeEvent(QResizeEvent *)
{
    updateHeadsUpDisplay();
}

void
Pane::wheelEvent(QWheelEvent *e)
{
    cerr << "wheelEvent, delta " << e->delta() << ", angleDelta " << e->angleDelta().x() << "," << e->angleDelta().y() << ", pixelDelta " << e->pixelDelta().x() << "," << e->pixelDelta().y() << ", modifiers " << e->modifiers() << endl;

    int dx = e->angleDelta().x();
    int dy = e->angleDelta().y();

    if (dx == 0 && dy == 0) {
        return;
    }

    int d = dy;
    bool horizontal = false;

    if (abs(dx) > abs(dy)) {
        d = dx;
        horizontal = true;
    } else if (e->modifiers() & Qt::ControlModifier) {
        // treat a vertical wheel as horizontal
        horizontal = true;
    }

    if (e->phase() == Qt::ScrollBegin ||
        fabs(d) >= 120 ||
        (d > 0 && m_pendingWheelAngle < 0) ||
        (d < 0 && m_pendingWheelAngle > 0)) {
        m_pendingWheelAngle = d;
    } else {
        m_pendingWheelAngle += d;
    }

    if (horizontal && e->pixelDelta().x() != 0) {

        // Have fine pixel information: use it

        wheelHorizontalFine(e->pixelDelta().x(), e->modifiers());
    
        m_pendingWheelAngle = 0;

    } else {

        // Coarse wheel information (or vertical zoom, which is
        // necessarily coarse itself)

        // Sometimes on Linux we're seeing absurdly extreme angles on
        // the first wheel event -- discard those entirely
        if (abs(m_pendingWheelAngle) >= 600) {
            m_pendingWheelAngle = 0;
            return;
        }
        
        while (abs(m_pendingWheelAngle) >= 120) {

            int sign = (m_pendingWheelAngle < 0 ? -1 : 1);

            if (horizontal) {
                wheelHorizontal(sign, e->modifiers());
            } else {
                wheelVertical(sign, e->modifiers());
            }

            m_pendingWheelAngle -= sign * 120;
        }
    }
}

void
Pane::wheelVertical(int sign, Qt::KeyboardModifiers mods)
{
    cerr << "wheelVertical: sign = " << sign << endl;

    if (mods & Qt::ShiftModifier) {

        // Pan vertically

        if (m_vpan) {
            m_vpan->scroll(sign > 0);
        }

    } else if (mods & Qt::AltModifier) {

        // Zoom vertically

        if (m_vthumb) {
            m_vthumb->scroll(sign > 0);
        }

    } else {

        // Zoom in or out

        int newZoomLevel = m_zoomLevel;
  
        if (sign > 0) {
            if (newZoomLevel <= 2) {
                newZoomLevel = 1;
            } else {
                newZoomLevel = getZoomConstraintBlockSize
                    (newZoomLevel - 1, ZoomConstraint::RoundDown);
            }
        } else { // sign < 0
            newZoomLevel = getZoomConstraintBlockSize
                (newZoomLevel + 1, ZoomConstraint::RoundUp);
        }
    
        if (newZoomLevel != m_zoomLevel) {
            setZoomLevel(newZoomLevel);
        }
    }

    emit paneInteractedWith();
}

void
Pane::wheelHorizontal(int sign, Qt::KeyboardModifiers mods)
{
    cerr << "wheelHorizontal: sign = " << sign << endl;

    // Scroll left or right, rapidly

    wheelHorizontalFine((width() / 4) * sign, mods);
}

void
Pane::wheelHorizontalFine(int pixels, Qt::KeyboardModifiers)
{
    cerr << "wheelHorizontalFine: pixels = " << pixels << endl;

    // Scroll left or right by a fixed number of pixels

    if (getStartFrame() < 0 && 
        getEndFrame() >= getModelsEndFrame()) return;

    int delta = (pixels * m_zoomLevel);

    if (m_centreFrame < delta) {
        setCentreFrame(0);
    } else if (m_centreFrame - delta >= getModelsEndFrame()) {
        setCentreFrame(getModelsEndFrame());
    } else {
        setCentreFrame(m_centreFrame - delta);
    }

    emit paneInteractedWith();
}

void
Pane::horizontalThumbwheelMoved(int value)
{
    //!!! dupe with updateHeadsUpDisplay

    int count = 0;
    int level = 1;


    //!!! pull out into function (presumably in View)
    bool haveConstraint = false;
    for (LayerList::const_iterator i = m_layerStack.begin(); i != m_layerStack.end();
         ++i) {
        if ((*i)->getZoomConstraint() && !(*i)->supportsOtherZoomLevels()) {
            haveConstraint = true;
            break;
        }
    }

    if (haveConstraint) {
        while (true) {
            if (m_hthumb->getMaximumValue() - value == count) break;
            int newLevel = getZoomConstraintBlockSize(level + 1,
                                                      ZoomConstraint::RoundUp);
            if (newLevel == level) break;
            level = newLevel;
            if (++count == 50) break;
        }
    } else {
        while (true) {
            if (m_hthumb->getMaximumValue() - value == count) break;
            int step = level / 10;
            int pwr = 0;
            while (step > 0) {
                ++pwr;
                step /= 2;
            }
            step = 1;
            while (pwr > 0) {
                step *= 2;
                --pwr;
            }
//            cerr << level << endl;
            level += step;
            if (++count == 100 || level > 262144) break;
        }
    }
        
//    cerr << "new level is " << level << endl;
    setZoomLevel(level);
}    

void
Pane::verticalThumbwheelMoved(int value)
{
    Layer *layer = 0;
    if (getLayerCount() > 0) layer = getLayer(getLayerCount() - 1);
    if (layer) {
        int defaultStep = 0;
        int max = layer->getVerticalZoomSteps(defaultStep);
        if (max == 0) {
            updateHeadsUpDisplay();
            return;
        }
        if (value > max) {
            value = max;
        }
        layer->setVerticalZoomStep(value);
        updateVerticalPanner();
    }
}    

void
Pane::verticalPannerMoved(float , float y0, float , float h)
{
    double vmin, vmax, dmin, dmax;
    if (!getTopLayerDisplayExtents(vmin, vmax, dmin, dmax)) return;
    double y1 = y0 + h;
    double newmax = vmin + ((1.0 - y0) * (vmax - vmin));
    double newmin = vmin + ((1.0 - y1) * (vmax - vmin));
//    cerr << "verticalPannerMoved: (" << x0 << "," << y0 << "," << w
//              << "," << h << ") -> (" << newmin << "," << newmax << ")" << endl;
    setTopLayerDisplayExtents(newmin, newmax);
}

void
Pane::editVerticalPannerExtents()
{
    if (!m_vpan || !m_manager || !m_manager->getZoomWheelsEnabled()) return;

    double vmin, vmax, dmin, dmax;
    QString unit;
    if (!getTopLayerDisplayExtents(vmin, vmax, dmin, dmax, &unit)
        || vmax == vmin) {
        return;
    }

    RangeInputDialog dialog(tr("Enter new range"),
                            tr("New vertical display range, from %1 to %2 %4:")
                            .arg(vmin).arg(vmax).arg(unit),
                            unit, double(vmin), double(vmax), this);
    dialog.setRange(double(dmin), double(dmax));

    if (dialog.exec() == QDialog::Accepted) {
        double newmin, newmax;
        dialog.getRange(newmin, newmax);
        setTopLayerDisplayExtents(newmin, newmax);
        updateVerticalPanner();
    }
}

void
Pane::layerParametersChanged()
{
    View::layerParametersChanged();
    updateHeadsUpDisplay();
}

void
Pane::dragEnterEvent(QDragEnterEvent *e)
{
    QStringList formats(e->mimeData()->formats());
    cerr << "dragEnterEvent: format: "
              << formats.join(",")
              << ", possibleActions: " << e->possibleActions()
              << ", proposedAction: " << e->proposedAction() << endl;
    
    if (e->mimeData()->hasFormat("text/uri-list") ||
        e->mimeData()->hasFormat("text/plain")) {

        if (e->proposedAction() & Qt::CopyAction) {
            e->acceptProposedAction();
        } else {
            e->setDropAction(Qt::CopyAction);
            e->accept();
        }
    }
}

void
Pane::dropEvent(QDropEvent *e)
{
    cerr << "dropEvent: text: \"" << e->mimeData()->text()
              << "\"" << endl;

    if (e->mimeData()->hasFormat("text/uri-list") || 
        e->mimeData()->hasFormat("text/plain")) {

        if (e->proposedAction() & Qt::CopyAction) {
            e->acceptProposedAction();
        } else {
            e->setDropAction(Qt::CopyAction);
            e->accept();
        }

        if (e->mimeData()->hasFormat("text/uri-list")) {

            cerr << "accepting... data is \"" << e->mimeData()->data("text/uri-list").data() << "\"" << endl;
            emit dropAccepted(QString::fromLocal8Bit
                              (e->mimeData()->data("text/uri-list").data())
                              .split(QRegExp("[\\r\\n]+"), 
                                     QString::SkipEmptyParts));
        } else {
            emit dropAccepted(QString::fromLocal8Bit
                              (e->mimeData()->data("text/plain").data()));
        }
    }
}

bool
Pane::editSelectionStart(QMouseEvent *e)
{
    if (!m_identifyFeatures ||
        !m_manager ||
        m_manager->getToolModeFor(this) != ViewManager::EditMode) {
        return false;
    }

    bool closeToLeft, closeToRight;
    Selection s(getSelectionAt(e->x(), closeToLeft, closeToRight));
    if (s.isEmpty()) return false;
    m_editingSelection = s;
    m_editingSelectionEdge = (closeToLeft ? -1 : closeToRight ? 1 : 0);
    m_mousePos = e->pos();
    return true;
}

bool
Pane::editSelectionDrag(QMouseEvent *e)
{
    if (m_editingSelection.isEmpty()) return false;
    m_mousePos = e->pos();
    update();
    return true;
}

bool
Pane::editSelectionEnd(QMouseEvent *)
{
    if (m_editingSelection.isEmpty()) return false;

    int offset = m_mousePos.x() - m_clickPos.x();
    Layer *layer = getInteractionLayer();

    if (offset == 0 || !layer) {
        m_editingSelection = Selection();
        return true;
    }

    int p0 = getXForFrame(m_editingSelection.getStartFrame()) + offset;
    int p1 = getXForFrame(m_editingSelection.getEndFrame()) + offset;

    sv_frame_t f0 = getFrameForX(p0);
    sv_frame_t f1 = getFrameForX(p1);

    Selection newSelection(f0, f1);
    
    if (m_editingSelectionEdge == 0) {
    
        CommandHistory::getInstance()->startCompoundOperation
            (tr("Drag Selection"), true);

        layer->moveSelection(m_editingSelection, f0);
    
    } else {
    
        CommandHistory::getInstance()->startCompoundOperation
            (tr("Resize Selection"), true);

        if (m_editingSelectionEdge < 0) {
            f1 = m_editingSelection.getEndFrame();
        } else {
            f0 = m_editingSelection.getStartFrame();
        }

        newSelection = Selection(f0, f1);
        layer->resizeSelection(m_editingSelection, newSelection);
    }
    
    m_manager->removeSelection(m_editingSelection);
    m_manager->addSelection(newSelection);

    CommandHistory::getInstance()->endCompoundOperation();

    m_editingSelection = Selection();
    return true;
}

void
Pane::toolModeChanged()
{
    ViewManager::ToolMode mode = m_manager->getToolModeFor(this);
//    cerr << "Pane::toolModeChanged(" << mode << ")" << endl;

    if (mode == ViewManager::MeasureMode && !m_measureCursor1) {
        m_measureCursor1 = new QCursor(QBitmap(":/icons/visualiser/measure1cursor.xbm"),
                                       QBitmap(":/icons/visualiser/measure1mask.xbm"),
                                       15, 14);
        m_measureCursor2 = new QCursor(QBitmap(":/icons/visualiser/measure2cursor.xbm"),
                                       QBitmap(":/icons/visualiser/measure2mask.xbm"),
                                       16, 17);
    }

    switch (mode) {

    case ViewManager::NavigateMode:
        setCursor(Qt::PointingHandCursor);
        break;
    
    case ViewManager::SelectMode:
        setCursor(Qt::ArrowCursor);
        break;
    
    case ViewManager::EditMode:
        setCursor(Qt::UpArrowCursor);
        break;
    
    case ViewManager::DrawMode:
        setCursor(Qt::CrossCursor);
        break;
    
    case ViewManager::EraseMode:
        setCursor(Qt::CrossCursor);
        break;

    case ViewManager::MeasureMode:
        if (m_measureCursor1) setCursor(*m_measureCursor1);
        break;

        // GF: NoteEditMode uses the same default cursor as EditMode, but it will change in a context sensitive manner.
    case ViewManager::NoteEditMode:
        setCursor(Qt::UpArrowCursor);
        break;

/*  
    case ViewManager::TextMode:
    setCursor(Qt::IBeamCursor);
    break;
*/
    }
}

void
Pane::zoomWheelsEnabledChanged()
{
    updateHeadsUpDisplay();
    update();
}

void
Pane::viewZoomLevelChanged(View *v, int z, bool locked)
{
//    cerr << "Pane[" << this << "]::zoomLevelChanged (global now "
//              << (m_manager ? m_manager->getGlobalZoom() : 0) << ")" << endl;

    View::viewZoomLevelChanged(v, z, locked);

    if (m_hthumb && !m_hthumb->isVisible()) return;

    if (v != this) {
        if (!locked || !m_followZoom) return;
    }

    if (m_manager && m_manager->getZoomWheelsEnabled()) {
        updateHeadsUpDisplay();
    }
}

void
Pane::propertyContainerSelected(View *v, PropertyContainer *pc)
{
    Layer *layer = 0;

    if (getLayerCount() > 0) {
        layer = getLayer(getLayerCount() - 1);
        disconnect(layer, SIGNAL(verticalZoomChanged()),
                   this, SLOT(verticalZoomChanged()));
    }

    View::propertyContainerSelected(v, pc);
    updateHeadsUpDisplay();

    if (m_vthumb) {
        RangeMapper *rm = 0;
        if (layer) rm = layer->getNewVerticalZoomRangeMapper();
        if (rm) m_vthumb->setRangeMapper(rm);
    }

    if (getLayerCount() > 0) {
        layer = getLayer(getLayerCount() - 1);
        connect(layer, SIGNAL(verticalZoomChanged()),
                this, SLOT(verticalZoomChanged()));
    }
}

void
Pane::verticalZoomChanged()
{
    Layer *layer = 0;

    if (getLayerCount() > 0) {

        layer = getLayer(getLayerCount() - 1);

        if (m_vthumb && m_vthumb->isVisible()) {
            m_vthumb->setValue(layer->getCurrentVerticalZoomStep());
        }
    }
}

void
Pane::updateContextHelp(const QPoint *pos)
{
    QString help = "";

    if (m_clickedInRange) {
        emit contextHelpChanged("");
        return;
    }

    ViewManager::ToolMode mode = ViewManager::NavigateMode;
    if (m_manager) mode = m_manager->getToolModeFor(this);

    bool editable = false;
    Layer *layer = getInteractionLayer();
    if (layer && layer->isLayerEditable()) {
        editable = true;
    }
        
    if (mode == ViewManager::NavigateMode) {

        help = tr("Click and drag to navigate");
        
    } else if (mode == ViewManager::SelectMode) {

        if (!hasTopLayerTimeXAxis()) return;

        bool haveSelection = (m_manager && !m_manager->getSelections().empty());

        if (haveSelection) {
#ifdef Q_OS_MAC
            if (editable) {
                help = tr("Click and drag to select a range; hold Shift to avoid snapping to items; hold Cmd for multi-select; middle-click and drag to navigate");
            } else {
                help = tr("Click and drag to select a range; hold Cmd for multi-select; middle-click and drag to navigate");
            }
#else
            if (editable) {
                help = tr("Click and drag to select a range; hold Shift to avoid snapping to items; hold Ctrl for multi-select; middle-click and drag to navigate");
            } else {
                help = tr("Click and drag to select a range; hold Ctrl for multi-select; middle-click and drag to navigate");
            }                
#endif

            if (pos) {
                bool closeToLeft = false, closeToRight = false;
                Selection selection = getSelectionAt(pos->x(), closeToLeft, closeToRight);
                if ((closeToLeft || closeToRight) && !(closeToLeft && closeToRight)) {
                    
                    help = tr("Click and drag to move the selection boundary");
                }
            }
        } else {
            if (editable) {
                help = tr("Click and drag to select a range; hold Shift to avoid snapping to items; middle-click to navigate");
            } else {
                help = tr("Click and drag to select a range; middle-click and drag to navigate");
            }
        }

    } else if (mode == ViewManager::DrawMode) {
        
        //!!! could call through to a layer function to find out exact meaning
        if (editable) {
            help = tr("Click to add a new item in the active layer");
        }

    } else if (mode == ViewManager::EraseMode) {
        
        //!!! could call through to a layer function to find out exact meaning
        if (editable) {
            help = tr("Click to erase an item from the active layer");
        }
        
    } else if (mode == ViewManager::EditMode) {
        
        //!!! could call through to layer
        if (editable) {
            help = tr("Click and drag an item in the active layer to move it; hold Shift to override initial resistance");
            if (pos) {
                bool closeToLeft = false, closeToRight = false;
                Selection selection = getSelectionAt(pos->x(), closeToLeft, closeToRight);
                if (!selection.isEmpty()) {
                    help = tr("Click and drag to move all items in the selected range");
                }
            }
        }
    }

    emit contextHelpChanged(help);
}

void
Pane::mouseEnteredWidget()
{
    QWidget *w = dynamic_cast<QWidget *>(sender());
    if (!w) return;

    if (w == m_vpan) {
        emit contextHelpChanged(tr("Click and drag to adjust the visible range of the vertical scale"));
    } else if (w == m_vthumb) {
        emit contextHelpChanged(tr("Click and drag to adjust the vertical zoom level"));
    } else if (w == m_hthumb) {
        emit contextHelpChanged(tr("Click and drag to adjust the horizontal zoom level"));
    } else if (w == m_reset) {
        emit contextHelpChanged(tr("Reset horizontal and vertical zoom levels to their defaults"));
    }
}

void
Pane::mouseLeftWidget()
{
    emit contextHelpChanged("");
}

void
Pane::toXml(QTextStream &stream,
            QString indent, QString extraAttributes) const
{
    View::toXml
        (stream, indent,
     QString("type=\"pane\" centreLineVisible=\"%1\" height=\"%2\" %3")
     .arg(m_centreLineVisible).arg(height()).arg(extraAttributes));
}


