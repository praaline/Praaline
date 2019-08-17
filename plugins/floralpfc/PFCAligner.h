#ifndef PFCALIGNER_H
#define PFCALIGNER_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
class Interval;
}
}

struct PFCAlignerData;

class PFCAligner
{
public:
    PFCAligner();
    ~PFCAligner();

    QString align(Praaline::Core::CorpusCommunication *com, const QString &method);
    void dictionaryMFAClose(const QString &filename);
    void setMFAPath(const QString &path);
    void setOutputWaveFiles(bool);
    QString scriptCrossAlignment();
    QString combineDictionaries();

private:
    PFCAlignerData *d;
};

#endif // PFCALIGNER_H
