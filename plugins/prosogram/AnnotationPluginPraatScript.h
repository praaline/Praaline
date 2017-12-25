#ifndef ANNOTATIONPLUGINPRAATSCRIPT_H
#define ANNOTATIONPLUGINPRAATSCRIPT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>

class AnnotationPluginPraatScript : public QObject
{
    Q_OBJECT
public:
    explicit AnnotationPluginPraatScript(QObject *parent = 0);
    virtual ~AnnotationPluginPraatScript();

signals:
    void logOutput(const QString &output);
    void finished(int exitcode);

public slots:

protected:
    void executePraatScript(QString script, QStringList scriptArguments);
    void readyRead();
    void processFinished(int exitcode);
    QProcess *m_process;

};

#endif // ANNOTATIONPLUGINPRAATSCRIPT_H
