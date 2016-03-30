#ifndef _Formant_h_
#define _Formant_h_
/* Formant.h
 *
 * Copyright (C) 1992-2011,2015 Paul Boersma
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Matrix.h"
#include "Table.h"
#include "Interpreter_decl.h"

#ifdef PRAAT_LIB
#include "praatlib.h"
#endif

#include "Formant_def.h"
oo_CLASS_CREATE (Formant, Sampled);

PRAAT_LIB_EXPORT Formant Formant_create (double tmin, double tmax, long nt, double dt, double t1, int maxnFormants);
/*
	Function:
		return a new instance of Formant, or NULL if out of memory.
	Preconditions:
		nt >= 1;
		dt > 0.0;
		maxnFormants >= 1;
	Postconditions:
		my xmin = tmin;
		my xmax = tmax;
		my nx = nt;
		my dx = dt;
		my x1 = t1;
		my maximumNumberOfPairs == maxnFormants;
		my frames [1..nt]. intensity == 0.0;
		my frames [1..nt]. numberOfPairs == 0;
		my frames [1..nt]. formants [1..maxnFormants] = 0.0;
		my frames [1..nt]. bandwidths [1..maxnFormants] = 0.0;
*/
PRAAT_LIB_EXPORT long Formant_getMinNumFormants (Formant me);
PRAAT_LIB_EXPORT long Formant_getMaxNumFormants (Formant me);

PRAAT_LIB_EXPORT double Formant_getValueAtTime (Formant me, int iformant, double time, int bark);
PRAAT_LIB_EXPORT double Formant_getBandwidthAtTime (Formant me, int iformant, double time, int bark);

PRAAT_LIB_EXPORT void Formant_getExtrema (Formant me, int iformant, double tmin, double tmax, double *fmin, double *fmax);
PRAAT_LIB_EXPORT void Formant_getMinimumAndTime (Formant me, int iformant, double tmin, double tmax, int bark, int interpolate,
	double *return_minimum, double *return_timeOfMinimum);
PRAAT_LIB_EXPORT void Formant_getMaximumAndTime (Formant me, int iformant, double tmin, double tmax, int bark, int interpolate,
	double *return_maximum, double *return_timeOfMaximum);
PRAAT_LIB_EXPORT double Formant_getMinimum (Formant me, int iformant, double tmin, double tmax, int bark, int interpolate);
PRAAT_LIB_EXPORT double Formant_getMaximum (Formant me, int iformant, double tmin, double tmax, int bark, int interpolate);
PRAAT_LIB_EXPORT double Formant_getTimeOfMaximum (Formant me, int iformant, double tmin, double tmax, int bark, int interpolate);
PRAAT_LIB_EXPORT double Formant_getTimeOfMinimum (Formant me, int iformant, double tmin, double tmax, int bark, int interpolate);

PRAAT_LIB_EXPORT double Formant_getQuantile (Formant me, int iformant, double quantile, double tmin, double tmax, int bark);
PRAAT_LIB_EXPORT double Formant_getQuantileOfBandwidth (Formant me, int iformant, double quantile, double tmin, double tmax, int bark);
PRAAT_LIB_EXPORT double Formant_getMean (Formant me, int iformant, double tmin, double tmax, int bark);
PRAAT_LIB_EXPORT double Formant_getStandardDeviation (Formant me, int iformant, double tmin, double tmax, int bark);

PRAAT_LIB_EXPORT void Formant_sort (Formant me);

void Formant_drawTracks (Formant me, Graphics g, double tmin, double tmax, double fmax, int garnish);
void Formant_drawSpeckles_inside (Formant me, Graphics g, double tmin, double tmax, double fmin, double fmax,
	double suppress_dB);
void Formant_drawSpeckles (Formant me, Graphics g, double tmin, double tmax, double fmax,
	double suppress_dB, int garnish);
void Formant_scatterPlot (Formant me, Graphics g, double tmin, double tmax,
	int iformant1, double fmin1, double fmax1, int iformant2, double fmin2, double fmax2,
	double size_mm, const char32 *mark, int garnish);

PRAAT_LIB_EXPORT Matrix Formant_to_Matrix (Formant me, int iformant);
PRAAT_LIB_EXPORT Matrix Formant_to_Matrix_bandwidths (Formant me, int iformant);
void Formant_formula_frequencies (Formant me, const char32 *formula, Interpreter interpreter);
void Formant_formula_bandwidths (Formant me, const char32 *formula, Interpreter interpreter);

PRAAT_LIB_EXPORT Formant Formant_tracker (Formant me, int numberOfTracks,
	double refF1, double refF2, double refF3, double refF4, double refF5,
	double dfCost,   // per kHz
	double bfCost, double octaveJumpCost);

PRAAT_LIB_EXPORT Table Formant_downto_Table (Formant me, bool includeFrameNumbers,
	bool includeTimes, int timeDecimals,
	bool includeIntensity, int intensityDecimals,
	bool includeNumberOfFormants, int frequencyDecimals,
	bool includeBandwidths);
void Formant_list (Formant me, bool includeFrameNumbers,
	bool includeTimes, int timeDecimals,
	bool includeIntensity, int intensityDecimals,
	bool includeNumberOfFormants, int frequencyDecimals,
	bool includeBandwidths);

#ifdef PRAAT_LIB
PRAAT_LIB_EXPORT double Formant_getValueAtSample(Formant me, long isample, long which, int units);
PRAAT_LIB_EXPORT double Formant_getIntensityAtSample(Formant me, long iframe);
#endif

/* End of file Formant.h */
#endif
