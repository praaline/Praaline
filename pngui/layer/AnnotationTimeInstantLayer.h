#ifndef ANNOTATIONTIMEINSTANTLAYER_H
#define ANNOTATIONTIMEINSTANTLAYER_H

#include <QObject>
#include <QColor>

class View;
class QPainter;

#include "svgui/layer/SingleColourLayer.h"
#include "pngui/model/visualiser/AnnotationGridModel.h"


//class AnnotationTimeInstantLayer : public SingleColourLayer
//{
//    Q_OBJECT
//public:
//    explicit AnnotationTimeInstantLayer(QObject *parent = nullptr);
//    virtual ~AnnotationTimeInstantLayer();

//    std::string getType() const { return "AnnotationTimeInstants"; }

//    virtual const Model *getModel() const { return m_model; }
//    virtual bool trySetModel(Model *);
//    void setModel(AnnotationGridModel *model);
//    static Model *createEmptyModel(Model *baseModel);

//    virtual void paint(View *v, QPainter &paint, QRect rect) const;

//    virtual QString getLabelPreceding(sv_frame_t) const;
//    virtual QString getFeatureDescription(View *v, QPoint &) const;

//    virtual bool snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y = 0) const;

//    virtual void drawStart(View *v, QMouseEvent *);
//    virtual void drawDrag(View *v, QMouseEvent *);
//    virtual void drawEnd(View *v, QMouseEvent *);

//    virtual void eraseStart(View *v, QMouseEvent *);
//    virtual void eraseDrag(View *v, QMouseEvent *);
//    virtual void eraseEnd(View *v, QMouseEvent *);

//    virtual void editStart(View *v, QMouseEvent *);
//    virtual void editDrag(View *v, QMouseEvent *);
//    virtual void editEnd(View *v, QMouseEvent *);

//    virtual bool editOpen(View *, QMouseEvent *);

//    virtual void moveSelection(Selection s, sv_frame_t newStartFrame);
//    virtual void resizeSelection(Selection s, Selection newSize);
//    virtual void deleteSelection(Selection s);

//    virtual void copy(View *v, Selection s, Clipboard &to);
//    virtual bool paste(View *v, const Clipboard &from, sv_frame_t frameOffset, bool interactive);

//    virtual PropertyList getProperties() const;
//    virtual QString getPropertyLabel(const PropertyName &) const;
//    virtual PropertyType getPropertyType(const PropertyName &) const;
//    virtual int getPropertyRangeAndValue(const PropertyName &, int *min, int *max, int *deflt) const;
//    virtual QString getPropertyValueLabel(const PropertyName &, int value) const;
//    virtual void setProperty(const PropertyName &, int value);

//    enum PlotStyle {
//        PlotInstants,
//        PlotSegmentation
//    };

//    void setPlotStyle(PlotStyle style);
//    PlotStyle getPlotStyle() const { return m_plotStyle; }

//    virtual bool isLayerScrollable(const View *v) const;

//    virtual bool isLayerEditable() const { return true; }

//    virtual int getCompletion(View *) const { return m_model->getCompletion(); }

//    virtual bool needsTextLabelHeight() const { return m_model->hasTextLabels(); }

//    virtual bool getValueExtents(double &, double &, bool &, QString &) const {
//        return false;
//    }

//    virtual void toXml(QTextStream &stream, QString indent = "", QString extraAttributes = "") const;

//    void setProperties(const QXmlAttributes &attributes);

//    virtual ColourSignificance getLayerColourSignificance() const {
//        if (m_plotStyle == PlotSegmentation) {
//            return ColourHasMeaningfulValue;
//        } else {
//            return ColourDistinguishes;
//        }
//    }

//    virtual int getVerticalScaleWidth(View *, bool, QPainter &) const { return 0; }

//protected:
//    AnnotationGridPointModel::PointList getLocalPoints(View *v, int) const;

//    virtual int getDefaultColourHint(bool dark, bool &impose);

//    bool clipboardAlignmentDiffers(View *v, const Clipboard &) const;

//    AnnotationGridModel *m_model;
//    bool m_editing;
//    AnnotationGridPointModel::Point m_editingPoint;
//    AnnotationGridPointModel::EditCommand *m_editingCommand;
//    PlotStyle m_plotStyle;

//    void finish(AnnotationGridPointModel::EditCommand *command) {
//        UndoableCommand *c = command->finish();
//        if (c) CommandHistory::getInstance()->addCommand(c, false);
//    }
//};

#endif // ANNOTATIONTIMEINSTANTLAYER_H
