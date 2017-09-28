#ifndef TOKENLIST_H
#define TOKENLIST_H

/*
    DisMo Annotator / Praaline
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
#include <QList>
#include <QTextStream>
// Praaline.Core
#include "annotation/AnnotationTierGroup.h"
#include "annotation/IntervalTier.h"
// DisMo
#include "Token.h"

namespace DisMoAnnotator {

class TokenList : public QList<Token *>
{
public:
    TokenList();
    TokenList(const QString &speakerID);

    QString speakerID() const { return m_speakerID; }

    QString tokenTextToString(const QString separator = QString(" ")) const;
    QString tokenTextToString(const QString separatorBefore, const QString separatorAfter) const;

    // Serialisation from and to tables
    void readFromTable(const QStringList &input);
    void readFromTableFile(const QString &filename);
    QStringList writeToTable() const;
    void writeToTableFile(const QString &filename) const;

    // Serialisation from and to textgrids
    void readFromTextGrid(const AnnotationTierGroup &txg);
    void readFromTiers(const IntervalTier *tierTokMin, const IntervalTier *tierPOSMin,
                       const IntervalTier *tierTokMWU, const IntervalTier *tierPOSMWU,
                       const IntervalTier *tierDiscourse, const IntervalTier *tierDisfluency,
                       const IntervalTier *tierBoundary);
    void writeToTextGrid(AnnotationTierGroup &txg, const QString &tierNamePrefix = "dismo-",
                         bool outputTokMin = true, bool outputPOSMin = true,
                         bool outputTokMWU = true, bool outputPOSMWU = true,
                         bool outputDiscourse = true, bool outputDisfluency = true,
                         bool outputBoundary = true, int tierIndex = -1) const;
    void writeToTiers(IntervalTier *tierTokMin, IntervalTier *tierPOSMin,
                      IntervalTier *tierTokMWU, IntervalTier *tierPOSMWU,
                      IntervalTier *tierDiscourse, IntervalTier *tierDisfluency,
                      IntervalTier *tierBoundary) const;
    void updateTokenizedTiers(IntervalTier *tierTokMin, IntervalTier *tierTokMWU,
                              const QHash<QString, QString> &attributes, bool keepOnlyMin = false);

    void mergeTokens(int from, int to);

private:
    QString m_speakerID;
};

}

#endif // TOKENLIST_H
