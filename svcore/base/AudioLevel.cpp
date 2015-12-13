/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

/*
   This is a modified version of a source file from the 
   Rosegarden MIDI and audio sequencer and notation editor.
   This file copyright 2000-2006 Chris Cannam.
*/

#include "AudioLevel.h"
#include <cmath>
#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include "system/System.h"

const double AudioLevel::DB_FLOOR = -1000.;

struct FaderDescription
{
    FaderDescription(double _minDb, double _maxDb, double _zeroPoint) :
	minDb(_minDb), maxDb(_maxDb), zeroPoint(_zeroPoint) { }

    double minDb;
    double maxDb;
    double zeroPoint; // as fraction of total throw
};

static const FaderDescription faderTypes[] = {
    FaderDescription(-40.,  +6., 0.75), // short
    FaderDescription(-70., +10., 0.80), // long
    FaderDescription(-70.,   0., 1.00), // IEC268
    FaderDescription(-70., +10., 0.80), // IEC268 long
    FaderDescription(-40.,   0., 1.00), // preview
};

//typedef std::vector<double> LevelList;
//static std::map<int, LevelList> previewLevelCache;
//static const LevelList &getPreviewLevelCache(int levels);

double
AudioLevel::multiplier_to_dB(double multiplier)
{
    if (multiplier == 0.) return DB_FLOOR;
    else if (multiplier < 0.) return multiplier_to_dB(-multiplier);
    double dB = 10 * log10(multiplier);
    return dB;
}

double
AudioLevel::dB_to_multiplier(double dB)
{
    if (dB == DB_FLOOR) return 0.;
    double m = pow(10., dB / 10.);
    return m;
}

/* IEC 60-268-18 fader levels.  Thanks to Steve Harris. */

static double iec_dB_to_fader(double db)
{
    double def = 0.0f; // Meter deflection %age

    if (db < -70.0f) {
        def = 0.0f;
    } else if (db < -60.0f) {
        def = (db + 70.0f) * 0.25f;
    } else if (db < -50.0f) {
        def = (db + 60.0f) * 0.5f + 2.5f; // corrected from 5.0f base, thanks Robin Gareus
    } else if (db < -40.0f) {
        def = (db + 50.0f) * 0.75f + 7.5f;
    } else if (db < -30.0f) {
        def = (db + 40.0f) * 1.5f + 15.0f;
    } else if (db < -20.0f) {
        def = (db + 30.0f) * 2.0f + 30.0f;
    } else {
        def = (db + 20.0f) * 2.5f + 50.0f;
    }

    return def;
}

static double iec_fader_to_dB(double def)  // Meter deflection %age
{
    double db = 0.0f;

    if (def >= 50.0f) {
	db = (def - 50.0f) / 2.5f - 20.0f;
    } else if (def >= 30.0f) {
	db = (def - 30.0f) / 2.0f - 30.0f;
    } else if (def >= 15.0f) {
	db = (def - 15.0f) / 1.5f - 40.0f;
    } else if (def >= 7.5f) {
	db = (def - 7.5f) / 0.75f - 50.0f;
    } else if (def >= 2.5f) {
	db = (def - 2.5f) / 0.5f - 60.0f;
    } else {
	db = (def / 0.25f) - 70.0f;
    }

    return db;
}

double
AudioLevel::fader_to_dB(int level, int maxLevel, FaderType type)
{
    if (level == 0) return DB_FLOOR;

    if (type == IEC268Meter || type == IEC268LongMeter) {

	double maxPercent = iec_dB_to_fader(faderTypes[type].maxDb);
	double percent = double(level) * maxPercent / double(maxLevel);
	double dB = iec_fader_to_dB(percent);
	return dB;

    } else { // scale proportional to sqrt(fabs(dB))

	int zeroLevel = int(maxLevel * faderTypes[type].zeroPoint);
    
	if (level >= zeroLevel) {
	    
	    double value = level - zeroLevel;
	    double scale = (maxLevel - zeroLevel) /
		sqrt(faderTypes[type].maxDb);
	    value /= scale;
	    double dB = pow(value, 2.);
	    return dB;
	    
	} else {
	    
	    double value = zeroLevel - level;
	    double scale = zeroLevel / sqrt(0. - faderTypes[type].minDb);
	    value /= scale;
	    double dB = pow(value, 2.);
	    return 0. - dB;
	}
    }
}


int
AudioLevel::dB_to_fader(double dB, int maxLevel, FaderType type)
{
    if (dB == DB_FLOOR) return 0;

    if (type == IEC268Meter || type == IEC268LongMeter) {

	// The IEC scale gives a "percentage travel" for a given dB
	// level, but it reaches 100% at 0dB.  So we want to treat the
	// result not as a percentage, but as a scale between 0 and
	// whatever the "percentage" for our (possibly >0dB) max dB is.
	
	double maxPercent = iec_dB_to_fader(faderTypes[type].maxDb);
	double percent = iec_dB_to_fader(dB);
	int faderLevel = int((maxLevel * percent) / maxPercent + 0.01f);
	
	if (faderLevel < 0) faderLevel = 0;
	if (faderLevel > maxLevel) faderLevel = maxLevel;
	return faderLevel;

    } else {

	int zeroLevel = int(maxLevel * faderTypes[type].zeroPoint);

	if (dB >= 0.) {
	    
            if (faderTypes[type].maxDb <= 0.) {
                
                return maxLevel;

            } else {

                double value = sqrt(dB);
                double scale = (maxLevel - zeroLevel) / sqrt(faderTypes[type].maxDb);
                value *= scale;
                int level = int(value + 0.01f) + zeroLevel;
                if (level > maxLevel) level = maxLevel;
                return level;
            }
	    
	} else {

	    dB = 0. - dB;
	    double value = sqrt(dB);
	    double scale = zeroLevel / sqrt(0. - faderTypes[type].minDb);
	    value *= scale;
	    int level = zeroLevel - int(value + 0.01f);
	    if (level < 0) level = 0;
	    return level;
	}
    }
}

	
double
AudioLevel::fader_to_multiplier(int level, int maxLevel, FaderType type)
{
    if (level == 0) return 0.;
    return dB_to_multiplier(fader_to_dB(level, maxLevel, type));
}

int
AudioLevel::multiplier_to_fader(double multiplier, int maxLevel, FaderType type)
{
    if (multiplier == 0.) return 0;
    double dB = multiplier_to_dB(multiplier);
    int fader = dB_to_fader(dB, maxLevel, type);
    return fader;
}

/*
const LevelList &
getPreviewLevelCache(int levels)
{
    LevelList &ll = previewLevelCache[levels];
    if (ll.empty()) {
	for (int i = 0; i <= levels; ++i) {
	    double m = AudioLevel::fader_to_multiplier
		(i + levels/4, levels + levels/4, AudioLevel::PreviewLevel);
	    if (levels == 1) m /= 100; // noise
	    ll.push_back(m);
	}
    }
    return ll;
}
*/

int
AudioLevel::multiplier_to_preview(double m, int levels)
{
    assert(levels > 0);
    return multiplier_to_fader(m, levels, PreviewLevel);

    /* The original multiplier_to_preview which follows is not thread-safe.

    if (m < 0.) return -multiplier_to_preview(-m, levels);

    const LevelList &ll = getPreviewLevelCache(levels);
    int result = -1;

    int lo = 0, hi = levels;

    // binary search
    int level = -1;
    while (result < 0) {
	int newlevel = (lo + hi) / 2;
	if (newlevel == level ||
	    newlevel == 0 ||
	    newlevel == levels) {
	    result = newlevel;
	    break;
	}
	level = newlevel;
	if (ll[level] >= m) {
	    hi = level;
	} else if (ll[level+1] >= m) {
	    result = level;
	} else {
	    lo = level;
	}
    }
		   
    return result;

    */
}

double
AudioLevel::preview_to_multiplier(int level, int levels)
{
    assert(levels > 0);
    return fader_to_multiplier(level, levels, PreviewLevel);
/*
    if (level < 0) return -preview_to_multiplier(-level, levels);
    const LevelList &ll = getPreviewLevelCache(levels);
    return ll[level];
*/
}
	

