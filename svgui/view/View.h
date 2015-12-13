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

#ifndef _VIEW_H_
#define _VIEW_H_

#include <QFrame>
#include <QProgressBar>

#include "base/ZoomConstraint.h"
#include "base/PropertyContainer.h"
#include "ViewManager.h"
#include "base/XmlExportable.h"
#include "base/BaseTypes.h"

// #define DEBUG_VIEW_WIDGET_PAINT 1

class Layer;
class ViewPropertyContainer;

class QPushButton;

#include <map>
#include <set>

/**
 * View is the base class of widgets that display one or more
 * overlaid views of data against a horizontal time scale. 
 *
 * A View may have any number of attached Layers, each of which
 * is expected to have one data Model (although multiple views may
 * share the same model).
 *
 * A View may be panned in time and zoomed, although the
 * mechanisms for doing so (as well as any other operations and
 * properties available) depend on the subclass.
 */

class View : public QFrame,
	     public XmlExportable
{
    Q_OBJECT

public:
    /**
     * Deleting a View does not delete any of its layers.  They should
     * be managed elsewhere (e.g. by the Document).
     */
    virtual ~View();

    /**
     * Retrieve the first visible sample frame on the widget.
     * This is a calculated value based on the centre-frame, widget
     * width and zoom level.  The result may be negative.
     */
    sv_frame_t getStartFrame() const;

    /**
     * Set the widget pan based on the given first visible frame.  The
     * frame value may be negative.
     */
    void setStartFrame(sv_frame_t);

    /**
     * Return the centre frame of the visible widget.  This is an
     * exact value that does not depend on the zoom block size.  Other
     * frame values (start, end) are calculated from this based on the
     * zoom and other factors.
     */
    sv_frame_t getCentreFrame() const { return m_centreFrame; }

    /**
     * Set the centre frame of the visible widget.
     */
    void setCentreFrame(sv_frame_t f) { setCentreFrame(f, true); }

    /**
     * Retrieve the last visible sample frame on the widget.
     * This is a calculated value based on the centre-frame, widget
     * width and zoom level.
     */
    sv_frame_t getEndFrame() const;

    /**
     * Return the pixel x-coordinate corresponding to a given sample
     * frame (which may be negative).
     */
    int getXForFrame(sv_frame_t frame) const;

    /**
     * Return the closest frame to the given pixel x-coordinate.
     */
    sv_frame_t getFrameForX(int x) const;

    /**
     * Return the pixel y-coordinate corresponding to a given
     * frequency, if the frequency range is as specified.  This does
     * not imply any policy about layer frequency ranges, but it might
     * be useful for layers to match theirs up if desired.
     *
     * Not thread-safe in logarithmic mode.  Call only from GUI thread.
     */
    double getYForFrequency(double frequency, double minFreq, double maxFreq, 
			   bool logarithmic) const;

    /**
     * Return the closest frequency to the given pixel y-coordinate,
     * if the frequency range is as specified.
     *
     * Not thread-safe in logarithmic mode.  Call only from GUI thread.
     */
    double getFrequencyForY(int y, double minFreq, double maxFreq,
			   bool logarithmic) const;

    /**
     * Return the zoom level, i.e. the number of frames per pixel
     */
    int getZoomLevel() const;

    /**
     * Set the zoom level, i.e. the number of frames per pixel.  The
     * centre frame will be unchanged; the start and end frames will
     * change.
     */
    virtual void setZoomLevel(int z);

    /**
     * Zoom in or out.
     */
    virtual void zoom(bool in);

    /**
     * Scroll left or right by a smallish or largish amount.
     */
    virtual void scroll(bool right, bool lots, bool doEmit = true);

    /**
     * Add a layer to the view. (Normally this should be handled
     * through some command abstraction instead of using this function
     * directly.)
     */
    virtual void addLayer(Layer *v);

    /**
     * Remove a layer from the view. Does not delete the
     * layer. (Normally this should be handled through some command
     * abstraction instead of using this function directly.)
     */
    virtual void removeLayer(Layer *v);

    /**
     * Return the number of layers, regardless of whether visible or
     * dormant, i.e. invisible, in this view.
     */
    virtual int getLayerCount() const { return int(m_layerStack.size()); }

    /**
     * Return the nth layer, counted in stacking order.  That is,
     * layer 0 is the bottom layer and layer "getLayerCount()-1" is
     * the top one. The returned layer may be visible or it may be
     * dormant, i.e. invisible.
     */
    virtual Layer *getLayer(int n) {
        if (in_range_for(m_layerStack, n)) return m_layerStack[n];
        else return 0;
    }

    /**
     * Return the nth layer, counted in the order they were
     * added. Unlike the stacking order used in getLayer(), which
     * changes each time a layer is selected, this ordering remains
     * fixed. The returned layer may be visible or it may be dormant,
     * i.e. invisible.
     */
    virtual Layer *getFixedOrderLayer(int n) {
        if (n < int(m_fixedOrderLayers.size())) return m_fixedOrderLayers[n];
        else return 0;
    }

    /**
     * Return the layer currently active for tool interaction. This is
     * the topmost non-dormant (i.e. visible) layer in the view. If
     * there are no visible layers in the view, return 0.
     */
    virtual Layer *getInteractionLayer();

    virtual const Layer *getInteractionLayer() const;

    /**
     * Return the layer most recently selected by the user. This is
     * the layer that any non-tool-driven commands should operate on,
     * in the case where this view is the "current" one.
     *
     * If the user has selected the view itself more recently than any
     * of the layers on it, this function will return 0, and any
     * non-tool-driven layer commands should be deactivated while this
     * view is current. It will also return 0 if there are no layers
     * in the view.
     *
     * Note that, unlike getInteractionLayer(), this could return an
     * invisible (dormant) layer.
     */
    virtual Layer *getSelectedLayer();

    virtual const Layer *getSelectedLayer() const;

    /**
     * Return the "top" layer in the view, whether visible or dormant.
     * This is the same as getLayer(getLayerCount()-1) if there is at
     * least one layer, and 0 otherwise.
     *
     * For most purposes involving interaction or commands, you
     * probably want either getInteractionLayer() or
     * getSelectedLayer() instead.
     */
    virtual Layer *getTopLayer() {
        return m_layerStack.empty() ? 0 : m_layerStack[m_layerStack.size()-1];
    }

    virtual void setViewManager(ViewManager *m);
    virtual void setViewManager(ViewManager *m, sv_frame_t initialFrame);
    virtual ViewManager *getViewManager() const { return m_manager; }

    virtual void setFollowGlobalPan(bool f);
    virtual bool getFollowGlobalPan() const { return m_followPan; }

    virtual void setFollowGlobalZoom(bool f);
    virtual bool getFollowGlobalZoom() const { return m_followZoom; }

    virtual bool hasLightBackground() const;
    virtual QColor getForeground() const;
    virtual QColor getBackground() const;

    enum TextStyle {
	BoxedText,
	OutlinedText,
        OutlinedItalicText
    };

    virtual void drawVisibleText(QPainter &p, int x, int y,
				 QString text, TextStyle style) const;

    virtual void drawMeasurementRect(QPainter &p, const Layer *,
                                     QRect rect, bool focus) const;

    virtual bool shouldShowFeatureLabels() const {
        return m_manager && m_manager->shouldShowFeatureLabels();
    }
    virtual bool shouldIlluminateLocalFeatures(const Layer *, QPoint &) const {
	return false;
    }
    virtual bool shouldIlluminateLocalSelection(QPoint &, bool &, bool &) const {
	return false;
    }

    virtual void setPlaybackFollow(PlaybackFollowMode m);
    virtual PlaybackFollowMode getPlaybackFollow() const { return m_followPlay; }

    typedef PropertyContainer::PropertyName PropertyName;

    // We implement the PropertyContainer API, although we don't
    // actually subclass PropertyContainer.  We have our own
    // PropertyContainer that we can return on request that just
    // delegates back to us.
    virtual PropertyContainer::PropertyList getProperties() const;
    virtual QString getPropertyLabel(const PropertyName &) const;
    virtual PropertyContainer::PropertyType getPropertyType(const PropertyName &) const;
    virtual int getPropertyRangeAndValue(const PropertyName &,
					 int *min, int *max, int *deflt) const;
    virtual QString getPropertyValueLabel(const PropertyName &,
					  int value) const;
    virtual void setProperty(const PropertyName &, int value);
    virtual QString getPropertyContainerName() const {
	return objectName();
    }
    virtual QString getPropertyContainerIconName() const = 0;

    virtual int getPropertyContainerCount() const;

    // The 0th property container is the view's own; the rest are the
    // layers in fixed-order series
    virtual const PropertyContainer *getPropertyContainer(int i) const;
    virtual PropertyContainer *getPropertyContainer(int i);

    // Render the contents on a wide canvas
    virtual QImage *toNewImage(sv_frame_t f0, sv_frame_t f1);
    virtual QImage *toNewImage();
    virtual QSize getImageSize(sv_frame_t f0, sv_frame_t f1);
    virtual QSize getImageSize();

    virtual int getTextLabelHeight(const Layer *layer, QPainter &) const;

    virtual bool getValueExtents(QString unit, double &min, double &max,
                                 bool &log) const;

    virtual void toXml(QTextStream &stream, QString indent = "",
                       QString extraAttributes = "") const;

    // First frame actually in model, to right of scale, if present
    virtual sv_frame_t getFirstVisibleFrame() const;
    virtual sv_frame_t getLastVisibleFrame() const;

    sv_frame_t getModelsStartFrame() const;
    sv_frame_t getModelsEndFrame() const;

    typedef std::set<Model *> ModelSet;
    ModelSet getModels();

    //!!!
    Model *getAligningModel() const;
    sv_frame_t alignFromReference(sv_frame_t) const;
    sv_frame_t alignToReference(sv_frame_t) const;
    sv_frame_t getAlignedPlaybackFrame() const;

signals:
    void propertyContainerAdded(PropertyContainer *pc);
    void propertyContainerRemoved(PropertyContainer *pc);
    void propertyContainerPropertyChanged(PropertyContainer *pc);
    void propertyContainerPropertyRangeChanged(PropertyContainer *pc);
    void propertyContainerNameChanged(PropertyContainer *pc);
    void propertyContainerSelected(PropertyContainer *pc);
    void propertyChanged(PropertyContainer::PropertyName);

    void layerModelChanged();

    void centreFrameChanged(sv_frame_t frame,
                            bool globalScroll,
                            PlaybackFollowMode followMode);

    void zoomLevelChanged(int, bool);

    void contextHelpChanged(const QString &);

public slots:
    virtual void modelChanged();
    virtual void modelChangedWithin(sv_frame_t startFrame, sv_frame_t endFrame);
    virtual void modelCompletionChanged();
    virtual void modelAlignmentCompletionChanged();
    virtual void modelReplaced();
    virtual void layerParametersChanged();
    virtual void layerParameterRangesChanged();
    virtual void layerMeasurementRectsChanged();
    virtual void layerNameChanged();

    virtual void globalCentreFrameChanged(sv_frame_t);
    virtual void viewCentreFrameChanged(View *, sv_frame_t);
    virtual void viewManagerPlaybackFrameChanged(sv_frame_t);
    virtual void viewZoomLevelChanged(View *, int, bool);

    virtual void propertyContainerSelected(View *, PropertyContainer *pc);

    virtual void selectionChanged();
    virtual void toolModeChanged();
    virtual void overlayModeChanged();
    virtual void zoomWheelsEnabledChanged();

    virtual void cancelClicked();

    virtual void progressCheckStalledTimerElapsed();

protected:
    View(QWidget *, bool showProgress);
    virtual void paintEvent(QPaintEvent *e);
    virtual void drawSelections(QPainter &);
    virtual bool shouldLabelSelections() const { return true; }
    virtual bool render(QPainter &paint, int x0, sv_frame_t f0, sv_frame_t f1);
    virtual void setPaintFont(QPainter &paint);
    
    typedef std::vector<Layer *> LayerList;

    sv_samplerate_t getModelsSampleRate() const;
    bool areLayersScrollable() const;
    LayerList getScrollableBackLayers(bool testChanged, bool &changed) const;
    LayerList getNonScrollableFrontLayers(bool testChanged, bool &changed) const;
    int getZoomConstraintBlockSize(int blockSize,
				      ZoomConstraint::RoundingDirection dir =
				      ZoomConstraint::RoundNearest) const;

    // True if the top layer(s) use colours for meaningful things.  If
    // this is the case, selections will be shown using unfilled boxes
    // rather than with a translucent fill.
    bool areLayerColoursSignificant() const;

    // True if the top layer has a time axis on the x coordinate (this
    // is generally the case except for spectrum/slice layers).  It
    // will not be possible to make or display selections if this is
    // false.
    bool hasTopLayerTimeXAxis() const;

    bool setCentreFrame(sv_frame_t f, bool doEmit);

    void movePlayPointer(sv_frame_t f);

    void checkProgress(void *object);
    int getProgressBarWidth() const; // if visible

    sv_frame_t          m_centreFrame;
    int                 m_zoomLevel;
    bool                m_followPan;
    bool                m_followZoom;
    PlaybackFollowMode  m_followPlay;
    bool                m_followPlayIsDetached;
    sv_frame_t          m_playPointerFrame;
    bool                m_lightBackground;
    bool                m_showProgress;

    QPixmap            *m_cache;
    sv_frame_t          m_cacheCentreFrame;
    int                 m_cacheZoomLevel;
    bool                m_selectionCached;

    bool                m_deleting;

    LayerList           m_layerStack; // I don't own these, but see dtor note above
    LayerList           m_fixedOrderLayers;
    bool                m_haveSelectedLayer;

    QString             m_lastError;

    // caches for use in getScrollableBackLayers, getNonScrollableFrontLayers
    mutable LayerList m_lastScrollableBackLayers;
    mutable LayerList m_lastNonScrollableBackLayers;

    struct ProgressBarRec {
        QPushButton *cancel;
        QProgressBar *bar;
        int lastCheck;
        QTimer *checkTimer;
    };
    typedef std::map<Layer *, ProgressBarRec> ProgressMap;
    ProgressMap m_progressBars; // I own the ProgressBars

    ViewManager *m_manager; // I don't own this
    ViewPropertyContainer *m_propertyContainer; // I own this
};


// Use this for delegation, because we can't subclass from
// PropertyContainer (which is a QObject) ourselves because of
// ambiguity with QFrame parent

class ViewPropertyContainer : public PropertyContainer
{
    Q_OBJECT

public:
    ViewPropertyContainer(View *v);
    virtual ~ViewPropertyContainer();

    PropertyList getProperties() const { return m_v->getProperties(); }
    QString getPropertyLabel(const PropertyName &n) const {
        return m_v->getPropertyLabel(n);
    }
    PropertyType getPropertyType(const PropertyName &n) const {
	return m_v->getPropertyType(n);
    }
    int getPropertyRangeAndValue(const PropertyName &n, int *min, int *max,
                                 int *deflt) const {
	return m_v->getPropertyRangeAndValue(n, min, max, deflt);
    }
    QString getPropertyValueLabel(const PropertyName &n, int value) const {
	return m_v->getPropertyValueLabel(n, value);
    }
    QString getPropertyContainerName() const {
	return m_v->getPropertyContainerName();
    }
    QString getPropertyContainerIconName() const {
	return m_v->getPropertyContainerIconName();
    }

public slots:
    virtual void setProperty(const PropertyName &n, int value) {
	m_v->setProperty(n, value);
    }

protected:
    View *m_v;
};

#endif

