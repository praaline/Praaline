#include <QString>
#include <QList>
#include <QHash>
#include <QFile>
#include <QTextStream>
#include "attributenametranslation.h"

AttributeNameTranslation::AttributeNameTranslation(QObject *parent) :
    QObject(parent)
{
}

bool AttributeNameTranslation::readFromFile(const QString &filename)
{
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        return false;
    QString line;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    do {
        line = stream.readLine();
        m_translation[line.section('\t', 0, 0)] = line.section('\t', 1, 1);
    }
    while (!stream.atEnd());
    file.close();
    return true;
}

QString AttributeNameTranslation::translate(const QString &attribute) const
{
    if (m_translation.contains(attribute))
        return m_translation.value(attribute);
    else
        return attribute;
}

//SELECT * FROM syll INNER JOIN speaker ON syll.AnnotationID=speaker.AnnotationID AND speaker.xMin <= syll.xMin AND syll.xMax <= speaker.xMax
