#include "../sys/praatlib.h"
#include "../fon/Sound.h"
#include "../fon/Pitch.h"
#include "../fon/Sound_to_Pitch.h"
#include "PraatSession.h"
#include "PraatPitch.h"

PraatPitch::PraatPitch()
{

}

Pitch PraatPitch::Pitch_create(double tmin, double tmax, long nt, double dt, double t1,
                               double ceiling, int maxnCandidates)
{
    try {
        return Pitch_create(tmin, tmax, nt, dt, t1, ceiling, maxnCandidates);
    } catch (const char* e) {
        PraatSession::set_error(e);
    } catch (MelderError) {
        PraatSession::set_melder_error();
    } catch (...) {
        PraatSession::set_error("Unknown error");
    }
    return NULL;
}

void PraatPitch::Pitch_pathFinder(Pitch pitch, double silenceThreshold, double voicingThreshold,
                                  double octaveCost, double octaveJumpCost, double voicedUnvoicedCost,
                                  double ceiling, int pullFormants)
{
    try {
        Pitch_pathFinder(pitch, silenceThreshold, voicingThreshold, octaveCost, octaveJumpCost,
                         voicedUnvoicedCost, ceiling, pullFormants);
    } catch (const char* e) {
        PraatSession::set_error(e);
    } catch (MelderError) {
        PraatSession::set_melder_error();
    } catch (...) {
        PraatSession::set_error("Unknown error");
    }
}

Pitch PraatPitch::Pitch_subtractLinearFit(Pitch pitch, int unit)
{
    try {
        return Pitch_subtractLinearFit(pitch, unit);
    } catch (const char* e) {
        PraatSession::set_error(e);
    } catch (MelderError) {
        PraatSession::set_melder_error();
    } catch (...) {
        PraatSession::set_error("Unknown error");
    }
    return NULL;
}

Pitch PraatPitch::Pitch_interpolate(Pitch pitch)
{
    try {
        return Pitch_interpolate(pitch);
    } catch (const char* e) {
        PraatSession::set_error(e);
    } catch (MelderError) {
        PraatSession::set_melder_error();
    } catch (...) {
        PraatSession::set_error("Unknown error");
    }
    return NULL;
}

Pitch PraatPitch::Pitch_killOctaveJumps(Pitch pitch)
{
    try {
        return Pitch_killOctaveJumps(pitch);
    } catch (const char* e) {
        PraatSession::set_error(e);
    } catch (MelderError) {
        PraatSession::set_melder_error();
    } catch (...) {
        PraatSession::set_error("Unknown error");
    }
    return NULL;
}

Pitch PraatPitch::Pitch_smooth(Pitch pitch, double bandWidth)
{
    try {
        return Pitch_smooth(pitch, bandWidth);
    } catch (const char* e) {
        PraatSession::set_error(e);
    } catch (MelderError) {
        PraatSession::set_melder_error();
    } catch (...) {
        PraatSession::set_error("Unknown error");
    }
    return NULL;
}

Pitch PraatPitch::Sound_to_Pitch_ac(Sound sound, double dt, double minimumPitch, double periodsPerWindow, int maxnCandidates, int accurate,
                                    double silenceThreshold, double voicingThreshold,
                                    double octaveCost, double octaveJumpCost, double voicedUnvoicedCost, double ceiling)
{
    try {
        return Sound_to_Pitch_ac(sound, dt, minimumPitch, periodsPerWindow, maxnCandidates, accurate,
                                 silenceThreshold, voicingThreshold,
                                 octaveCost, octaveJumpCost, voicedUnvoicedCost, ceiling);
    } catch (const char* e) {
        PraatSession::set_error(e);
    } catch (MelderError) {
        PraatSession::set_melder_error();
    } catch (...) {
        PraatSession::set_error("Unknown error");
    }
    return NULL;
}

Pitch PraatPitch::Sound_to_Pitch_cc(Sound sound, double dt, double minimumPitch, double periodsPerWindow, int maxnCandidates, int accurate,
                                    double silenceThreshold, double voicingThreshold,
                                    double octaveCost, double octaveJumpCost, double voicedUnvoicedCost, double ceiling)
{
    try {
        return Sound_to_Pitch_cc(sound, dt, minimumPitch, periodsPerWindow, maxnCandidates, accurate,
                                 silenceThreshold, voicingThreshold,
                                 octaveCost, octaveJumpCost, voicedUnvoicedCost, ceiling);
    } catch (const char* e) {
        PraatSession::set_error(e);
    } catch (MelderError) {
        PraatSession::set_melder_error();
    } catch (...) {
        PraatSession::set_error("Unknown error");
    }
    return NULL;
}

Pitch PraatPitch::Sound_to_Pitch_any(Sound sound, double dt, double minimumPitch, double periodsPerWindow, int maxnCandidates,
                                     int method, double silenceThreshold, double voicingThreshold,
                                     double octaveCost, double octaveJumpCost, double voicedUnvoicedCost, double ceiling)
{
    try {
        return Sound_to_Pitch_any(sound, dt, minimumPitch, periodsPerWindow, maxnCandidates,
                                  method, silenceThreshold, voicingThreshold,
                                  octaveCost, octaveJumpCost, voicedUnvoicedCost, ceiling);
    } catch (const char* e) {
        PraatSession::set_error(e);
    } catch (MelderError) {
        PraatSession::set_melder_error();
    } catch (...) {
        PraatSession::set_error("Unknown error");
    }
    return NULL;
}

Pitch PraatPitch::Sound_to_Pitch(Sound sound, double timeStep, double minimumPitch, double maximumPitch) {
    try {
        return Sound_to_Pitch(sound, timeStep, minimumPitch, maximumPitch);
    } catch (const char* e) {
        PraatSession::set_error(e);
    } catch (MelderError) {
        PraatSession::set_melder_error();
    } catch (...) {
        PraatSession::set_error("Unknown error");
    }
    return NULL;
}

