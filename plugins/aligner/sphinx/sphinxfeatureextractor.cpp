#include <QDebug>
#include <QString>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextStream>
#include <QCoreApplication>

#include "pncore/corpus/corpus.h"

#include "sphinxfeatureextractor.h"

SphinxFeatureExtractor::SphinxFeatureExtractor(QObject *parent) :
    QObject(parent), m_filenameSphinxFeatParams("feat.params"), m_sampleRate(16000)
{

}

SphinxFeatureExtractor::~SphinxFeatureExtractor()
{

}

void SphinxFeatureExtractor::setFeatureParametersFile(const QString &filename)
{
    m_filenameSphinxFeatParams = filename;
}

void SphinxFeatureExtractor::setSampleRate(quint64 samplerate)
{
    m_sampleRate = samplerate;
}

bool SphinxFeatureExtractor::createSphinxMFC(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    // List WAV files whose features will be extracted in a fileids temporary file
    // Get a temporary file and write out the utterances
    QString filenameCtl;
    QTemporaryFile fileCtl;
    if (!fileCtl.open()) return false;
    filenameCtl = fileCtl.fileName();
    QTextStream ctl(&fileCtl);
    ctl.setCodec("ISO 8859-1");
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            if (!rec) continue;
            ctl << QString(rec->filename()).replace(".wav", "") << "\n";
        }
    }
    fileCtl.close();
    // Launch Sphinx feature extraction
    // sphinx_fe -argfile french_f0/feat.params -samprate 16000 -c anais_participant01a.fileids -di . -do . -ei wav -eo mfc -mswav yes
    QString appPath = QCoreApplication::applicationDirPath();
    QString sphinxPath = appPath + "/plugins/aligner/sphinx/";
    QProcess sphinxFE;
    sphinxFE.setWorkingDirectory(sphinxPath);
    QStringList sphinxFEparams;
    sphinxFEparams << "-argfile" << m_filenameSphinxFeatParams <<
                      "-samprate" << QString("%1").arg(m_sampleRate) <<
                      "-c" << filenameCtl <<
                      "-di" << corpus->baseMediaPath() <<
                      "-do" << corpus->baseMediaPath() <<
                      "-ei" << "16k.wav" <<
                      "-eo" << "mfc" <<
                      "-mswav" << "yes";
    sphinxFE.start(sphinxPath + "sphinx_fe", sphinxFEparams);
    if (!sphinxFE.waitForStarted(-1)) return false;
    if (!sphinxFE.waitForFinished(-1)) return false;
    QString out = QString(sphinxFE.readAllStandardOutput());
    qDebug() << out;
    QString err = QString(sphinxFE.readAllStandardError());
    qDebug() << err;
    return true;
}
