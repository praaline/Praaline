#ifndef PRAATPITCH_H
#define PRAATPITCH_H

#include "../sys/praatlib.h"
#include "../fon/Sound.h"
#include "../fon/Pitch.h"

class PRAAT_LIB_EXPORT PraatPitch
{
public:
    static Pitch Pitch_create(double tmin, double tmax, long nt, double dt, double t1,
                              double ceiling, int maxnCandidates);
    static void Pitch_pathFinder(Pitch pitch, double silenceThreshold, double voicingThreshold,
                                 double octaveCost, double octaveJumpCost, double voicedUnvoicedCost,
                                 double ceiling, int pullFormants);
    static Pitch Pitch_subtractLinearFit(Pitch pitch, int unit);
    static Pitch Pitch_interpolate(Pitch pitch);
    static Pitch Pitch_killOctaveJumps(Pitch pitch);
    static Pitch Pitch_smooth(Pitch pitch, double bandWidth);

    static Pitch Sound_to_Pitch_ac(Sound sound, double dt, double minimumPitch, double periodsPerWindow, int maxnCandidates, int accurate,
                                   double silenceThreshold, double voicingThreshold,
                                   double octaveCost, double octaveJumpCost, double voicedUnvoicedCost, double ceiling);
    static Pitch Sound_to_Pitch_cc(Sound sound, double dt, double minimumPitch, double periodsPerWindow, int maxnCandidates, int accurate,
                                   double silenceThreshold, double voicingThreshold,
                                   double octaveCost, double octaveJumpCost, double voicedUnvoicedCost, double ceiling);
    static Pitch Sound_to_Pitch_any(Sound sound, double dt, double minimumPitch, double periodsPerWindow, int maxnCandidates,
                                    int method, double silenceThreshold, double voicingThreshold,
                                    double octaveCost, double octaveJumpCost, double voicedUnvoicedCost, double ceiling);
    static Pitch Sound_to_Pitch(Sound sound, double timeStep, double minimumPitch, double maximumPitch);

private:
    PraatPitch();
};

#endif // PRAATPITCH_H
