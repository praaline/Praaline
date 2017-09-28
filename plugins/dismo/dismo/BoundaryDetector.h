#ifndef BOUNDARYDETECTOR_H
#define BOUNDARYDETECTOR_H

/*
    DisMo Annotator
    Copyright (c) 2012-2014 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include <QString>
#include "TokenList.h"

namespace DisMoAnnotator {

class BoundaryDetector
{
public:
    BoundaryDetector(TokenList &tokens) : m_tokens(tokens) {}
    void process();
private:
    // state
    TokenList &m_tokens;
};

}

#endif // BOUNDARYDETECTOR_H
