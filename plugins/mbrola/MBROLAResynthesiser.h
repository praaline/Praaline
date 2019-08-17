#ifndef MBROLARESYNTHESISER_H
#define MBROLARESYNTHESISER_H

#include <QObject>
#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class MBROLAResynthesiser : public QObject
{
    Q_OBJECT
public:
    explicit MBROLAResynthesiser(QObject *parent = nullptr);
    static QString createPhoFile(const QString &filenamePho, Praaline::Core::CorpusCommunication *com);
    static QString resynthesise(const QString &directory, Praaline::Core::CorpusCommunication *com);

signals:

public slots:
};

#endif // MBROLARESYNTHESISER_H
