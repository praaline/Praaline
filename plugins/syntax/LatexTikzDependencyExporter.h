#ifndef LATEXTIKZDEPENDENCYEXPORTER_H
#define LATEXTIKZDEPENDENCYEXPORTER_H

#include <QObject>
#include <QString>

namespace Praaline {
namespace Core {
class CorpusCommunication;
class IntervalTier;
}
}

struct LatexTikzDependencyExporterData;

class LatexTikzDependencyExporter : public QObject
{
    Q_OBJECT
public:
    explicit LatexTikzDependencyExporter(QObject *parent = nullptr);
    virtual ~LatexTikzDependencyExporter();

    QString process(Praaline::Core::CorpusCommunication *com);

signals:

protected:
    QString codeTikzDependencyForSentence(Praaline::Core::IntervalTier *tier_tokens, int tokenIndexFrom, int tokenIndexTo);


private:
    LatexTikzDependencyExporterData *d;

};

#endif // LATEXTIKZDEPENDENCYEXPORTER_H
