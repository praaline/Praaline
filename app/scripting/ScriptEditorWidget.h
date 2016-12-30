#ifndef SCRIPTEDITORWIDGET_H
#define SCRIPTEDITORWIDGET_H

#include <QMainWindow>
class QAction;
class QMenu;
class QsciScintilla;

struct ScriptEditorWidgetData;

class ScriptEditorWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit ScriptEditorWidget(QWidget *parent = 0);
    ~ScriptEditorWidget();

protected:
    void closeEvent(QCloseEvent *event);

signals:

public slots:

protected slots:
    virtual void newFile();
    virtual void open();
    virtual bool save();
    virtual bool saveAs();
    virtual void documentWasModified();

private:
    ScriptEditorWidgetData *d;

    void setupActions();
    void setupToolBars();
    void setupStatusBar();

    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
};

#endif // SCRIPTEDITORWIDGET_H
