#ifndef DISMOTRAINER_H
#define DISMOTRAINER_H

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
#include <QStringList>
#include "TokenList.h"

namespace DisMoAnnotator {

class DismoTrainer
{
public:
    enum DismoTrainerModel {
        POSMin,
        Disfluency,
        Discourse,
        POSFin,
        MWUList,
        CorrDict,
        All
    };

    DismoTrainer(const QString modelsPath = QString());
    ~DismoTrainer();
    // load training corpus
    void loadTableFiles(const QStringList fileNames);
    void loadTextgrids(const QStringList fileNames);
    void train(const QString &filenameBase = QString(), const DismoTrainerModel model = DismoTrainer::All);

private:
    void prepareTrainingFiles(const QString &filenameBase, const QString &modelname);
    void outputTrainingFile(const QString &modelname, const int indexTL, QTextStream &out);

    QString m_modelsPath;
    QList<TokenList *> m_tokenLists;
};

}

#endif // DISMOTRAINER_H
