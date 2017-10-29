#ifndef REGIONAVERAGELAYER_H
#define REGIONAVERAGELAYER_H

#include "pngui/model/visualiser/RegionAverageModel.h"
#include "svgui/layer/SingleColourLayer.h"
#include "svgui/layer/VerticalScaleLayer.h"
#include "svgui/layer/RegionLayer.h"

#include <QObject>
#include <QColor>
#include <QMap>

class View;
class QPainter;

class RegionAverageLayer : public SingleColourLayer, public VerticalScaleLayer
{
    Q_OBJECT
public:
    RegionAverageLayer();
    virtual ~RegionAverageLayer();

    std::string getType() const override { return "Region Average"; }

    virtual const Model *getModel() const override { return m_model; }
    virtual bool trySetModel(Model *) override;
    void setModel(RegionAverageModel *model);

    virtual void paint(View *v, QPainter &paint, QRect rect) const override;

    virtual int getVerticalScaleWidth(View *v, bool, QPainter &) const override;
    virtual void paintVerticalScale(View *v, bool, QPainter &paint, QRect rect) const override;

    virtual QString getFeatureDescription(View *v, QPoint &) const override;

    virtual bool snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const override;

    virtual PropertyList getProperties() const override;
    virtual QString getPropertyLabel(const PropertyName &) const override;
    virtual PropertyType getPropertyType(const PropertyName &) const override;
    virtual int getPropertyRangeAndValue(const PropertyName &, int *min, int *max, int *deflt) const override;
    virtual QString getPropertyValueLabel(const PropertyName &, int value) const override;
    virtual void setProperty(const PropertyName &, int value) override;
    void setProperties(const QXmlAttributes &attributes) override;

    virtual bool isLayerScrollable(const View *v) const override;
    virtual bool isLayerEditable() const override { return false; }
    virtual int getCompletion(View *) const override { return 100; }

    virtual bool getValueExtents(double &min, double &max, bool &, QString &) const override;
    virtual bool getDisplayExtents(double &min, double &max) const override;
    virtual bool setDisplayExtents(double min, double max) override;

    virtual void toXml(QTextStream &stream, QString indent = "", QString extraAttributes = "") const override;

    // Properties
    QString getLevelID() const;
    void setLevelID(QString);
    QString getGroupingLevelID() const;
    void setGroupingLevelID(QString);

    // VerticalScaleLayer methods
    virtual int getYForValue(View *, double value) const override;
    virtual double getValueForY(View *, int y) const override;
    virtual QString getScaleUnits() const override;

protected:
    void getScaleExtents(View *, double &min, double &max, bool &log) const;
    virtual int getDefaultColourHint(bool dark, bool &impose) override;

    RegionAverageModel *m_model;
    QMap<QString, RegionLayer *> m_sublayers;

    void updateSublayers();
};

#endif // REGIONAVERAGELAYER_H
