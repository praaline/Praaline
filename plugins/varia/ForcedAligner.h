#ifndef FORCEDALIGNER_H
#define FORCEDALIGNER_H

#include <QString>

class ForcedAligner
{
public:
    ForcedAligner();

    void run(const QString &filenameWave, const QString &filenameTextgrid);

};

#endif // FORCEDALIGNER_H
