#include <QObject>
#include "nodeeditor/NodeData.h"
#include "nodeeditor/NodeDataModel.h"
#include <memory>

#include "pncore/structure/AnnotationStructure.h"
using namespace Praaline::Core;

#include "AnnotationAttributeNodeData.h"
#include "AnnotationLevelDataModel.h"

struct AnnotationLevelDataModelData {
    QString levelID;
    QString levelName;
};

AnnotationLevelDataModel::AnnotationLevelDataModel(const QString &levelID) :
    d(new AnnotationLevelDataModelData())
{
    d->levelID = levelID;
}

AnnotationLevelDataModel::~AnnotationLevelDataModel()
{
    delete d;
}

void AnnotationLevelDataModel::setLevel(AnnotationStructureLevel *level)
{
    if (!level) return;
    d->levelID = level->ID();
    d->levelName = level->name();
}

QString AnnotationLevelDataModel::caption() const {
    return d->levelName;
}

QString AnnotationLevelDataModel::name() const {
    return d->levelID;
}

std::unique_ptr<NodeDataModel> AnnotationLevelDataModel::clone() const {
    return std::make_unique<AnnotationLevelDataModel>(d->levelID);
}

void AnnotationLevelDataModel::save(Properties &p) const {
    p.put("model_name", AnnotationLevelDataModel::name());
}

unsigned int AnnotationLevelDataModel::nPorts(PortType) const {
    return 3;
}

NodeDataType AnnotationLevelDataModel::dataType(PortType, PortIndex) const {
    return AnnotationAttributeNodeData().type();
}

std::shared_ptr<NodeData> AnnotationLevelDataModel::outData(PortIndex) {
    return std::make_shared<AnnotationAttributeNodeData>();
}

void AnnotationLevelDataModel::setInData(std::shared_ptr<NodeData>, int) {
    //
}
