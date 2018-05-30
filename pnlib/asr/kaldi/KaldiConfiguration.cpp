#include <QString>
#include <QList>
#include <QHash>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include "KaldiConfiguration.h"

namespace Praaline {
namespace ASR {

KaldiConfiguration::KaldiConfiguration()
{
}

// num_iters : int          Number of training iterations to perform, defaults to 40
// scale_opts : list        Options for specifying scaling in alignment
// beam : int               Default beam width for alignment, defaults = 10
// retry_beam : int         Beam width to fall back on if no alignment is produced, defaults to 40
// max_iter_inc : int       Last iter to increase #Gauss on, defaults to 30
// totgauss : int           Total number of gaussians, defaults to 1000
// boost_silence : float    Factor by which to boost silence likelihoods in alignment, defaults to 1.0
// realign_iters : list     List of iterations to perform alignment
// stage : int              Not used
// power : float            Exponent for number of gaussians according to occurrence counts, defaults to 0.25
// do_fmllr : bool          Specifies whether to do speaker adaptation, defaults to False

KaldiMonophoneConfig::KaldiMonophoneConfig(bool align_often)
{
    num_iters = 40;
    scale_opts << "--transition-scale=1.0" << "--acoustic-scale=0.1" << "--self-loop-scale=0.1";
    beam = 10;
    retry_beam = 40;
    initial_gauss_count = 0;
    max_gauss_count = 1000;
    boost_silence = 1.0;
    if (align_often)
        realign_iters << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10
                      << 12 << 14 << 16 << 18 << 20
                      << 23 << 26 << 29 << 32 << 35 << 38;
    else
        realign_iters << 1 << 5 << 10 << 15 << 20 << 25 << 30 << 35 << 38;
    stage = -4;
    power = 0.25;
    do_fmllr = false;
}

int KaldiMonophoneConfig::max_iter_inc()
{
    return num_iters - 10;
}

int KaldiMonophoneConfig::inc_gauss_count()
{
    return int((max_gauss_count - initial_gauss_count) / max_iter_inc());
}

KaldiTriphoneConfig::KaldiTriphoneConfig(bool align_often) :
    KaldiMonophoneConfig(align_often)
{
    num_iters = 35;
    initial_gauss_count = 3100;
    max_gauss_count = 50000;
    cluster_threshold = 100;
}

KaldiTriphoneFmllrConfig::KaldiTriphoneFmllrConfig(bool align_often) :
    KaldiTriphoneConfig(align_often)
{
    do_fmllr = true;
    fmllr_update_type = "full";
    fmllr_iters << 2 << 4 << 6 << 12;
    fmllr_power = 0.2;
    silence_weight = 0.0;
}


MfccConfig::MfccConfig(const QString &output_directory, const QString &job,
                       QHash<QString, QString> config_dict) :
    output_directory(output_directory), job(job), config_dict(config_dict)
{
    if (!config_dict.contains("use-energy")) config_dict.insert("use-energy", "false");
    if (!config_dict.contains("frame-shift")) config_dict.insert("frame-shift", "10");
    write();
}

void MfccConfig::update(QHash<QString, QString> new_config_dict)
{
    foreach (QString key, new_config_dict.keys()) {
        if (config_dict.contains(key))
            config_dict[key] = new_config_dict[key];
        else
            config_dict.insert(key, new_config_dict[key]);
    }
}

QString MfccConfig::config_directory() const
{
    QString config_path = output_directory + "/config";
    QDir d;
    d.mkpath(config_path);
    return config_path;
}

QString MfccConfig::path() const
{
    if (job.isEmpty())
        return config_directory() + "/mfcc.conf";
    return config_directory() + QString("/mfcc.%1.conf").arg(job);
}

/// Write configuration dictionary to a file for use in Kaldi binaries
bool MfccConfig::write() const
{
    QFile file(path());
    if ( !file.open( QIODevice::ReadWrite | QIODevice::Text ) ) return false;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);
    foreach (QString key, config_dict.keys()) {
        out << "--" << key << "=" << config_dict.value(key) << "\n";
    }

}


} // namespace ASR
} // namespace Praaline

