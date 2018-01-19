#ifndef ANNOTATIONLEVELDATAMODEL_H
#define ANNOTATIONLEVELDATAMODEL_H

#include <QObject>
#include "nodeeditor/NodeData.h"
#include "nodeeditor/NodeDataModel.h"
#include <memory>

namespace Praaline {
namespace Core {
class AnnotationStructureLevel;
}
}

struct AnnotationLevelDataModelData;

/// The model dictates the number of inputs and outputs for the Node.
class AnnotationLevelDataModel : public NodeDataModel
{
    Q_OBJECT
public:
    AnnotationLevelDataModel(const QString &levelID);
    virtual ~AnnotationLevelDataModel();

    void setLevel(Praaline::Core::AnnotationStructureLevel *level);

public:
    QString caption() const override;
    QString name() const override;
    std::unique_ptr<NodeDataModel> clone() const override;
    void save(Properties &p) const override;
    unsigned int nPorts(PortType) const override;
    NodeDataType dataType(PortType, PortIndex) const override;
    std::shared_ptr<NodeData> outData(PortIndex) override;
    void setInData(std::shared_ptr<NodeData>, int) override;
    QWidget * embeddedWidget() override { return nullptr; }

private:
    AnnotationLevelDataModelData *d;
};

#endif // ANNOTATIONLEVELDATAMODEL_H
