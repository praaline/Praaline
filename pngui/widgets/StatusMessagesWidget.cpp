#include <QObject>
#include <QTextEdit>
#include <QAction>
#include <QToolBar>
#include <QFont>
#include <QFontDatabase>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QGridLayout>
#include "StatusMessagesWidget.h"

struct StatusMessagesWidgetData {
    StatusMessagesWidgetData() :
        gridLayoutOutput(0), toolbarOutput(0), actionSaveOutput(0), actionClearOutput(0), textResults(0)
    {}

    QGridLayout *gridLayoutOutput;
    QToolBar *toolbarOutput;
    QAction *actionSaveOutput;
    QAction *actionClearOutput;
    QTextEdit *textResults;
};

StatusMessagesWidget::StatusMessagesWidget(QWidget *parent) :
    QWidget(parent), d(new StatusMessagesWidgetData)
{
    d->textResults = new QTextEdit(this);
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    d->textResults->setFont(fixedFont);

    d->gridLayoutOutput = new QGridLayout(this);
    d->gridLayoutOutput->setSpacing(0);
    d->gridLayoutOutput->setObjectName(QStringLiteral("gridLayoutMessages"));
    d->gridLayoutOutput->setContentsMargins(0, 0, 0, 0);

    d->toolbarOutput = new QToolBar(this);
    d->toolbarOutput->setObjectName(QStringLiteral("toolbarOutput"));
    d->toolbarOutput->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarOutput->setIconSize(QSize(16, 16));
    d->toolbarOutput->setToolButtonStyle(Qt::ToolButtonIconOnly);
    d->toolbarOutput->setFloatable(false);

    d->gridLayoutOutput->addWidget(d->toolbarOutput, 0, 0, 1, 1);
    d->gridLayoutOutput->addWidget(d->textResults, 1, 0, 1, 1);

    // Setup actions
    d->actionSaveOutput = d->toolbarOutput->addAction(QIcon(":/icons/actions/action_save.png"), tr("Save output"));
    connect(d->actionSaveOutput, SIGNAL(triggered()), this, SLOT(actionSaveOutput()));
    d->actionClearOutput = d->toolbarOutput->addAction(QIcon(":/icons/actions/clear.png"), tr("Clear output"));
    connect(d->actionClearOutput, SIGNAL(triggered()), this, SLOT(actionClearOutput()));
}

StatusMessagesWidget::~StatusMessagesWidget()
{
    delete d;
}

void StatusMessagesWidget::actionSaveOutput()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Messages Output"),
                                tr("praaline_report.txt"), tr("Text File (*.txt);;All Files (*)"),
                                &selectedFilter, options);
    if (fileName.isEmpty()) return;
    QFile fileOut(fileName);
    if (! fileOut.open(QFile::WriteOnly | QFile::Text)) return;
    QTextStream out(&fileOut);
    out.setCodec("UTF-8");
    out << d->textResults->document()->toPlainText();
    fileOut.close();
}

void StatusMessagesWidget::actionClearOutput()
{
    d->textResults->clear();
}

void StatusMessagesWidget::setToolbarVisible(bool visible)
{
    d->toolbarOutput->setVisible(visible);
}

void StatusMessagesWidget::appendMessage(const QString &message)
{
    d->textResults->append(message);
    d->textResults->moveCursor(QTextCursor::End);
}
