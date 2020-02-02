#include <QString>
#include <QList>
#include <QFile>
#include <QDir>
#include <QProcess>

#include "PraalineCore/Annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "PraalineASR/ForcedAligner.h"
#include "PraalineASR/Kaldi/KaldiForcedAligner.h"

namespace Praaline {
namespace ASR {

struct KaldiForcedAlignerData {
    int i;
};

KaldiForcedAligner::KaldiForcedAligner(QObject *parent) :
    ForcedAligner(parent), d(new KaldiForcedAlignerData())
{
}

KaldiForcedAligner::~KaldiForcedAligner()
{
    delete d;
}

bool KaldiForcedAligner::alignTokens(const QString &waveFilepath, RealTime timeFrom, RealTime timeTo,
                                     IntervalTier *tierTokens, int &indexFrom, int &indexTo,
                                     bool insertLeadingAndTrailingPauses,
                                     QList<Interval *> &outPhonesList, QString &outAlignerOutput)
{
    return false;
}

void KaldiForcedAligner::mfcc_func(const QString &mfcc_directory, const QString &log_directory,
                              const QString &job_name, const QString &mfcc_config_path)
{
    QString raw_mfcc_path = mfcc_directory + QString("/raw_mfcc.%1.ark").arg(job_name);
    QString raw_scp_path = mfcc_directory + QString("raw_mfcc.%1.scp").arg(job_name);
    QString log_path = log_directory + QString("/make_mfcc.%1.log").arg(job_name);
    QString segment_path = log_directory + QString("/segments.%1").arg(job_name);
    QString scp_path = log_directory + QString("/wav.%1.scp").arg(job_name);

    QProcess seg_proc;
    QProcess comp_proc;
    QProcess copy_proc;

    if (QFile(segment_path).exists()) {
    } else {
    }
    //    with open(log_path, 'w') as f:
    //        if os.path.exists(segment_path):
    //            seg_proc = subprocess.Popen([thirdparty_binary('extract-segments'),
    //                                         'scp,p:' + scp_path, segment_path, 'ark:-'],
    //                                        stdout=subprocess.PIPE, stderr=f)
    //            comp_proc = subprocess.Popen([thirdparty_binary('compute-mfcc-feats'), '--verbose=2',
    //                                          '--config=' + mfcc_config_path,
    //                                          'ark:-', 'ark:-'],
    //                                         stdout=subprocess.PIPE, stderr=f, stdin=seg_proc.stdout)
    //        else:

    //            comp_proc = subprocess.Popen([thirdparty_binary('compute-mfcc-feats'), '--verbose=2',
    //                                          '--config=' + mfcc_config_path,
    //                                          'scp,p:' + scp_path, 'ark:-'],
    //                                         stdout=subprocess.PIPE, stderr=f)
    //        copy_proc = subprocess.Popen([thirdparty_binary('copy-feats'),
    //                                      '--compress=true', 'ark:-',
    //                                      'ark,scp:{},{}'.format(raw_mfcc_path, raw_scp_path)],
    //                                     stdin=comp_proc.stdout, stderr=f)
    //        copy_proc.wait()
}

} // namespace ASR
} // namespace Praaline

