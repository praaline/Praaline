#ifndef CORPUSSPEAKER_H
#define CORPUSSPEAKER_H

#include "pncore_global.h"
#include <QObject>
#include <QString>
#include "corpusobject.h"

class Corpus;

class PRAALINE_CORE_SHARED_EXPORT CorpusSpeaker : public CorpusObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    explicit CorpusSpeaker(QObject *parent = 0);
    CorpusSpeaker(const QString &ID, QObject *parent = 0);
    CorpusSpeaker(CorpusSpeaker *other, QObject *parent = 0);

    CorpusObject::Type type() const { return CorpusObject::Type_Speaker; }

    QPointer<Corpus> corpus() const;
    QString basePath() const;
    QString baseMediaPath() const;

    QString name() const { return m_name; }
    void setName(const QString &name);

signals:
    
public slots:

private:
    QString m_name;
    
};

#endif // CORPUSSPEAKER_H
