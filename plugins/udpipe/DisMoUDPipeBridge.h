#ifndef DISMOUDPIPEBRIDGE_H
#define DISMOUDPIPEBRIDGE_H

#include <QString>
#include <QList>

struct DisMoUDPipeBridgeData;

class DisMoUDPipeBridge
{
public:
    DisMoUDPipeBridge();
    ~DisMoUDPipeBridge();

private:
    DisMoUDPipeBridgeData *d;
};

#endif // DISMOUDPIPEBRIDGE_H
