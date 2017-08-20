#ifndef BRATSYNTAXANDDISFLUENCIES_H
#define BRATSYNTAXANDDISFLUENCIES_H

#include <QString>
#include <QList>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct BratSyntaxAndDisfluenciesData;

class BratSyntaxAndDisfluencies
{
public:
    BratSyntaxAndDisfluencies();
    ~BratSyntaxAndDisfluencies();

    QString sentenceTier() const;
    void setSentenceTier(const QString &tiername);

    QString getHTML(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    BratSyntaxAndDisfluenciesData *d;
};

#endif // BRATSYNTAXANDDISFLUENCIES_H
