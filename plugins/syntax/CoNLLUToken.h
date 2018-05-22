#ifndef CONLLUTOKEN_H
#define CONLLUTOKEN_H

struct CoNLLUTokenData;

class CoNLLUToken
{
public:
    CoNLLUToken();
    ~CoNLLUToken();

private:
    CoNLLUTokenData *d;
};

#endif // CONLLUTOKEN_H
