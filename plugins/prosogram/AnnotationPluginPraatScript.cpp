#include <QObject>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDebug>

#include "AnnotationPluginPraatScript.h"

AnnotationPluginPraatScript::AnnotationPluginPraatScript(QObject *parent) :
    QObject(parent)
{
    m_process = nullptr;
}

AnnotationPluginPraatScript::~AnnotationPluginPraatScript()
{
    if (m_process) delete m_process;
}

void AnnotationPluginPraatScript::executePraatScript(QString script, QStringList scriptArguments)
{
    // Execute prosogram through praatcon
    if (m_process) delete m_process;
    m_process = new QProcess(this);
    m_process->setReadChannel(QProcess::StandardOutput);
    //connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readyRead()));
    //connect(m_process, SIGNAL(finished(int)), this, SLOT(processFinished(int)));
    QString praatCommand;
#ifdef Q_OS_WIN
    praatCommand = QCoreApplication::applicationDirPath() + "/tools/Praat.exe";
    if (!QFile::exists(praatCommand)) {
        praatCommand = QDir::homePath() + "/Praaline/tools/Praat.exe";
    }
#else
#ifdef Q_OS_MAC
    praatCommand = "/Applications/Praat.app/Contents/MacOS/Praat";
#else
    praatCommand = "/usr/bin/praat";
#endif
#endif
    emit logOutput("Praat script: " + script + " "+ scriptArguments.join(" "));
    // QStringList args;
    //foreach (QString arg, scriptArguments) args << QString("\"").append(arg).append("\"");
    m_process->start(praatCommand, QStringList() << "--run" << script << scriptArguments);
    if (!m_process->waitForStarted(-1)) {
        emit logOutput("Error: " + m_process->errorString());
        return;
    }
    if (!m_process->waitForFinished(-1)) {
        qDebug() << m_process->errorString();
        emit logOutput("Error: " + m_process->errorString());
        return;
    }
    QByteArray result = m_process->readAllStandardOutput();
    emit logOutput(QString(result));
    qDebug() << QString(result);
    emit finished(m_process->exitCode());
    if (m_process) {
        m_process->close();
        delete m_process;
        m_process = nullptr;
    }
}

void AnnotationPluginPraatScript::readyRead()
{
    QByteArray result = m_process->readAllStandardOutput();
    emit logOutput(QString(result));
}

void AnnotationPluginPraatScript::processFinished(int exitcode)
{
    QByteArray result = m_process->readAllStandardOutput();
    emit logOutput(QString(result));
    emit finished(exitcode);
}
