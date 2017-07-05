#include <QString>
#include <QList>
#include <QStringList>

#include "ForcedAligner.h"

ForcedAligner::ForcedAligner()
{

}

void ForcedAligner::run(const QString &filenameWave, const QString &filenameTextgrid)
{
    QString tiernameTranscription = "ortho";
    QString tiernamePhonetisation = "phono";

    QStringList charactersToIgnore;
    charactersToIgnore << "}" << "-" << "'" << ";" << "(" << ")" << "," << ".";

    bool preciseTranscriptionBoundaries = true;
    bool considerStar = true;
    bool considerElision = true;

    QString bsil;
    if (preciseTranscriptionBoundaries) {
        bsil ="-b sil";
    } else {
        bsil = "";
    }

    int n_utt_rec=0;
    int n_utt_notrec=0;
    int n_utt_ignored=0;
    int n_utt_misformatted=0;
    int n_utt_silence=0;

    int htkSamplingRate = 16000;
    QString htkParameters = "MFCC_0_D_A";


}
