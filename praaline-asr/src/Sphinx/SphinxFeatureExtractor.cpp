#include <QDebug>
#include <QString>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextStream>
#include <QCoreApplication>

#include "PraalineASR/Sphinx/SphinxConfiguration.h"
#include "PraalineASR/Sphinx/SphinxFeatureExtractor.h"

namespace Praaline {
namespace ASR {

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

bool SphinxFeatureExtractor::batchCreateSphinxMFC(QStringList filenamesWave16k)
{
    // List WAV files whose features will be extracted in a fileids temporary file
    // Get a temporary file and write out the utterances
    QString filenameCtl;
    QTemporaryFile fileCtl;
    if (!fileCtl.open()) return false;
    filenameCtl = fileCtl.fileName();
    QTextStream ctl(&fileCtl);
    ctl.setCodec("ISO 8859-1");
    foreach (QString filename, filenamesWave16k) {
        if (!QFile::exists(filename)) continue;
        QString s = QString(filename).replace(".wav", "");
        qDebug() << s;
        ctl << s << "\n";
    }
    fileCtl.close();
    // Launch Sphinx feature extraction
    // sphinx_fe -argfile french_f0/feat.params -samprate 16000 -c inputfile.fileids -di . -do . -ei wav -eo mfc -mswav yes
    QString sphinxPath = SphinxConfiguration::sphinxPath();
    QProcess sphinxFE;
    sphinxFE.setWorkingDirectory(sphinxPath);
    QStringList sphinxFEparams;
    // It is very important NOT to remove silence (de-activate VAD) because otherwise there will be missing frames in the
    // output and it will be impossible to correlate time instants from the original recording with frames.
    sphinxFEparams << "-argfile" << m_filenameSphinxFeatParams <<
                      "-samprate" << QString("%1").arg(m_sampleRate) <<
                      "-c" << filenameCtl <<
                      // "-di" << basePath <<
                      // "-do" << basePath <<
                      "-ei" << "wav" <<
                      "-eo" << "mfc" <<
                      "-mswav" << "yes" <<
                      "-remove_silence" << "no";
                      // << "-ofmt" << "text";
    qDebug() << sphinxFEparams;
    sphinxFE.start(sphinxPath + "sphinx_fe", sphinxFEparams);
    if (!sphinxFE.waitForStarted(-1)) return false;
    if (!sphinxFE.waitForFinished(-1)) return false;
    QString out = QString(sphinxFE.readAllStandardOutput());
    qDebug() << out;
    QString err = QString(sphinxFE.readAllStandardError());
    qDebug() << err;
    return true;
}

} // namespace ASR
} // namespace Praaline
