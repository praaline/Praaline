#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QTemporaryFile>
#include <QCoreApplication>
#include <QDebug>

#include "crfannotator.h"

CRFAnnotator::CRFAnnotator()
{
}

CRFAnnotator::~CRFAnnotator()
{
}

void CRFAnnotator::annotate(const QString &filenameModel) {
    annotateFromCRF(filenameModel);
}
void CRFAnnotator::writeTraining(QTextStream &out) {
    writeCRFFile(out, true);
}


void CRFAnnotator::annotateFromCRF(const QString &filenameModel) {
    // Get a temporary file and write out the features
    QString filenameIn, filenameOut;
    QTemporaryFile fileIn, fileOut;

    if (fileIn.open()) {
        filenameIn = fileIn.fileName();
        QTextStream streamIn(&fileIn);
        streamIn.setCodec("UTF-8");
        streamIn.setGenerateByteOrderMark(true);
        writeCRFFile(streamIn, false);
        fileIn.close();
    }

    // Touch the output file, and get its filename
    if (fileOut.open()) {
        filenameOut = fileOut.fileName();
        fileOut.close();
    }

    qDebug() << "IN: " << filenameIn;
    qDebug() << "OUT: " << filenameOut;

    // Pass to CRF decoder
    QProcess decoder;
    // DIRECTORY:
    QString decoderCommand;
#ifdef Q_OS_WIN
    QString appPath = QCoreApplication::applicationDirPath();
    decoderCommand = appPath + "/tools/crf/crf_test";
#endif
#ifdef Q_OS_MAC
    decoderCommand = "/usr/local/bin/crf_test"
#else
    decoderCommand = "/usr/local/bin/crf_test";
#endif
    decoder.start(decoderCommand, QStringList() <<
                  "-m" << filenameModel << "-v" << "2" << "-o" << filenameOut << filenameIn);
    if (!decoder.waitForStarted(-1)) return;
    if (!decoder.waitForFinished(-1)) return;
    // Read responses into token list
    if (fileOut.open()) {
        QTextStream streamOut(&fileOut);
        streamOut.setCodec("UTF-8");
        readCRFDecoding(streamOut);
        fileOut.close();
    }
}

void CRFAnnotator::writeCRFFile(QTextStream &out, bool isTraining) {
    bool endSequence = true;
    for (int i = 0; i < m_numberOfUnits; i++) {
        if (isEndOfSequence(i)) {
            if (!endSequence) {
                out << "\n";
                endSequence = true;
            }
            continue;
        }
        else if (skipUnit(i)) {
            continue;
        }
        endSequence = false;
        writeUnitToCRF(out, i, isTraining);
    }
}

void CRFAnnotator::readCRFDecoding(QTextStream &in) {
    QString line;
    for (int i = 0; i < m_numberOfUnits; i++) {
        if (isEndOfSequence(i)) {
            continue;
        }
        else if (skipUnit(i)) {
            continue;
        }
        line = in.readLine();
        while (line.startsWith("#") || line.isEmpty()) {
            line = in.readLine();
            if (in.atEnd())
                return; // finished
        }
        decodeUnitFromCRF(line, i);
    }
}
