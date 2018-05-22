#include "CoNLLUToken.h"

struct CoNLLUTokenData
{

};

CoNLLUToken::CoNLLUToken() :
    d(new CoNLLUTokenData)
{
}

CoNLLUToken::~CoNLLUToken()
{
    delete d;
}

