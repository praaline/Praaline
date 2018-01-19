#ifndef ANNOTATIONATTRIBUTENODEDATA_H
#define ANNOTATIONATTRIBUTENODEDATA_H

#include "pngui/nodeeditor/NodeDataModel.h"

/// Encapsulates data about Annotation Attributes that needs to transferred
/// within the Node Editor graph.
class AnnotationAttributeNodeData : public NodeData
{
public:
    AnnotationAttributeNodeData() :
        m_ID(QString()), m_name(QString())
    {}
    AnnotationAttributeNodeData(const QString &ID, const QString &name) :
        m_ID(ID), m_name(name)
    {}

    NodeDataType type() const override {
        return NodeDataType { m_ID, m_name };
    }

    QString ID() const {
        return m_ID;
    }

    QString name() const {
        return m_name;
    }

private:
    QString m_ID;
    QString m_name;
};

#endif // ANNOTATIONATTRIBUTENODEDATA_H
