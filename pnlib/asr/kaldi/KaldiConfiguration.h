#ifndef KALDICONFIGURATION_H
#define KALDICONFIGURATION_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QStringList>

namespace Praaline {
namespace ASR {

class KaldiConfiguration
{
public:
    KaldiConfiguration();
    virtual ~KaldiConfiguration() {}
};

// Configuration class for monophone training
// Scale options defaults to::
// ['--transition-scale=1.0', '--acoustic-scale=0.1', '--self-loop-scale=0.1']
// If ``align_often`` is False in the keyword arguments, ``realign_iters`` will be::
//      [1, 5, 10, 15, 20, 25, 30, 35, 38]
// Otherwise, ``realign_iters`` will be::
//      [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 23, 26, 29, 32, 35, 38]
// Attributes
// ----------
// num_iters : int              Number of training iterations to perform, defaults to 40
// scale_opts : list            Options for specifying scaling in alignment
// beam : int                   Default beam width for alignment, defaults = 10
// retry_beam : int             Beam width to fall back on if no alignment is produced, defaults to 40
// max_iter_inc : int           Last iter to increase #Gauss on, defaults to 30
// totgauss : int               Total number of gaussians, defaults to 1000
// boost_silence : float        Factor by which to boost silence likelihoods in alignment, defaults to 1.0
// realign_iters : list         List of iterations to perform alignment
// stage : int                  Not used
// power : float                Exponent for number of gaussians according to occurrence counts, defaults to 0.25
// do_fmllr : bool              Specifies whether to do speaker adaptation, defaults to False

class KaldiMonophoneConfig : public KaldiConfiguration
{
public:
    KaldiMonophoneConfig(bool align_often = false);
    ~KaldiMonophoneConfig() {}

    int num_iters;
    QStringList scale_opts;
    int beam;
    int retry_beam;
    int initial_gauss_count;
    int max_gauss_count;
    int totgauss;
    double boost_silence;
    QList<int> realign_iters;
    int stage;
    double power;
    bool do_fmllr;

    int max_iter_inc();
    int inc_gauss_count();
};


// Configuration class for triphone training
// Scale options defaults to::
//      ['--transition-scale=1.0', '--acoustic-scale=0.1', '--self-loop-scale=0.1']
// If ``align_often`` is True in the keyword arguments, ``realign_iters`` will be::
//      [1, 5, 10, 15, 20, 25, 30, 35, 38]
// Otherwise, ``realign_iters`` will be::
//      [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 23, 26, 29, 32, 35, 38]
// Attributes
// ----------
// num_iters : int              Number of training iterations to perform, defaults to 35
// scale_opts : list            Options for specifying scaling in alignment
// beam : int                   Default beam width for alignment, defaults = 10
// retry_beam : int             Beam width to fall back on if no alignment is produced, defaults to 40
// max_iter_inc : int           Last iter to increase #Gauss on, defaults to 30
// totgauss : int               Total number of gaussians, defaults to 1000
// boost_silence : float        Factor by which to boost silence likelihoods in alignment, defaults to 1.0
// realign_iters : list         List of iterations to perform alignment
// stage : int                  Not used
// power : float                Exponent for number of gaussians according to occurrence counts, defaults to 0.25
// do_fmllr : bool              Specifies whether to do speaker adaptation, defaults to False
// initial_gauss_count : int    Number of states in the decision tree, defaults to 3100
// max_gauss_count : int        Number of gaussians in the decision tree, defaults to 50000
// cluster_threshold : int      For build-tree control final bottom-up clustering of leaves, defaults to 100

class KaldiTriphoneConfig : public KaldiMonophoneConfig
{
public:
    KaldiTriphoneConfig(bool align_often = false);
    ~KaldiTriphoneConfig() {}

    int cluster_threshold;
};

// Configuration class for speaker-adapted triphone training
// Scale options defaults to::
//    ['--transition-scale=1.0', '--acoustic-scale=0.1', '--self-loop-scale=0.1']
// If ``align_often`` is True in the keyword arguments, ``realign_iters`` will be::
//    [1, 5, 10, 15, 20, 25, 30, 35, 38]
// Otherwise, ``realign_iters`` will be::
//    [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 23, 26, 29, 32, 35, 38]
// ``fmllr_iters`` defaults to::
//    [2, 4, 6, 12]
// Attributes
// ----------
// num_iters : int              Number of training iterations to perform, defaults to 35
// scale_opts : list            Options for specifying scaling in alignment
// beam : int                   Default beam width for alignment, defaults = 10
// retry_beam : int             Beam width to fall back on if no alignment is produced, defaults to 40
// max_iter_inc : int           Last iter to increase #Gauss on, defaults to 30
// totgauss : int               Total number of gaussians, defaults to 1000
// boost_silence : float        Factor by which to boost silence likelihoods in alignment, defaults to 1.0
// realign_iters : list         List of iterations to perform alignment
// stage : int                  Not used
// power : float                Exponent for number of gaussians according to occurrence counts, defaults to 0.25
// do_fmllr : bool              Specifies whether to do speaker adaptation, defaults to True
// initial_gauss_count : int    Number of states in the decision tree, defaults to 3100
// max_gauss_count : int        Number of gaussians in the decision tree, defaults to 50000
// cluster_threshold : int      For build-tree control final bottom-up clustering of leaves, defaults to 100
// fmllr_update_type : str      Type of fMLLR estimation, defaults to ``'full'``
// fmllr_iters : list           List of iterations to perform fMLLR estimation
// fmllr_power : float          Defaults to 0.2
// silence_weight : float       Weight on silence in fMLLR estimation

class KaldiTriphoneFmllrConfig : public KaldiTriphoneConfig
{
public:
    KaldiTriphoneFmllrConfig(bool align_often = false);
    ~KaldiTriphoneFmllrConfig() {}

    QString fmllr_update_type;
    QList<int> fmllr_iters;
    double fmllr_power;
    double silence_weight;
};

// Class to store configuration information about MFCC generation
// The ``config_dict`` currently stores one key ``'use-energy'`` which
// defaults to False
// Parameters
// ----------
// output_directory : str       Path to directory to save configuration files for Kaldi
// kwargs : dict, optional      If specified, updates ``config_dict`` with this dictionary
// Attributes
// ----------
// config_dict : dict           Dictionary of configuration parameters

class MfccConfig : public KaldiConfiguration
{
    MfccConfig(const QString &output_directory, const QString &job = QString(),
               QHash<QString, QString> config_dict = QHash<QString, QString>());
    ~MfccConfig() {}

    QString job;
    QHash<QString, QString> config_dict;
    QString output_directory;

    void update(QHash<QString, QString> new_config_dict);
    QString config_directory() const;
    QString path() const;
    bool write() const;
};


} // namespace ASR
} // namespace Praaline

#endif // KALDICONFIGURATION_H
