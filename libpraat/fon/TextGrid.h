#ifndef _TextGrid_h_
#define _TextGrid_h_
/* TextGrid.h
 *
 * Copyright (C) 1992-2012,2014,2015 Paul Boersma
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

#include "AnyTier.h"
#include "Label.h"
#include "Graphics.h"
#include "TableOfReal.h"
#include "Table.h"

#include "TextGrid_def.h"

#ifdef PRAAT_LIB
#include "praatlib.h"
#endif

oo_CLASS_CREATE (TextPoint, AnyPoint);
PRAAT_LIB_EXPORT TextPoint TextPoint_create (double time, const char32 *mark);

PRAAT_LIB_EXPORT void TextPoint_setText (TextPoint me, const char32 *text);

oo_CLASS_CREATE (TextInterval, Function);
PRAAT_LIB_EXPORT TextInterval TextInterval_create (double tmin, double tmax, const char32 *text);

PRAAT_LIB_EXPORT void TextInterval_setText (TextInterval me, const char32 *text);

oo_CLASS_CREATE (TextTier, Function);
PRAAT_LIB_EXPORT TextTier TextTier_create (double tmin, double tmax);

PRAAT_LIB_EXPORT void TextTier_addPoint (TextTier me, double time, const char32 *mark);
PRAAT_LIB_EXPORT TextTier TextTier_readFromXwaves (MelderFile file);
PRAAT_LIB_EXPORT PointProcess TextTier_getPoints (TextTier me, const char32 *text);

oo_CLASS_CREATE (IntervalTier, Function);
PRAAT_LIB_EXPORT IntervalTier IntervalTier_create (double tmin, double tmax);
PRAAT_LIB_EXPORT IntervalTier IntervalTier_readFromXwaves (MelderFile file);
PRAAT_LIB_EXPORT void IntervalTier_writeToXwaves (IntervalTier me, MelderFile file);

PRAAT_LIB_EXPORT long IntervalTier_timeToLowIndex (IntervalTier me, double t);
PRAAT_LIB_EXPORT long IntervalTier_timeToIndex (IntervalTier me, double t);   // obsolete
PRAAT_LIB_EXPORT long IntervalTier_timeToHighIndex (IntervalTier me, double t);
PRAAT_LIB_EXPORT long IntervalTier_hasTime (IntervalTier me, double t);
PRAAT_LIB_EXPORT long IntervalTier_hasBoundary (IntervalTier me, double t);
PRAAT_LIB_EXPORT PointProcess IntervalTier_getStartingPoints (IntervalTier me, const char32 *text);
PRAAT_LIB_EXPORT PointProcess IntervalTier_getEndPoints (IntervalTier me, const char32 *text);
PRAAT_LIB_EXPORT PointProcess IntervalTier_getCentrePoints (IntervalTier me, const char32 *text);
PRAAT_LIB_EXPORT PointProcess IntervalTier_PointProcess_startToCentre (IntervalTier tier, PointProcess point, double phase);
PRAAT_LIB_EXPORT PointProcess IntervalTier_PointProcess_endToCentre (IntervalTier tier, PointProcess point, double phase);
PRAAT_LIB_EXPORT void IntervalTier_removeLeftBoundary (IntervalTier me, long iinterval);

PRAAT_LIB_EXPORT void TextTier_removePoint (TextTier me, long ipoint);

oo_CLASS_CREATE (TextGrid, Function);
PRAAT_LIB_EXPORT TextGrid TextGrid_createWithoutTiers (double tmin, double tmax);
PRAAT_LIB_EXPORT TextGrid TextGrid_create (double tmin, double tmax, const char32 *tierNames, const char32 *pointTiers);

PRAAT_LIB_EXPORT long TextGrid_countLabels (TextGrid me, long itier, const char32 *text);
long TextGrid_countIntervalsWhere (TextGrid me, long tierNumber, int which_Melder_STRING, const char32 *criterion);
long TextGrid_countPointsWhere (TextGrid me, long tierNumber, int which_Melder_STRING, const char32 *criterion);
PRAAT_LIB_EXPORT PointProcess TextGrid_getStartingPoints (TextGrid me, long itier, int which_Melder_STRING, const char32 *criterion);
PRAAT_LIB_EXPORT PointProcess TextGrid_getEndPoints (TextGrid me, long itier, int which_Melder_STRING, const char32 *criterion);
PRAAT_LIB_EXPORT PointProcess TextGrid_getCentrePoints (TextGrid me, long itier, int which_Melder_STRING, const char32 *criterion);
PRAAT_LIB_EXPORT PointProcess TextGrid_getPoints (TextGrid me, long itier, int which_Melder_STRING, const char32 *criterion);
PRAAT_LIB_EXPORT PointProcess TextGrid_getPoints_preceded (TextGrid me, long tierNumber,
	int which_Melder_STRING, const char32 *criterion,
	int which_Melder_STRING_precededBy, const char32 *criterion_precededBy);
PRAAT_LIB_EXPORT PointProcess TextGrid_getPoints_followed (TextGrid me, long tierNumber,
	int which_Melder_STRING, const char32 *criterion,
	int which_Melder_STRING_followedBy, const char32 *criterion_followedBy);

PRAAT_LIB_EXPORT Function TextGrid_checkSpecifiedTierNumberWithinRange (TextGrid me, long tierNumber);
PRAAT_LIB_EXPORT IntervalTier TextGrid_checkSpecifiedTierIsIntervalTier (TextGrid me, long tierNumber);
PRAAT_LIB_EXPORT TextTier TextGrid_checkSpecifiedTierIsPointTier (TextGrid me, long tierNumber);

PRAAT_LIB_EXPORT void TextGrid_addTier (TextGrid me, Function tier);
PRAAT_LIB_EXPORT TextGrid TextGrid_merge (Collection textGrids);
PRAAT_LIB_EXPORT TextGrid TextGrid_extractPart (TextGrid me, double tmin, double tmax, int preserveTimes);

TextGrid Label_to_TextGrid (Label me, double duration);
TextGrid Label_Function_to_TextGrid (Label me, Any function);

TextTier PointProcess_upto_TextTier (PointProcess me, const char32 *text);
TableOfReal IntervalTier_downto_TableOfReal (IntervalTier me, const char32 *label);
TableOfReal IntervalTier_downto_TableOfReal_any (IntervalTier me);
TableOfReal TextTier_downto_TableOfReal (TextTier me, const char32 *label);
TableOfReal TextTier_downto_TableOfReal_any (TextTier me);

TextGrid PointProcess_to_TextGrid_vuv (PointProcess me, double maxT, double meanT);

PRAAT_LIB_EXPORT long TextInterval_labelLength (TextInterval me);
PRAAT_LIB_EXPORT long TextPoint_labelLength (TextPoint me);
PRAAT_LIB_EXPORT long IntervalTier_maximumLabelLength (IntervalTier me);
PRAAT_LIB_EXPORT long TextTier_maximumLabelLength (TextTier me);
PRAAT_LIB_EXPORT long TextGrid_maximumLabelLength (TextGrid me);
PRAAT_LIB_EXPORT void TextGrid_genericize (TextGrid me);
PRAAT_LIB_EXPORT void TextGrid_nativize (TextGrid me);

PRAAT_LIB_EXPORT void TextInterval_removeText (TextInterval me);
PRAAT_LIB_EXPORT void TextPoint_removeText (TextPoint me);
PRAAT_LIB_EXPORT void IntervalTier_removeText (IntervalTier me);
PRAAT_LIB_EXPORT void TextTier_removeText (TextTier me);

PRAAT_LIB_EXPORT void TextGrid_insertBoundary (TextGrid me, int itier, double t);
PRAAT_LIB_EXPORT void TextGrid_removeBoundaryAtTime (TextGrid me, int itier, double t);
PRAAT_LIB_EXPORT void TextGrid_setIntervalText (TextGrid me, int itier, long iinterval, const char32 *text);
PRAAT_LIB_EXPORT void TextGrid_insertPoint (TextGrid me, int itier, double t, const char32 *mark);
PRAAT_LIB_EXPORT void TextGrid_setPointText (TextGrid me, int itier, long ipoint, const char32 *text);

PRAAT_LIB_EXPORT void TextGrid_writeToChronologicalTextFile (TextGrid me, MelderFile file);
PRAAT_LIB_EXPORT TextGrid TextGrid_readFromChronologicalTextFile (MelderFile file);
PRAAT_LIB_EXPORT TextGrid TextGrid_readFromCgnSyntaxFile (MelderFile file);

PRAAT_LIB_EXPORT Table TextGrid_downto_Table (TextGrid me, bool includeLineNumbers, int timeDecimals, bool includeTierNames, bool includeEmptyIntervals);
void TextGrid_list (TextGrid me, bool includeLineNumbers, int timeDecimals, bool includeTierNames, bool includeEmptyIntervals);

void TextGrid_correctRoundingErrors (TextGrid me);
TextGrid TextGrids_concatenate (Collection me);

/* End of file TextGrid.h */
#endif
