#ifndef PROSODICUNITSANNOTATOR_H
#define PROSODICUNITSANNOTATOR_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct ProsodicUnitsAnnotatorData;

class ProsodicUnitsAnnotator
{
public:
    ProsodicUnitsAnnotator();
    ~ProsodicUnitsAnnotator();

    QString levelTokens() const;
    void setLevelTokens(const QString &levelID);

    QString levelSyllables() const;
    void setLevelSyllables(const QString &levelID);

    QString attributeBoundary() const;
    void setAttributeBoundary(const QString &attributeID);

    QString levelProsodicUnit() const;
    void setLevelProsodicUnit(const QString &levelID);

    QString createProsodicUnitsTierBoundaries(Praaline::Core::CorpusCommunication *com, const QString &boundaryLabel,
                                              bool asSequences = false);

private:
    ProsodicUnitsAnnotatorData *d;
};

#endif // PROSODICUNITSANNOTATOR_H
