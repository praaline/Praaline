#include <QWidget>
#include <QGridLayout>

#include "nodeeditor/NodeData.h"
#include "nodeeditor/FlowScene.h"
#include "nodeeditor/FlowView.h"
#include "nodeeditor/DataModelRegistry.h"
#include "nodeeditor/NodeStyle.h"
#include "nodeeditor/FlowViewStyle.h"
#include "nodeeditor/ConnectionStyle.h"

#include "DatasetEditorModels.h"
#include "DatasetEditorWidget.h"

struct DatasetEditorWidgetData {
    FlowScene *scene;
    FlowView *view;
};

DatasetEditorWidget::DatasetEditorWidget(QWidget *parent) :
    QWidget(parent), d(new DatasetEditorWidgetData)
{
    setStyle();
    d->scene = new FlowScene(registerDataModels());
    d->view = new FlowView(d->scene);
    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(d->view);
    layout->setMargin(0);
    this->setLayout(layout);
}

DatasetEditorWidget::~DatasetEditorWidget()
{
    delete d;
}

// static
std::shared_ptr<DataModelRegistry> DatasetEditorWidget::registerDataModels()
{
    auto ret = std::make_shared<DataModelRegistry>();
    ret->registerModel<MyDataModel>();
    return ret;
}

// static
void DatasetEditorWidget::setStyle()
{
    FlowViewStyle::setStyle(
                R"(
                {
                    "FlowViewStyle": {
                        "BackgroundColor": [255, 255, 240],
                        "FineGridColor": [245, 245, 230],
                        "CoarseGridColor": [235, 235, 220]
                    }
                }
                )");
    NodeStyle::setNodeStyle(
                R"(
                {
                    "NodeStyle": {
                        "NormalBoundaryColor": "darkgray",
                        "SelectedBoundaryColor": "deepskyblue",
                        "GradientColor0": "mintcream",
                        "GradientColor1": "mintcream",
                        "GradientColor2": "mintcream",
                        "GradientColor3": "mintcream",
                        "ShadowColor": [200, 200, 200],
                        "FontColor": [10, 10, 10],
                        "FontColorFaded": [100, 100, 100],
                        "ConnectionPointColor": "white",
                        "PenWidth": 2.0,
                        "HoveredPenWidth": 2.5,
                        "ConnectionPointDiameter": 10.0,
                        "Opacity": 1.0
                    }
                }
                )");
    ConnectionStyle::setConnectionStyle(
                R"(
                {
                    "ConnectionStyle": {
                        "ConstructionColor": "gray",
                        "NormalColor": "black",
                        "SelectedColor": "gray",
                        "SelectedHaloColor": "deepskyblue",
                        "HoveredColor": "deepskyblue",

                        "LineWidth": 3.0,
                        "ConstructionLineWidth": 2.0,
                        "PointDiameter": 10.0,

                        "UseDataDefinedColors": false
                    }
                }
                )");
}

