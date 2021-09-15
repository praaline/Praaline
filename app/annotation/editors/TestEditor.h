#ifndef TESTEDITOR_H
#define TESTEDITOR_H

#include <QWidget>

namespace Ui {
class TestEditor;
}

class TestEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TestEditor(QWidget *parent = nullptr);
    ~TestEditor();

private:
    Ui::TestEditor *ui;
};

#endif // TESTEDITOR_H
