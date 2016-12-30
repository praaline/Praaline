#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QStatusBar>
#include <QTextStream>
#include <QToolBar>

#include <Qsci/qsciscintilla.h>

#include "ScriptEditorWidget.h"

struct ScriptEditorWidgetData {
    QString currentFile;
    QsciScintilla *textEdit;
    QMenu *fileMenu;
    QMenu *editMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
};

ScriptEditorWidget::ScriptEditorWidget(QWidget *parent) :
    QMainWindow(parent), d(new ScriptEditorWidgetData)
{
    d->textEdit = new QsciScintilla(this);
    setCentralWidget(d->textEdit);

    setupActions();
    setupToolBars();
    setupStatusBar();

    connect(d->textEdit, SIGNAL(textChanged()), this, SLOT(documentWasModified()));

    setCurrentFile("");
}

ScriptEditorWidget::~ScriptEditorWidget()
{
    delete d;
}

void ScriptEditorWidget::setupActions()
{
    d->newAction = new QAction(QIcon(":/icons/actions/action_new.png"), tr("&New"), this);
    d->newAction->setShortcut(tr("Ctrl+N"));
    d->newAction->setStatusTip(tr("Create a new file"));
    connect(d->newAction, SIGNAL(triggered()), this, SLOT(newFile()));

    d->openAction = new QAction(QIcon(":/icons/actions/action_open.png"), tr("&Open..."), this);
    d->openAction->setShortcut(tr("Ctrl+O"));
    d->openAction->setStatusTip(tr("Open an existing file"));
    connect(d->openAction, SIGNAL(triggered()), this, SLOT(open()));

    d->saveAction = new QAction(QIcon(":/icons/actions/action_save.png"), tr("&Save"), this);
    d->saveAction->setShortcut(tr("Ctrl+S"));
    d->saveAction->setStatusTip(tr("Save the document to disk"));
    connect(d->saveAction, SIGNAL(triggered()), this, SLOT(save()));

    d->saveAsAction = new QAction(QIcon(":/icons/actions/action_saveas.png"), tr("Save &As..."), this);
    d->saveAsAction->setStatusTip(tr("Save the document under a new name"));
    connect(d->saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

    d->cutAction = new QAction(QIcon(":/icons/actions/edit_cut.png"), tr("Cu&t"), this);
    d->cutAction->setShortcut(tr("Ctrl+X"));
    d->cutAction->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(d->cutAction, SIGNAL(triggered()), d->textEdit, SLOT(cut()));

    d->copyAction = new QAction(QIcon(":/icons/actions/edit_copy.png"), tr("&Copy"), this);
    d->copyAction->setShortcut(tr("Ctrl+C"));
    d->copyAction->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(d->copyAction, SIGNAL(triggered()), d->textEdit, SLOT(copy()));

    d->pasteAction = new QAction(QIcon(":/icons/actions/edit_paste.png"), tr("&Paste"), this);
    d->pasteAction->setShortcut(tr("Ctrl+V"));
    d->pasteAction->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    connect(d->pasteAction, SIGNAL(triggered()), d->textEdit, SLOT(paste()));

    d->cutAction->setEnabled(false);
    d->copyAction->setEnabled(false);
    connect(d->textEdit, SIGNAL(copyAvailable(bool)), d->cutAction, SLOT(setEnabled(bool)));
    connect(d->textEdit, SIGNAL(copyAvailable(bool)), d->copyAction, SLOT(setEnabled(bool)));
}

void ScriptEditorWidget::setupToolBars()
{
    d->fileToolBar = addToolBar(tr("File"));
    d->fileToolBar->addAction(d->newAction);
    d->fileToolBar->addAction(d->openAction);
    d->fileToolBar->addAction(d->saveAction);
    d->fileToolBar->addAction(d->saveAsAction);

    d->editToolBar = addToolBar(tr("Edit"));
    d->editToolBar->addAction(d->cutAction);
    d->editToolBar->addAction(d->copyAction);
    d->editToolBar->addAction(d->pasteAction);
}

void ScriptEditorWidget::setupStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

// ==============================================================================================================================

bool ScriptEditorWidget::maybeSave()
{
    if (d->textEdit->isModified()) {
        int ret = QMessageBox::warning(this, tr("Praaline Script Editor"),
                     tr("The document has been modified.\nDo you want to save your changes?"),
                     QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
                     QMessageBox::Cancel | QMessageBox::Escape);
        if (ret == QMessageBox::Yes)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void ScriptEditorWidget::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Praaline Script Editor"),
                             tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return;
    }
    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    d->textEdit->setText(in.readAll());
    QApplication::restoreOverrideCursor();
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File %1 loaded").arg(fileName), 2000);
}

bool ScriptEditorWidget::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, tr("Praaline Script Editor"),
                             tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << d->textEdit->text();
    QApplication::restoreOverrideCursor();
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File %1 saved").arg(fileName), 2000);
    return true;
}

void ScriptEditorWidget::setCurrentFile(const QString &fileName)
{
    d->currentFile = fileName;
    d->textEdit->setModified(false);
    setWindowModified(false);

    QString shownName;
    if (d->currentFile.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = QFileInfo(d->currentFile).fileName();

    setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("Praaline Script Editor")));
}

// ==============================================================================================================================

void ScriptEditorWidget::newFile()
{
    if (maybeSave()) {
        d->textEdit->clear();
        setCurrentFile("");
    }
}

void ScriptEditorWidget::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool ScriptEditorWidget::save()
{
    if (d->currentFile.isEmpty())
        return saveAs();
    // else
    return saveFile(d->currentFile);
}

bool ScriptEditorWidget::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty()) return false;
    return saveFile(fileName);
}

void ScriptEditorWidget::documentWasModified()
{
    setWindowModified(d->textEdit->isModified());
}

void ScriptEditorWidget::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

