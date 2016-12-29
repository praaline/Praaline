#ifndef METADATASTRUCTURESECTION_H
#define METADATASTRUCTURESECTION_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QString>
#include "MetadataStructureAttribute.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT MetadataStructureSection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString ID READ ID WRITE setID)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(int itemOrder READ itemOrder WRITE setItemOrder)

public:
    explicit MetadataStructureSection(QObject *parent = 0);
    MetadataStructureSection(const QString &ID, const QString &name = QString(),
                             const QString &description = QString(), int itemOrder = 0, QObject *parent = 0);
    ~MetadataStructureSection();

    // Data
    QString ID() const { return m_ID; }
    void setID(const QString &id) { m_ID = id; }
    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    QString description() const { return m_description; }
    void setDescription(const QString &description) { m_description = description; }
    int itemOrder() const { return m_itemOrder; }
    void setItemOrder(int itemOrder) { m_itemOrder = itemOrder; }

    // ATTRIBUTES - Accessors
    QPointer<MetadataStructureAttribute> attribute(int index) const;
    QPointer<MetadataStructureAttribute> attribute(const QString &ID) const;
    int attributeIndexByID(const QString &ID) const;
    int attributesCount() const;
    bool hasAttributes() const;
    bool hasAttribute(const QString &ID);
    QStringList attributeIDs() const;
    QList<QPointer<MetadataStructureAttribute> > attributes() const;
    void insertAttribute(int index, MetadataStructureAttribute *attribute);
    void addAttribute(MetadataStructureAttribute *attribute);
    void swapAttribute(int oldIndex, int newIndex);
    void removeAttributeAt(int i);
    void removeAttributeByID(const QString &ID);

signals:

public slots:

protected:
    QString m_ID;           // Section ID (e.g. speaker)
    QString m_name;         // User-friendly name (e.g. Speaker)
    QString m_description;  // Description (e.g. Speakers metadata)
    int m_itemOrder;        // Order for presentation purposes
    QList<QPointer<MetadataStructureAttribute> > m_attributes;
};

} // namespace Core
} // namespace Praaline

#endif // METADATASTRUCTURESECTION_H
