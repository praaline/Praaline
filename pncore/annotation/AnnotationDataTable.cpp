#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QVariant>
#include <QHash>
#include <QFile>
#include <QTextStream>
#include "base/RealTime.h"
#include "base/RealValueList.h"
#include "annotation/AnnotationDataTable.h"

namespace Praaline {
namespace Core {

AnnotationDataTable::AnnotationDataTable(QObject *parent) :
    QObject(parent)
{
}

AnnotationDataTable::~AnnotationDataTable()
{
}

// ==========================================================================================================
// Table dimensions
// ==========================================================================================================

int AnnotationDataTable::getRowCount() const
{
    return m_data.count();
}

int AnnotationDataTable::getColumnCount() const
{
    return m_fieldNames.count();
}

// ==========================================================================================================
// Field descriptors
// ==========================================================================================================

QStringList AnnotationDataTable::getFieldNames() const
{
    return m_fieldNames.keys();
}
QStringList AnnotationDataTable::getFieldTypes() const
{
    return m_fieldTypes.values();
}
QString AnnotationDataTable::getFieldType(const QString &fieldname) const
{
    return m_fieldTypes.value(fieldname, "Unknown");
}
QStringList AnnotationDataTable::getFieldDescriptions() const
{
    return m_fieldDescriptions.values();
}
QString AnnotationDataTable::getFieldDescription(const QString &fieldname) const
{
    return m_fieldDescriptions.value(fieldname, "Unknown");
}

// ==========================================================================================================
// Data
// ==========================================================================================================

void AnnotationDataTable::clear()
{
    m_fieldNames.clear();
    m_fieldTypes.clear();
    m_fieldDescriptions.clear();
    m_data.clear();
}

QVariant AnnotationDataTable::getData(int row, int column)
{
    if (row < 0 || row >= m_data.count()) return QVariant();
    if (column < 0 || column >= m_fieldNames.count()) return QVariant();
    return m_data.at(row).at(column);
}

QVariant AnnotationDataTable::getData(int row, const QString &fieldname)
{
    return getData(row, m_fieldNames.value(fieldname, -1));
}

RealTime AnnotationDataTable::getSeconds(int row, const QString &fieldname)
{
    QVariant v = getData(row, m_fieldNames.value(fieldname, -1));
    RealTime r = RealTime::fromSeconds(v.toDouble());
    return r;
}

RealValueList AnnotationDataTable::getValueList(const QString &fieldname)
{
    RealValueList ret;
    for (int row = 0; row < m_data.count(); row++) {
        ret.append(getData(row, fieldname).toDouble());
    }
    return ret;
}

// ==========================================================================================================
// Serialization
// ==========================================================================================================

bool AnnotationDataTable::readFromFile(const QString &filename)
{
    // Read tab-separated file
    QFile file(filename);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text ))
        return false;
    QTextStream stream(&file);
    bool headerLine = true;
    do {
        QString line = stream.readLine();
        if (headerLine) {
            int fieldColumn = 0;
            foreach (QString fieldName, line.split("\t")) {
                m_fieldNames.insert(fieldName.trimmed(), fieldColumn);
                fieldColumn++;
            }
            headerLine = false;
        }
        else {
            QList<QVariant> record;
            foreach (QString value, line.split("\t")) {
                record.append(QVariant(value));
            }
            m_data.append(record);
        }
    } while (!stream.atEnd());
    file.close();
    return true;
}

bool AnnotationDataTable::saveToFile(const QString &filename)
{
    Q_UNUSED(filename)
    return true;
}

} // namespace Core
} // namespace Praaline
