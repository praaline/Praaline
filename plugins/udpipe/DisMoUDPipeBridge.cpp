#include "DisMoUDPipeBridge.h"

struct DisMoUDPipeBridgeData {
    int i;
};

DisMoUDPipeBridge::DisMoUDPipeBridge() :
    d(new DisMoUDPipeBridgeData())
{
}

DisMoUDPipeBridge::~DisMoUDPipeBridge()
{
    delete d;
}
