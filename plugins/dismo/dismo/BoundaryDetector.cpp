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

#include "BoundaryDetector.h"
using namespace DisMoAnnotator;

void BoundaryDetector::process()
{
    // Il faut regarder pas simplement les tokens mais le suite des disfluece
    // ajouter un combinaison de disfluences
    foreach(Token *token, m_tokens) {
        if (token->getTagDiscourse() == "MD")
            token->setTagFrontier("MAJ", 1.0);
        else if (token->getTagPOS() == "INTROD") {
            token->setTagDiscourse("CON", 1.0);
            token->setTagFrontier("MINg", 1.0);
        }
        else if (token->getTagPOS() == "CON:coo")
            token->setTagFrontier("MINg", 1.0);
        else if (token->getTagPOS() == "CON:sub")
            token->setTagFrontier("MINg", 1.0);
        else if (token->isPause() && token->duration().toDouble() > 0.250)
            token->setTagFrontier("MAJ", 1.0);
        else if (token->isPause() && token->duration().toDouble() <= 0.250)
            token->setTagFrontier("MIN", 1.0);
        else if (token->getTagDisfluency() == "FIL" && token->duration().toDouble() > 0.250)
            token->setTagFrontier("MAJ", 1.0);
        else
            token->setTagFrontier("", 1.0);
    }
}
