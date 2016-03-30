/* TextGrid_def.h
 *
 * Copyright (C) 1992-2011,2014,2015 Paul Boersma
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

#ifdef PRAAT_LIB
#include "praatlib.h"
#endif


#define ooSTRUCT TextPoint
oo_DEFINE_CLASS (TextPoint, AnyPoint)

	oo_STRING (mark)

	/* 'after' was a temporary attribute (19970211-19970307). */
	#if oo_READING_TEXT
		if (Thing_version == 1) texgetw2 (a_text);
	#elif oo_READING_BINARY
		if (Thing_version == 1) bingetw2 (f);
	#endif

oo_END_CLASS (TextPoint)
#undef ooSTRUCT
#ifdef PRAAT_LIB
PRAAT_LIB_EXPORT const char32 *TextPoint_getText(TextPoint me);
#endif


#define ooSTRUCT TextInterval
oo_DEFINE_CLASS (TextInterval, Function)

	oo_STRING (text)

	#if oo_DECLARING
		int v_domainQuantity ()
			override { return MelderQuantity_TIME_SECONDS; }
	#endif

oo_END_CLASS (TextInterval)
#undef ooSTRUCT
#ifdef PRAAT_LIB
PRAAT_LIB_EXPORT const char32 *TextInterval_getText(TextInterval me);
#endif

#define ooSTRUCT TextTier
oo_DEFINE_CLASS (TextTier, Function)

	oo_COLLECTION (SortedSetOfDouble, points, TextPoint, 0)

	#if oo_DECLARING
		long numberOfPoints () // accessor
			{ return our points -> size; }
		TextPoint point (long i) // accessor
			{ return static_cast <TextPoint> (our points -> item [i]); }

		int v_domainQuantity ()
			override { return MelderQuantity_TIME_SECONDS; }
		void v_shiftX (double xfrom, double xto)
			override;
		void v_scaleX (double xminfrom, double xmaxfrom, double xminto, double xmaxto)
			override;
	#endif

oo_END_CLASS (TextTier)
#undef ooSTRUCT
#ifdef PRAAT_LIB
PRAAT_LIB_EXPORT long TextTier_numberOfPoints (I);
PRAAT_LIB_EXPORT TextPoint TextTier_point (I, long i);
PRAAT_LIB_EXPORT void TextTier_removePoints (I, int which_Melder_STRING, const char32 *criterion);
PRAAT_LIB_EXPORT int TextTier_domainQuantity (I);
PRAAT_LIB_EXPORT void TextTier_shiftX (I, double xfrom, double xto);
PRAAT_LIB_EXPORT void TextTier_scaleX (I, double xminfrom, double xmaxfrom, double xminto, double xmaxto);
#endif


#define ooSTRUCT IntervalTier
oo_DEFINE_CLASS (IntervalTier, Function)

	oo_COLLECTION (SortedSetOfDouble, intervals, TextInterval, 0)

	#if oo_DECLARING
		long numberOfIntervals () // accessor
			{ return our intervals -> size; }
		TextInterval interval (long i) // accessor
			{ return static_cast <TextInterval> (our intervals -> item [i]); }
		//template <class T> T& operator[] (long i) { return (T) (our intervals -> item [i]); }
		//TextInterval* intervalss () { return (TextInterval *) (our intervals -> item); }

		int v_domainQuantity ()
			override { return MelderQuantity_TIME_SECONDS; }
		void v_shiftX (double xfrom, double xto)
			override;
		void v_scaleX (double xminfrom, double xmaxfrom, double xminto, double xmaxto)
			override;
	#endif

oo_END_CLASS (IntervalTier)
#undef ooSTRUCT
#ifdef PRAAT_LIB
PRAAT_LIB_EXPORT long IntervalTier_numberOfIntervals (I);
PRAAT_LIB_EXPORT TextInterval IntervalTier_interval (I, long i);
PRAAT_LIB_EXPORT int IntervalTier_domainQuantity (I);
PRAAT_LIB_EXPORT void IntervalTier_shiftX (I, double xfrom, double xto);
PRAAT_LIB_EXPORT void IntervalTier_scaleX (I, double xminfrom, double xmaxfrom, double xminto, double xmaxto);

// 'unsafe' addition of interval to end of internal list
PRAAT_LIB_EXPORT void IntervalTier_addInterval (IntervalTier me, double tmin, double tmax, const char32 *label);
PRAAT_LIB_EXPORT void IntervalTier_removeInterval (IntervalTier me, long iinterval);
#endif

#define ooSTRUCT TextGrid
oo_DEFINE_CLASS (TextGrid, Function)

	oo_OBJECT (Ordered, 0, tiers)   // TextTier and IntervalTier objects

	#if oo_DECLARING
		long numberOfTiers () // accessor
			{ return our tiers -> size; }
		Function tier (long i) // accessor
			{ return static_cast <Function> (our tiers -> item [i]); }

		void v_info ()
			override;
		void v_repair ()
			override;
		int v_domainQuantity ()
			override { return MelderQuantity_TIME_SECONDS; }
		void v_shiftX (double xfrom, double xto)
			override;
		void v_scaleX (double xminfrom, double xmaxfrom, double xminto, double xmaxto)
			override;
	#endif

oo_END_CLASS (TextGrid)
#undef ooSTRUCT
#ifdef PRAAT_LIB
PRAAT_LIB_EXPORT long TextGrid_numberOfTiers (I);
PRAAT_LIB_EXPORT Function TextGrid_tier (I, long i);
PRAAT_LIB_EXPORT void TextGrid_removePoints (I, long tierNumber, int which_Melder_STRING, const char32 *criterion);
PRAAT_LIB_EXPORT void TextGrid_repair (I);
PRAAT_LIB_EXPORT int TextGrid_domainQuantity (I);
PRAAT_LIB_EXPORT void TextGrid_shiftX (I, double xfrom, double xto);
PRAAT_LIB_EXPORT void TextGrid_scaleX (I, double xminfrom, double xmaxfrom, double xminto, double xmaxto);
#endif


/* End of file TextGrid_def.h */
