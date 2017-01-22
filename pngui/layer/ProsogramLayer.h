#ifndef PROSOGRAMLAYER_H
#define PROSOGRAMLAYER_H

#include "pngui/model/annotation/ProsogramModel.h"
#include "svgui/layer/SingleColourLayer.h"
#include "svgui/layer/VerticalScaleLayer.h"
#include "svgui/layer/TimeValueLayer.h"

#include <QObject>
#include <QColor>

class View;
class QPainter;

class ProsogramLayer : public SingleColourLayer, public VerticalScaleLayer
{
    Q_OBJECT

public:
    ProsogramLayer();
    virtual ~ProsogramLayer();

    std::string getType() const { return "Prosogram"; }

    virtual const Model *getModel() const { return m_model; }
    virtual bool trySetModel(Model *);
    void setModel(ProsogramModel *model);

    virtual void paint(View *v, QPainter &paint, QRect rect) const;

    virtual int getVerticalScaleWidth(View *v, bool, QPainter &) const;
    virtual void paintVerticalScale(View *v, bool, QPainter &paint, QRect rect) const;

    virtual QString getFeatureDescription(View *v, QPoint &) const;

    virtual bool snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const;

    virtual PropertyList getProperties() const;
    virtual QString getPropertyLabel(const PropertyName &) const;
    virtual PropertyType getPropertyType(const PropertyName &) const;
    virtual int getPropertyRangeAndValue(const PropertyName &, int *min, int *max, int *deflt) const;
    virtual QString getPropertyValueLabel(const PropertyName &, int value) const;
    virtual void setProperty(const PropertyName &, int value);
    void setProperties(const QXmlAttributes &attributes);

    virtual bool isLayerScrollable(const View *v) const;
    virtual bool isLayerEditable() const { return false; }
    virtual int getCompletion(View *) const { return 100; }

    virtual bool getValueExtents(double &min, double &max, bool &, QString &) const;
    virtual bool getDisplayExtents(double &min, double &max) const;
    virtual bool setDisplayExtents(double min, double max);

    virtual void toXml(QTextStream &stream, QString indent = "",
                       QString extraAttributes = "") const;


    // VerticalScaleLayer methods
    virtual int getYForValue(View *, double value) const;
    virtual double getValueForY(View *, int y) const;
    virtual QString getScaleUnits() const;

protected:
    void getScaleExtents(View *, double &min, double &max, bool &log) const;
    virtual int getDefaultColourHint(bool dark, bool &impose);

    ProsogramTonalSegmentModel::PointList getLocalTonalSegments(View *v, int x) const;
    void paintAnnotationTier(View *v, QPainter &paint, sv_frame_t frame0, sv_frame_t frame1,
                             int tier_y0, int tier_y1, Qt::PenStyle verticalLinePenstyle,
                             QPointer<AnnotationGridPointModel> model) const;

    ProsogramModel *m_model;
    TimeValueLayer *m_layerPitch;
    TimeValueLayer *m_layerIntensity;

    mutable double m_scaleMinimum;
    mutable double m_scaleMaximum;
};

#endif // PROSOGRAMLAYER_H
