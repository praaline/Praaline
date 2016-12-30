#ifndef CORPUSLEVELSATTRIBUTESSELECTOR_H
#define CORPUSLEVELSATTRIBUTESSELECTOR_H

#include <QWidget>

namespace Ui {
class CorpusLevelsAttributesSelector;
}

class CorpusLevelsAttributesSelector : public QWidget
{
    Q_OBJECT

public:
    explicit CorpusLevelsAttributesSelector(QWidget *parent = 0);
    ~CorpusLevelsAttributesSelector();

private:
    Ui::CorpusLevelsAttributesSelector *ui;
};

#endif // CORPUSLEVELSATTRIBUTESSELECTOR_H
