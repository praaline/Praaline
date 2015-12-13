#ifndef ATTRIBUTENAMETRANSLATION_H
#define ATTRIBUTENAMETRANSLATION_H

#include <QObject>
#include <QString>
#include <QHash>

class AttributeNameTranslation : public QObject
{
    Q_OBJECT
public:
    explicit AttributeNameTranslation(QObject *parent = 0);
    bool readFromFile(const QString &filename);
    QString translate(const QString &attribute) const;

signals:

public slots:

private:
    QHash<QString, QString> m_translation;
};

#endif // ATTRIBUTENAMETRANSLATION_H
