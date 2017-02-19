#ifndef CORPUSLEVELSATTRIBUTESSELECTOR_H
#define CORPUSLEVELSATTRIBUTESSELECTOR_H

#include <QWidget>

struct CorpusLevelsAttributesSelectorData;

class CorpusLevelsAttributesSelector : public QWidget
{
    Q_OBJECT

public:
    explicit CorpusLevelsAttributesSelector(QWidget *parent = 0);
    ~CorpusLevelsAttributesSelector();

private:
    CorpusLevelsAttributesSelectorData *d;
};

#endif // CORPUSLEVELSATTRIBUTESSELECTOR_H
