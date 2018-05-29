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

} // namespace ASR
} // namespace Praaline

