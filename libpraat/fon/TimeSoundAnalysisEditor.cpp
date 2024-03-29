/* TimeSoundAnalysisEditor.cpp
 *
 * Copyright (C) 1992-2012,2013,2014,2015 Paul Boersma
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

#include <time.h>
#include "TimeSoundAnalysisEditor.h"
#include "Preferences.h"
#include "EditorM.h"
#include "Sound_and_Spectrogram.h"
#include "Sound_and_Spectrum.h"
#include "Sound_to_Pitch.h"
#include "Sound_to_Intensity.h"
#include "Sound_to_Formant.h"
#include "Pitch_to_PointProcess.h"
#include "VoiceAnalysis.h"
#include "praat_script.h"

#include "enums_getText.h"
#include "TimeSoundAnalysisEditor_enums.h"
#include "enums_getValue.h"
#include "TimeSoundAnalysisEditor_enums.h"

Thing_implement (TimeSoundAnalysisEditor, TimeSoundEditor, 0);

#include "prefs_define.h"
#include "TimeSoundAnalysisEditor_prefs.h"
#include "prefs_install.h"
#include "TimeSoundAnalysisEditor_prefs.h"
#include "prefs_copyToInstance.h"
#include "TimeSoundAnalysisEditor_prefs.h"

static const char32 * theMessage_Cannot_compute_spectrogram = U"The spectrogram is not defined at the edge of the sound.";
static const char32 * theMessage_Cannot_compute_pitch = U"The pitch contour is not defined at the edge of the sound.";
static const char32 * theMessage_Cannot_compute_formant = U"The formants are not defined at the edge of the sound.";
static const char32 * theMessage_Cannot_compute_intensity = U"The intensity curve is not defined at the edge of the sound.";
static const char32 * theMessage_Cannot_compute_pulses = U"The pulses are not defined at the edge of the sound.";

void structTimeSoundAnalysisEditor :: v_destroy () {
	v_destroy_analysis ();
	TimeSoundAnalysisEditor_Parent :: v_destroy ();
}

void structTimeSoundAnalysisEditor :: v_info () {
	TimeSoundAnalysisEditor_Parent :: v_info ();
	if (v_hasSpectrogram ()) {
		/* Spectrogram flag: */
		MelderInfo_writeLine (U"Spectrogram show: ", p_spectrogram_show);
		/* Spectrogram settings: */
		MelderInfo_writeLine (U"Spectrogram view from: ", p_spectrogram_viewFrom, U" Hz");
		MelderInfo_writeLine (U"Spectrogram view to: ", p_spectrogram_viewTo, U" Hz");
		MelderInfo_writeLine (U"Spectrogram window length: ", p_spectrogram_windowLength, U" seconds");
		MelderInfo_writeLine (U"Spectrogram dynamic range: ", p_spectrogram_dynamicRange, U" dB");
		/* Advanced spectrogram settings: */
		MelderInfo_writeLine (U"Spectrogram number of time steps: ", p_spectrogram_timeSteps);
		MelderInfo_writeLine (U"Spectrogram number of frequency steps: ", p_spectrogram_frequencySteps);
		MelderInfo_writeLine (U"Spectrogram method: ", U"Fourier");
		MelderInfo_writeLine (U"Spectrogram window shape: ", kSound_to_Spectrogram_windowShape_getText (p_spectrogram_windowShape));
		MelderInfo_writeLine (U"Spectrogram autoscaling: ", p_spectrogram_autoscaling);
		MelderInfo_writeLine (U"Spectrogram maximum: ", p_spectrogram_maximum, U" dB/Hz");
		MelderInfo_writeLine (U"Spectrogram pre-emphasis: ", p_spectrogram_preemphasis, U" dB/octave");
		MelderInfo_writeLine (U"Spectrogram dynamicCompression: ", p_spectrogram_dynamicCompression);
		/* Dynamic information: */
		MelderInfo_writeLine (U"Spectrogram cursor frequency: ", d_spectrogram_cursor, U" Hz");
	}
	if (v_hasPitch ()) {
		/* Pitch flag: */
		MelderInfo_writeLine (U"Pitch show: ", p_pitch_show);
		/* Pitch settings: */
		MelderInfo_writeLine (U"Pitch floor: ", p_pitch_floor, U" Hz");
		MelderInfo_writeLine (U"Pitch ceiling: ", p_pitch_ceiling, U" Hz");
		MelderInfo_writeLine (U"Pitch unit: ", Function_getUnitText (Thing_dummyObject (Pitch), Pitch_LEVEL_FREQUENCY, p_pitch_unit, Function_UNIT_TEXT_MENU));
		MelderInfo_writeLine (U"Pitch drawing method: ", kTimeSoundAnalysisEditor_pitch_drawingMethod_getText (p_pitch_drawingMethod));
		/* Advanced pitch settings: */
		MelderInfo_writeLine (U"Pitch view from: ", p_pitch_viewFrom, U" ", Function_getUnitText (Thing_dummyObject (Pitch), Pitch_LEVEL_FREQUENCY, p_pitch_unit, Function_UNIT_TEXT_MENU));
		MelderInfo_writeLine (U"Pitch view to: ", p_pitch_viewTo, U" ", Function_getUnitText (Thing_dummyObject (Pitch), Pitch_LEVEL_FREQUENCY, p_pitch_unit, Function_UNIT_TEXT_MENU));
		MelderInfo_writeLine (U"Pitch method: ", kTimeSoundAnalysisEditor_pitch_analysisMethod_getText (p_pitch_method));
		MelderInfo_writeLine (U"Pitch very accurate: ", p_pitch_veryAccurate);
		MelderInfo_writeLine (U"Pitch max. number of candidates: ", p_pitch_maximumNumberOfCandidates);
		MelderInfo_writeLine (U"Pitch silence threshold: ", p_pitch_silenceThreshold, U" of global peak");
		MelderInfo_writeLine (U"Pitch voicing threshold: ", p_pitch_voicingThreshold, U" (periodic power / total power)");
		MelderInfo_writeLine (U"Pitch octave cost: ", p_pitch_octaveCost, U" per octave");
		MelderInfo_writeLine (U"Pitch octave jump cost: ", p_pitch_octaveJumpCost, U" per octave");
		MelderInfo_writeLine (U"Pitch voiced/unvoiced cost: ", p_pitch_voicedUnvoicedCost);
	}
	if (v_hasIntensity ()) {
		/* Intensity flag: */
		MelderInfo_writeLine (U"Intensity show: ", p_intensity_show);
		/* Intensity settings: */
		MelderInfo_writeLine (U"Intensity view from: ", p_intensity_viewFrom, U" dB");
		MelderInfo_writeLine (U"Intensity view to: ", p_intensity_viewTo, U" dB");
		MelderInfo_writeLine (U"Intensity averaging method: ", kTimeSoundAnalysisEditor_intensity_averagingMethod_getText (p_intensity_averagingMethod));
		MelderInfo_writeLine (U"Intensity subtract mean pressure: ", p_intensity_subtractMeanPressure);
	}
	if (v_hasFormants ()) {
		/* Formant flag: */
		MelderInfo_writeLine (U"Formant show: ", p_formant_show);
		/* Formant settings: */
		MelderInfo_writeLine (U"Formant maximum formant: ", p_formant_maximumFormant, U" Hz");
		MelderInfo_writeLine (U"Formant number of poles: ", Melder_integer (2 * p_formant_numberOfFormants));   // should be a whole number
		MelderInfo_writeLine (U"Formant window length: ", p_formant_windowLength, U" seconds");
		MelderInfo_writeLine (U"Formant dynamic range: ", p_formant_dynamicRange, U" dB");
		MelderInfo_writeLine (U"Formant dot size: ", p_formant_dotSize, U" mm");
		/* Advanced formant settings: */
		MelderInfo_writeLine (U"Formant method: ", kTimeSoundAnalysisEditor_formant_analysisMethod_getText (p_formant_method));
		MelderInfo_writeLine (U"Formant pre-emphasis from: ", p_formant_preemphasisFrom, U" Hz");
	}
	if (v_hasPulses ()) {
		/* Pulses flag: */
		MelderInfo_writeLine (U"Pulses show: ", p_pulses_show);
		MelderInfo_writeLine (U"Pulses maximum period factor: ", p_pulses_maximumPeriodFactor);
		MelderInfo_writeLine (U"Pulses maximum amplitude factor: ", p_pulses_maximumAmplitudeFactor);
	}
}

void structTimeSoundAnalysisEditor :: v_destroy_analysis () {
	forget (d_spectrogram);
	forget (d_pitch);
	forget (d_intensity);
	forget (d_formant);
	forget (d_pulses);
}

enum {
	TimeSoundAnalysisEditor_PART_CURSOR = 1,
	TimeSoundAnalysisEditor_PART_SELECTION = 2
};

static const char32 *TimeSoundAnalysisEditor_partString (int part) {
	static const char32 *strings [] = { U"", U"CURSOR", U"SELECTION" };
	return strings [part];
}

static const char32 *TimeSoundAnalysisEditor_partString_locative (int part) {
	static const char32 *strings [] = { U"", U"at CURSOR", U"in SELECTION" };
	return strings [part];
}

static int makeQueriable (TimeSoundAnalysisEditor me, int allowCursor, double *tmin, double *tmax) {
	if (my d_endWindow - my d_startWindow > my p_longestAnalysis) {
		Melder_throw (U"Window too long to show analyses. Zoom in to at most ", Melder_half (my p_longestAnalysis), U" seconds "
			U"or set the \"longest analysis\" to at least ", Melder_half (my d_endWindow - my d_startWindow), U" seconds.");
	}
	if (my d_startSelection == my d_endSelection) {
		if (allowCursor) {
			*tmin = *tmax = my d_startSelection;
			return TimeSoundAnalysisEditor_PART_CURSOR;
		} else {
			Melder_throw (U"Make a selection first.");
		}
	} else if (my d_startSelection < my d_startWindow || my d_endSelection > my d_endWindow) {
		Melder_throw (U"Command ambiguous: a part of the selection (", my d_startSelection, U", ", my d_endSelection, U") "
			U"is outside of the window (", my d_startWindow, U", ", my d_endWindow, U"). "
			U"Either zoom or re-select.");
	}
	*tmin = my d_startSelection;
	*tmax = my d_endSelection;
	return TimeSoundAnalysisEditor_PART_SELECTION;
}

static void menu_cb_logSettings (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Log settings", U"Log files")
		OPTIONMENU (U"Write log 1 to", 3)
			OPTION (U"Log file only")
			OPTION (U"Info window only")
			OPTION (U"Log file and Info window")
		LABEL (U"", U"Log file 1:")
		TEXTFIELD (U"Log file 1",   my default_log1_fileName ())
		LABEL (U"", U"Log 1 format:")
		TEXTFIELD (U"Log 1 format", my default_log1_format   ())
		OPTIONMENU (U"Write log 2 to", 3)
			OPTION (U"Log file only")
			OPTION (U"Info window only")
			OPTION (U"Log file and Info window")
		LABEL (U"", U"Log file 2:")
		TEXTFIELD (U"Log file 2",   my default_log2_fileName ())
		LABEL (U"", U"Log 2 format:")
		TEXTFIELD (U"Log 2 format", my default_log2_format   ())
		LABEL (U"", U"Log script 3:")
		TEXTFIELD (U"Log script 3", my default_logScript3    ())
		LABEL (U"", U"Log script 4:")
		TEXTFIELD (U"Log script 4", my default_logScript4    ())
	EDITOR_OK
		SET_INTEGER (U"Write log 1 to", my p_log1_toLogFile + 2 * my p_log1_toInfoWindow)
		SET_STRING (U"Log file 1",      my p_log1_fileName)
		SET_STRING (U"Log 1 format",    my p_log1_format)
		SET_INTEGER (U"Write log 2 to", my p_log2_toLogFile + 2 * my p_log2_toInfoWindow)
		SET_STRING (U"Log file 2",      my p_log2_fileName)
		SET_STRING (U"Log 2 format",    my p_log2_format)
		SET_STRING (U"Log script 3",    my p_logScript3)
		SET_STRING (U"Log script 4",    my p_logScript4)
	EDITOR_DO
		my pref_log1_toLogFile    () = my p_log1_toLogFile    = (GET_INTEGER (U"Write log 1 to") & 1) != 0;
		my pref_log1_toInfoWindow () = my p_log1_toInfoWindow = (GET_INTEGER (U"Write log 1 to") & 2) != 0;
		pref_str32cpy2 (my pref_log1_fileName (), my p_log1_fileName, GET_STRING (U"Log file 1"));
		pref_str32cpy2 (my pref_log1_format   (), my p_log1_format,   GET_STRING (U"Log 1 format"));
		my pref_log2_toLogFile    () = my p_log2_toLogFile    = (GET_INTEGER (U"Write log 2 to") & 1) != 0;
		my pref_log2_toInfoWindow () = my p_log2_toInfoWindow = (GET_INTEGER (U"Write log 2 to") & 2) != 0;
		pref_str32cpy2 (my pref_log2_fileName (), my p_log2_fileName, GET_STRING (U"Log file 2"));
		pref_str32cpy2 (my pref_log2_format   (), my p_log2_format,   GET_STRING (U"Log 2 format"));
		pref_str32cpy2 (my pref_logScript3    (), my p_logScript3,    GET_STRING (U"Log script 3"));
		pref_str32cpy2 (my pref_logScript4    (), my p_logScript4,    GET_STRING (U"Log script 4"));
	EDITOR_END
}

static void menu_cb_deleteLogFile1 (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	structMelderFile file = { 0 };
	Melder_pathToFile (my p_log1_fileName, & file);
	MelderFile_delete (& file);
}

static void menu_cb_deleteLogFile2 (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	structMelderFile file = { 0 };
	Melder_pathToFile (my p_log2_fileName, & file);
	MelderFile_delete (& file);
}

static void do_log (TimeSoundAnalysisEditor me, int which) {
	char32 format [Preferences_STRING_BUFFER_SIZE], *p;
	double tmin, tmax;
	int part = makeQueriable (me, TRUE, & tmin, & tmax);
	str32cpy (format, which == 1 ? my p_log1_format : my p_log2_format);
	for (p = format; *p != U'\0'; p ++) if (*p == U'\'') {
		/*
		 * Found a left quote. Search for a matching right quote.
		 */
		char32 *q = p + 1, varName [300], *r, *s, *colon;
		int precision = -1;
		double value = NUMundefined;
		const char32 *stringValue = NULL;
		while (*q != U'\0' && *q != U'\'') q ++;
		if (*q == U'\0') break;   /* No matching right quote: done with this line. */
		if (q - p == 1) continue;   /* Ignore empty variable names. */
		/*
		 * Found a right quote. Get potential variable name.
		 */
		for (r = p + 1, s = varName; q - r > 0; r ++, s ++) *s = *r;
		*s = U'\0';   /* Trailing null byte. */
		colon = str32chr (varName, U':');
		if (colon) {
			precision = Melder_atoi (colon + 1);
			*colon = U'\0';
		}
		if (str32equ (varName, U"time")) {
			value = 0.5 * (tmin + tmax);
		} else if (str32equ (varName, U"t1")) {
			value = tmin;
		} else if (str32equ (varName, U"t2")) {
			value = tmax;
		} else if (str32equ (varName, U"dur")) {
			value = tmax - tmin;
		} else if (str32equ (varName, U"freq")) {
			value = my d_spectrogram_cursor;
		} else if (str32equ (varName, U"tab$")) {
			stringValue = U"\t";
		} else if (str32equ (varName, U"editor$")) {
			stringValue = my name;
		} else if (str32equ (varName, U"f0")) {
			if (! my p_pitch_show)
				Melder_throw (U"No pitch contour is visible.\nFirst choose \"Show pitch\" from the Pitch menu.");
			if (! my d_pitch) {
				Melder_throw (theMessage_Cannot_compute_pitch);
			}
			if (part == TimeSoundAnalysisEditor_PART_CURSOR) {
				value = Pitch_getValueAtTime (my d_pitch, tmin, my p_pitch_unit, 1);
			} else {
				value = Pitch_getMean (my d_pitch, tmin, tmax, my p_pitch_unit);
			}
		} else if (varName [0] == 'f' && varName [1] >= '1' && varName [1] <= '5' && varName [2] == '\0') {
			if (! my p_formant_show)
				Melder_throw (U"No formant contour is visible.\nFirst choose \"Show formants\" from the Formant menu.");
			if (! my d_formant) {
				Melder_throw (theMessage_Cannot_compute_formant);
			}
			if (part == TimeSoundAnalysisEditor_PART_CURSOR) {
				value = Formant_getValueAtTime (my d_formant, (int) (varName [1] - U'0'), tmin, 0);
			} else {
				value = Formant_getMean (my d_formant, (int) (varName [1] - U'0'), tmin, tmax, 0);
			}
		} else if (varName [0] == U'b' && varName [1] >= U'1' && varName [1] <= U'5' && varName [2] == U'\0') {
			if (! my p_formant_show)
				Melder_throw (U"No formant contour is visible.\nFirst choose \"Show formants\" from the Formant menu.");
			if (! my d_formant) {
				Melder_throw (theMessage_Cannot_compute_formant);
			}
			value = Formant_getBandwidthAtTime (my d_formant, (int) (varName [1] - U'0'), 0.5 * (tmin + tmax), 0);
		} else if (str32equ (varName, U"intensity")) {
			if (! my p_intensity_show)
				Melder_throw (U"No intensity contour is visible.\nFirst choose \"Show intensity\" from the Intensity menu.");
			if (! my d_intensity) {
				Melder_throw (theMessage_Cannot_compute_intensity);
			}
			if (part == TimeSoundAnalysisEditor_PART_CURSOR) {
				value = Vector_getValueAtX (my d_intensity, tmin, Vector_CHANNEL_1, Vector_VALUE_INTERPOLATION_LINEAR);
			} else {
				value = Intensity_getAverage (my d_intensity, tmin, tmax, my p_intensity_averagingMethod);
			}
		} else if (str32equ (varName, U"power")) {
			if (! my p_spectrogram_show)
				Melder_throw (U"No spectrogram is visible.\nFirst choose \"Show spectrogram\" from the Spectrum menu.");
			if (! my d_spectrogram) {
				Melder_throw (theMessage_Cannot_compute_spectrogram);
			}
			if (part != TimeSoundAnalysisEditor_PART_CURSOR) Melder_throw (U"Click inside the spectrogram first.");
			value = Matrix_getValueAtXY (my d_spectrogram, tmin, my d_spectrogram_cursor);
		}
		if (NUMdefined (value)) {
			int varlen = (q - p) - 1, headlen = p - format;
			char32 formattedNumber [400];
			if (precision >= 0) {
				Melder_sprint (formattedNumber,400, Melder_fixed (value, precision));
			} else {
				Melder_sprint (formattedNumber,400, value);
			}
			int arglen = str32len (formattedNumber);
			static MelderString buffer { 0 };
			MelderString_ncopy (& buffer, format, headlen);
			MelderString_append (& buffer, formattedNumber, p + varlen + 2);
			str32cpy (format, buffer.string);
			p += arglen - 1;
		} else if (stringValue != NULL) {
			int varlen = (q - p) - 1, headlen = p - format, arglen = str32len (stringValue);
			static MelderString buffer { 0 };
			MelderString_ncopy (& buffer, format, headlen);
			MelderString_append (& buffer, stringValue, p + varlen + 2);
			str32cpy (format, buffer.string);
			p += arglen - 1;
		} else {
			p = q - 1;   /* Go to before next quote. */
		}
	}
	if ((which == 1 && my p_log1_toInfoWindow) || (which == 2 && my p_log2_toInfoWindow)) {
		MelderInfo_write (format);
		MelderInfo_close ();
	}
	if ((which == 1 && my p_log1_toLogFile) || (which == 2 && my p_log2_toLogFile)) {
		structMelderFile file = { 0 };
		str32cpy (format + str32len (format), U"\n");
		Melder_relativePathToFile (which == 1 ? my p_log1_fileName : my p_log2_fileName, & file);
		MelderFile_appendText (& file, format);
	}
}

static void menu_cb_log1 (EDITOR_ARGS) { EDITOR_IAM (TimeSoundAnalysisEditor); do_log (me, 1); }
static void menu_cb_log2 (EDITOR_ARGS) { EDITOR_IAM (TimeSoundAnalysisEditor); do_log (me, 2); }

static void menu_cb_logScript3 (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	DO_RunTheScriptFromAnyAddedEditorCommand (me, my p_logScript3);
}
static void menu_cb_logScript4 (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	DO_RunTheScriptFromAnyAddedEditorCommand (me, my p_logScript4);
}

static void menu_cb_showAnalyses (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Show analyses", 0)
		BOOLEAN  (U"Show spectrogram",     my default_spectrogram_show ())
		BOOLEAN  (U"Show pitch",           my default_pitch_show       ())
		BOOLEAN  (U"Show intensity",       my default_intensity_show   ())
		BOOLEAN  (U"Show formants",        my default_formant_show     ())
		BOOLEAN  (U"Show pulses",          my default_pulses_show      ())
		POSITIVE (U"Longest analysis (s)", my default_longestAnalysis  ())
	EDITOR_OK
		SET_INTEGER (U"Show spectrogram", my p_spectrogram_show)
		SET_INTEGER (U"Show pitch",       my p_pitch_show)
		SET_INTEGER (U"Show intensity",   my p_intensity_show)
		SET_INTEGER (U"Show formants",    my p_formant_show)
		SET_INTEGER (U"Show pulses",      my p_pulses_show)
		SET_REAL    (U"Longest analysis", my p_longestAnalysis)
	EDITOR_DO
		GuiMenuItem_check (my spectrogramToggle , my pref_spectrogram_show () = my p_spectrogram_show = GET_INTEGER (U"Show spectrogram"));
		GuiMenuItem_check (my pitchToggle       , my pref_pitch_show       () = my p_pitch_show       = GET_INTEGER (U"Show pitch"));
		GuiMenuItem_check (my intensityToggle   , my pref_intensity_show   () = my p_intensity_show   = GET_INTEGER (U"Show intensity"));
		GuiMenuItem_check (my formantToggle     , my pref_formant_show     () = my p_formant_show     = GET_INTEGER (U"Show formants"));
		GuiMenuItem_check (my pulsesToggle      , my pref_pulses_show      () = my p_pulses_show      = GET_INTEGER (U"Show pulses"));
		my pref_longestAnalysis () = my p_longestAnalysis = GET_REAL (U"Longest analysis");
		FunctionEditor_redraw (me);
	EDITOR_END
}

static void menu_cb_timeStepSettings (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Time step settings", U"Time step settings...")
		OPTIONMENU_ENUM (U"Time step strategy", kTimeSoundAnalysisEditor_timeStepStrategy, my default_timeStepStrategy ())
		LABEL (U"", U"")
		LABEL (U"", U"If the time step strategy is \"fixed\":")
		POSITIVE (U"Fixed time step (s)", my default_fixedTimeStep ())
		LABEL (U"", U"")
		LABEL (U"", U"If the time step strategy is \"view-dependent\":")
		NATURAL (U"Number of time steps per view", my default_numberOfTimeStepsPerView ())
	EDITOR_OK
		SET_ENUM (U"Time step strategy", kTimeSoundAnalysisEditor_timeStepStrategy, my p_timeStepStrategy)
		SET_REAL (U"Fixed time step", my p_fixedTimeStep)
		SET_INTEGER (U"Number of time steps per view", my p_numberOfTimeStepsPerView)
	EDITOR_DO
		my pref_timeStepStrategy         () = my p_timeStepStrategy         = GET_ENUM    (kTimeSoundAnalysisEditor_timeStepStrategy, U"Time step strategy");
		my pref_fixedTimeStep            () = my p_fixedTimeStep            = GET_REAL    (U"Fixed time step");
		my pref_numberOfTimeStepsPerView () = my p_numberOfTimeStepsPerView = GET_INTEGER (U"Number of time steps per view");
		forget (my d_pitch);
		forget (my d_formant);
		forget (my d_intensity);
		forget (my d_pulses);
		FunctionEditor_redraw (me);
	EDITOR_END
}

/***** SPECTROGRAM MENU *****/

static void menu_cb_showSpectrogram (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	my pref_spectrogram_show () = my p_spectrogram_show = ! my p_spectrogram_show;
	GuiMenuItem_check (my spectrogramToggle, my p_spectrogram_show);   // in case we're called from a script
	FunctionEditor_redraw (me);
}

static void menu_cb_spectrogramSettings (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Spectrogram settings", U"Intro 3.2. Configuring the spectrogram")
		REAL     (U"left View range (Hz)",  my default_spectrogram_viewFrom     ())
		POSITIVE (U"right View range (Hz)", my default_spectrogram_viewTo       ())
		POSITIVE (U"Window length (s)",     my default_spectrogram_windowLength ())
		POSITIVE (U"Dynamic range (dB)",    my default_spectrogram_dynamicRange ())
		LABEL    (U"note1", U"")
		LABEL    (U"note2", U"")
	EDITOR_OK
		SET_REAL (U"left View range",  my p_spectrogram_viewFrom)
		SET_REAL (U"right View range", my p_spectrogram_viewTo)
		SET_REAL (U"Window length",    my p_spectrogram_windowLength)
		SET_REAL (U"Dynamic range",    my p_spectrogram_dynamicRange)
		if (my p_spectrogram_timeSteps          != Melder_atof (my default_spectrogram_timeSteps ()) ||
			my p_spectrogram_frequencySteps     != Melder_atof (my default_spectrogram_frequencySteps ()) ||
			my p_spectrogram_method             != my default_spectrogram_method () ||
			my p_spectrogram_windowShape        != my default_spectrogram_windowShape () ||
			my p_spectrogram_maximum            != Melder_atof (my default_spectrogram_maximum ()) ||
			my p_spectrogram_autoscaling        != my default_spectrogram_autoscaling ()||
			my p_spectrogram_preemphasis        != Melder_atof (my default_spectrogram_preemphasis ()) ||
			my p_spectrogram_dynamicCompression != Melder_atof (my default_spectrogram_dynamicCompression ()))
		{
			SET_STRING (U"note1", U"Warning: you have non-standard \"advanced settings\".")
		} else {
			SET_STRING (U"note1", U"(all of your \"advanced settings\" have their standard values)")
		}
		if (my p_timeStepStrategy != my default_timeStepStrategy ()) {
			SET_STRING (U"note2", U"Warning: you have a non-standard \"time step strategy\".")
		} else {
			SET_STRING (U"note2", U"(your \"time step strategy\" has its standard value: automatic)")
		}
	EDITOR_DO
		my pref_spectrogram_viewFrom     () = my p_spectrogram_viewFrom     = GET_REAL (U"left View range");
		my pref_spectrogram_viewTo       () = my p_spectrogram_viewTo       = GET_REAL (U"right View range");
		my pref_spectrogram_windowLength () = my p_spectrogram_windowLength = GET_REAL (U"Window length");
		my pref_spectrogram_dynamicRange () = my p_spectrogram_dynamicRange = GET_REAL (U"Dynamic range");
		forget (my d_spectrogram);
		FunctionEditor_redraw (me);
	EDITOR_END
}

static void menu_cb_advancedSpectrogramSettings (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Advanced spectrogram settings", U"Advanced spectrogram settings...")
		LABEL (U"", U"Time and frequency resolutions:")
		NATURAL (U"Number of time steps",      my default_spectrogram_timeSteps          ())
		NATURAL (U"Number of frequency steps", my default_spectrogram_frequencySteps     ())
		LABEL (U"", U"Spectrogram analysis settings:")
		OPTIONMENU_ENUM (U"Method",       kSound_to_Spectrogram_method,      my default_spectrogram_method ())
		OPTIONMENU_ENUM (U"Window shape", kSound_to_Spectrogram_windowShape, my default_spectrogram_windowShape ())
		LABEL (U"", U"Spectrogram view settings:")
		BOOLEAN (U"Autoscaling",               my default_spectrogram_autoscaling        ())
		REAL    (U"Maximum (dB/Hz)",           my default_spectrogram_maximum            ())
		REAL    (U"Pre-emphasis (dB/oct)",     my default_spectrogram_preemphasis        ())
		REAL    (U"Dynamic compression (0-1)", my default_spectrogram_dynamicCompression ())
	EDITOR_OK
		SET_INTEGER (U"Number of time steps",      my p_spectrogram_timeSteps)
		SET_INTEGER (U"Number of frequency steps", my p_spectrogram_frequencySteps)
		SET_ENUM    (U"Method",       kSound_to_Spectrogram_method,      my p_spectrogram_method)
		SET_ENUM    (U"Window shape", kSound_to_Spectrogram_windowShape, my p_spectrogram_windowShape)
		SET_INTEGER (U"Autoscaling",               my p_spectrogram_autoscaling)
		SET_REAL    (U"Maximum",                   my p_spectrogram_maximum)
		SET_REAL    (U"Pre-emphasis",              my p_spectrogram_preemphasis)
		SET_REAL    (U"Dynamic compression",       my p_spectrogram_dynamicCompression)
	EDITOR_DO
		my pref_spectrogram_timeSteps          () = my p_spectrogram_timeSteps          = GET_INTEGER (U"Number of time steps");
		my pref_spectrogram_frequencySteps     () = my p_spectrogram_frequencySteps     = GET_INTEGER (U"Number of frequency steps");
		my pref_spectrogram_method             () = my p_spectrogram_method             = GET_ENUM (kSound_to_Spectrogram_method, U"Method");
		my pref_spectrogram_windowShape        () = my p_spectrogram_windowShape        = GET_ENUM (kSound_to_Spectrogram_windowShape, U"Window shape");
		my pref_spectrogram_autoscaling        () = my p_spectrogram_autoscaling        = GET_INTEGER (U"Autoscaling");
		my pref_spectrogram_maximum            () = my p_spectrogram_maximum            = GET_REAL (U"Maximum");
		my pref_spectrogram_preemphasis        () = my p_spectrogram_preemphasis        = GET_REAL (U"Pre-emphasis");
		my pref_spectrogram_dynamicCompression () = my p_spectrogram_dynamicCompression = GET_REAL (U"Dynamic compression");
		forget (my d_spectrogram);
		FunctionEditor_redraw (me);
	EDITOR_END
}

static void menu_cb_getFrequency (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	Melder_informationReal (my d_spectrogram_cursor, U"Hz");
}

static void menu_cb_getSpectralPowerAtCursorCross (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	double tmin, tmax;
	int part = makeQueriable (me, TRUE, & tmin, & tmax);
	if (! my p_spectrogram_show)
		Melder_throw (U"No spectrogram is visible.\nFirst choose \"Show spectrogram\" from the Spectrum menu.");
	if (! my d_spectrogram) {
		TimeSoundAnalysisEditor_computeSpectrogram (me);
		if (! my d_spectrogram) Melder_throw (theMessage_Cannot_compute_spectrogram);
	}
	if (part != TimeSoundAnalysisEditor_PART_CURSOR) Melder_throw (U"Click inside the spectrogram first.");
	MelderInfo_open ();
	MelderInfo_write (Matrix_getValueAtXY (my d_spectrogram, tmin, my d_spectrogram_cursor),
		U" Pa2/Hz (at time = ", tmin, U" seconds and frequency = ", my d_spectrogram_cursor, U" Hz)");
	MelderInfo_close ();
}

static void menu_cb_moveFrequencyCursorTo (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	if (! my p_spectrogram_show)
		Melder_throw (U"No spectrogram is visible.\nFirst choose \"Show spectrogram\" from the Spectrum menu.");
	EDITOR_FORM (U"Move frequency cursor to", 0)
		REAL (U"Frequency (Hz)", U"0.0")
	EDITOR_OK
		SET_REAL (U"Frequency", my d_spectrogram_cursor)
	EDITOR_DO
		double frequency = GET_REAL (U"Frequency");
		my d_spectrogram_cursor = frequency;
		FunctionEditor_redraw (me);
	EDITOR_END
}

static Sound extractSound (TimeSoundAnalysisEditor me, double tmin, double tmax) {
	Sound sound = NULL;
	if (my d_longSound.data) {
		if (tmin < my d_longSound.data -> xmin) tmin = my d_longSound.data -> xmin;
		if (tmax > my d_longSound.data -> xmax) tmax = my d_longSound.data -> xmax;
		sound = LongSound_extractPart (my d_longSound.data, tmin, tmax, TRUE);
	} else if (my d_sound.data) {
		if (tmin < my d_sound.data -> xmin) tmin = my d_sound.data -> xmin;
		if (tmax > my d_sound.data -> xmax) tmax = my d_sound.data -> xmax;
		sound = Sound_extractPart (my d_sound.data, tmin, tmax, kSound_windowShape_RECTANGULAR, 1.0, TRUE);
	}
	return sound;
}

static void menu_cb_extractVisibleSpectrogram (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	if (! my p_spectrogram_show)
		Melder_throw (U"No spectrogram is visible.\nFirst choose \"Show spectrogram\" from the Spectrum menu.");
	if (! my d_spectrogram) {
		TimeSoundAnalysisEditor_computeSpectrogram (me);
		if (! my d_spectrogram) Melder_throw (theMessage_Cannot_compute_spectrogram);
	}
	autoSpectrogram publish = Data_copy (my d_spectrogram);
	Editor_broadcastPublication (me, publish.transfer());
}

static void menu_cb_viewSpectralSlice (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	double start = my d_startSelection == my d_endSelection ?
		my p_spectrogram_windowShape == kSound_to_Spectrogram_windowShape_GAUSSIAN ? my d_startSelection - my p_spectrogram_windowLength :
		my d_startSelection - my p_spectrogram_windowLength / 2 : my d_startSelection;
	double finish = my d_startSelection == my d_endSelection ?
		my p_spectrogram_windowShape == kSound_to_Spectrogram_windowShape_GAUSSIAN ? my d_endSelection + my p_spectrogram_windowLength :
		my d_endSelection + my p_spectrogram_windowLength / 2 : my d_endSelection;
	autoSound sound = extractSound (me, start, finish);
	Sound_multiplyByWindow (sound.peek(),
		my p_spectrogram_windowShape == kSound_to_Spectrogram_windowShape_SQUARE ? kSound_windowShape_RECTANGULAR :
		my p_spectrogram_windowShape == kSound_to_Spectrogram_windowShape_HAMMING ? kSound_windowShape_HAMMING :
		my p_spectrogram_windowShape == kSound_to_Spectrogram_windowShape_BARTLETT ? kSound_windowShape_TRIANGULAR :
		my p_spectrogram_windowShape == kSound_to_Spectrogram_windowShape_WELCH ? kSound_windowShape_PARABOLIC :
		my p_spectrogram_windowShape == kSound_to_Spectrogram_windowShape_HANNING ? kSound_windowShape_HANNING :
		my p_spectrogram_windowShape == kSound_to_Spectrogram_windowShape_GAUSSIAN ? kSound_windowShape_GAUSSIAN_2 : kSound_windowShape_RECTANGULAR);
	autoSpectrum publish = Sound_to_Spectrum (sound.peek(), TRUE);
	Thing_setName (publish.peek(), Melder_cat (( my data == NULL ? U"untitled" : ((Data) my data) -> name ),
		U"_", Melder_fixed (0.5 * (my d_startSelection + my d_endSelection), 3)));
	Editor_broadcastPublication (me, publish.transfer());
}

static void menu_cb_paintVisibleSpectrogram (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Paint visible spectrogram", 0)
		my v_form_pictureWindow (cmd);
		my v_form_pictureMargins (cmd);
		my v_form_pictureSelection (cmd);
		BOOLEAN (U"Garnish", 1);
	EDITOR_OK
		my v_ok_pictureWindow (cmd);
		my v_ok_pictureMargins (cmd);
		my v_ok_pictureSelection (cmd);
		SET_INTEGER (U"Garnish", my p_spectrogram_picture_garnish);
	EDITOR_DO
		my v_do_pictureWindow (cmd);
		my v_do_pictureMargins (cmd);
		my v_do_pictureSelection (cmd);
		my pref_spectrogram_picture_garnish () = my p_spectrogram_picture_garnish = GET_INTEGER (U"Garnish");
		if (! my p_spectrogram_show)
			Melder_throw (U"No spectrogram is visible.\nFirst choose \"Show spectrogram\" from the Spectrum menu.");
		if (! my d_spectrogram) {
			TimeSoundAnalysisEditor_computeSpectrogram (me);
			if (! my d_spectrogram) Melder_throw (theMessage_Cannot_compute_spectrogram);
		}
		Editor_openPraatPicture (me);
		Spectrogram_paint (my d_spectrogram, my pictureGraphics, my d_startWindow, my d_endWindow, my p_spectrogram_viewFrom, my p_spectrogram_viewTo,
			my p_spectrogram_maximum, my p_spectrogram_autoscaling, my p_spectrogram_dynamicRange, my p_spectrogram_preemphasis,
			my p_spectrogram_dynamicCompression, my p_spectrogram_picture_garnish);
		FunctionEditor_garnish (me);
		Editor_closePraatPicture (me);
	EDITOR_END
}

/***** PITCH MENU *****/

static void menu_cb_showPitch (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	my pref_pitch_show () = my p_pitch_show = ! my p_pitch_show;
	GuiMenuItem_check (my pitchToggle, my p_pitch_show);   // in case we're called from a script
	FunctionEditor_redraw (me);
}

static void menu_cb_pitchSettings (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Pitch settings", U"Intro 4.2. Configuring the pitch contour")
		POSITIVE (U"left Pitch range (Hz)", my default_pitch_floor ())
		POSITIVE (U"right Pitch range (Hz)", my default_pitch_ceiling ())
		OPTIONMENU_ENUM (U"Unit", kPitch_unit, my default_pitch_unit ())
		LABEL (U"opt1", U"The autocorrelation method optimizes for intonation research;")
		LABEL (U"opt2", U"and the cross-correlation method optimizes for voice research:")
		RADIO_ENUM (U"Analysis method", kTimeSoundAnalysisEditor_pitch_analysisMethod, my default_pitch_method ())
		OPTIONMENU_ENUM (U"Drawing method", kTimeSoundAnalysisEditor_pitch_drawingMethod, my default_pitch_drawingMethod ())
		LABEL (U"note1", U"")
		LABEL (U"note2", U"")
	EDITOR_OK
		SET_REAL (U"left Pitch range", my p_pitch_floor)
		SET_REAL (U"right Pitch range", my p_pitch_ceiling)
		SET_ENUM (U"Unit", kPitch_unit, my p_pitch_unit)
		SET_ENUM (U"Analysis method", kTimeSoundAnalysisEditor_pitch_analysisMethod, my p_pitch_method)
		SET_ENUM (U"Drawing method", kTimeSoundAnalysisEditor_pitch_drawingMethod, my p_pitch_drawingMethod)
		if (my p_pitch_viewFrom                  != Melder_atof (my default_pitch_viewFrom ()) ||
			my p_pitch_viewTo                    != Melder_atof (my default_pitch_viewTo ()) ||
			my p_pitch_veryAccurate              != my default_pitch_veryAccurate () ||
			my p_pitch_maximumNumberOfCandidates != Melder_atof (my default_pitch_maximumNumberOfCandidates ()) ||
			my p_pitch_silenceThreshold          != Melder_atof (my default_pitch_silenceThreshold ()) ||
			my p_pitch_voicingThreshold          != Melder_atof (my default_pitch_voicingThreshold ()) ||
			my p_pitch_octaveCost                != Melder_atof (my default_pitch_octaveCost ()) ||
			my p_pitch_octaveJumpCost            != Melder_atof (my default_pitch_octaveJumpCost ()) ||
			my p_pitch_voicedUnvoicedCost        != Melder_atof (my default_pitch_voicedUnvoicedCost ()))
		{
			SET_STRING (U"note1", U"Warning: you have some non-standard \"advanced settings\".")
		} else {
			SET_STRING (U"note1", U"(all of your \"advanced settings\" have their standard values)")
		}
		if (my p_timeStepStrategy != my default_timeStepStrategy ()) {
			SET_STRING (U"note2", U"Warning: you have a non-standard \"time step strategy\".")
		} else {
			SET_STRING (U"note2", U"(your \"time step strategy\" has its standard value: automatic)")
		}
	EDITOR_DO
		my pref_pitch_floor         () = my p_pitch_floor         = GET_REAL (U"left Pitch range");
		my pref_pitch_ceiling       () = my p_pitch_ceiling       = GET_REAL (U"right Pitch range");
		my pref_pitch_unit          () = my p_pitch_unit          = GET_ENUM (kPitch_unit, U"Unit");
		my pref_pitch_method        () = my p_pitch_method        = GET_ENUM (kTimeSoundAnalysisEditor_pitch_analysisMethod, U"Analysis method");
		my pref_pitch_drawingMethod () = my p_pitch_drawingMethod = GET_ENUM (kTimeSoundAnalysisEditor_pitch_drawingMethod, U"Drawing method");
		forget (my d_pitch);
		forget (my d_intensity);
		forget (my d_pulses);
		FunctionEditor_redraw (me);
	EDITOR_END
}

static void menu_cb_advancedPitchSettings (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Advanced pitch settings", U"Advanced pitch settings...")
		LABEL   (U"", U"Make view range different from analysis range:")
		REAL    (U"left View range (units)",   my default_pitch_viewFrom                  ())
		REAL    (U"right View range (units)",  my default_pitch_viewTo                    ())
		LABEL   (U"", U"Analysis settings:")
		BOOLEAN (U"Very accurate", 0)
		NATURAL (U"Max. number of candidates", my default_pitch_maximumNumberOfCandidates ())
		REAL    (U"Silence threshold",         my default_pitch_silenceThreshold          ())
		REAL    (U"Voicing threshold",         my default_pitch_voicingThreshold          ())
		REAL    (U"Octave cost",               my default_pitch_octaveCost                ())
		REAL    (U"Octave-jump cost",          my default_pitch_octaveJumpCost            ())
		REAL    (U"Voiced / unvoiced cost",    my default_pitch_voicedUnvoicedCost        ())
	EDITOR_OK
		SET_REAL    (U"left View range",           my p_pitch_viewFrom)
		SET_REAL    (U"right View range",          my p_pitch_viewTo)
		SET_INTEGER (U"Very accurate",             my p_pitch_veryAccurate)
		SET_INTEGER (U"Max. number of candidates", my p_pitch_maximumNumberOfCandidates)
		SET_REAL    (U"Silence threshold",         my p_pitch_silenceThreshold)
		SET_REAL    (U"Voicing threshold",         my p_pitch_voicingThreshold)
		SET_REAL    (U"Octave cost",               my p_pitch_octaveCost)
		SET_REAL    (U"Octave-jump cost",          my p_pitch_octaveJumpCost)
		SET_REAL    (U"Voiced / unvoiced cost",    my p_pitch_voicedUnvoicedCost)
	EDITOR_DO
		long maxnCandidates = GET_INTEGER (U"Max. number of candidates");
		if (maxnCandidates < 2) Melder_throw (U"Maximum number of candidates must be greater than 1.");
		my pref_pitch_viewFrom                  () = my p_pitch_viewFrom                  = GET_REAL    (U"left View range");
		my pref_pitch_viewTo                    () = my p_pitch_viewTo                    = GET_REAL    (U"right View range");
		my pref_pitch_veryAccurate              () = my p_pitch_veryAccurate              = GET_INTEGER (U"Very accurate");
		my pref_pitch_maximumNumberOfCandidates () = my p_pitch_maximumNumberOfCandidates = GET_INTEGER (U"Max. number of candidates");
		my pref_pitch_silenceThreshold          () = my p_pitch_silenceThreshold          = GET_REAL    (U"Silence threshold");
		my pref_pitch_voicingThreshold          () = my p_pitch_voicingThreshold          = GET_REAL    (U"Voicing threshold");
		my pref_pitch_octaveCost                () = my p_pitch_octaveCost                = GET_REAL    (U"Octave cost");
		my pref_pitch_octaveJumpCost            () = my p_pitch_octaveJumpCost            = GET_REAL    (U"Octave-jump cost");
		my pref_pitch_voicedUnvoicedCost        () = my p_pitch_voicedUnvoicedCost        = GET_REAL    (U"Voiced / unvoiced cost");
		forget (my d_pitch);
		forget (my d_intensity);
		forget (my d_pulses);
		FunctionEditor_redraw (me);
	EDITOR_END
}

static void menu_cb_pitchListing (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	double tmin, tmax;
	int part = makeQueriable (me, TRUE, & tmin, & tmax);
	if (! my p_pitch_show)
		Melder_throw (U"No pitch contour is visible.\nFirst choose \"Show pitch\" from the Pitch menu.");
	if (! my d_pitch) {
		TimeSoundAnalysisEditor_computePitch (me);
		if (! my d_pitch) Melder_throw (theMessage_Cannot_compute_pitch);
	}
	MelderInfo_open ();
	MelderInfo_writeLine (U"Time_s   F0_", Function_getUnitText (my d_pitch, Pitch_LEVEL_FREQUENCY, my p_pitch_unit, Function_UNIT_TEXT_SHORT));
	if (part == TimeSoundAnalysisEditor_PART_CURSOR) {
		double f0 = Pitch_getValueAtTime (my d_pitch, tmin, my p_pitch_unit, TRUE);
		f0 = Function_convertToNonlogarithmic (my d_pitch, f0, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
		MelderInfo_writeLine (Melder_fixed (tmin, 6), U"   ", Melder_fixed (f0, 6));
	} else {
		long i, i1, i2;
		Sampled_getWindowSamples (my d_pitch, tmin, tmax, & i1, & i2);
		for (i = i1; i <= i2; i ++) {
			double t = Sampled_indexToX (my d_pitch, i);
			double f0 = Sampled_getValueAtSample (my d_pitch, i, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
			f0 = Function_convertToNonlogarithmic (my d_pitch, f0, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
			MelderInfo_writeLine (Melder_fixed (t, 6), U"   ", Melder_fixed (f0, 6));
		}
	}
	MelderInfo_close ();
}

static void menu_cb_getPitch (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	double tmin, tmax;
	int part = makeQueriable (me, TRUE, & tmin, & tmax);
	if (! my p_pitch_show)
		Melder_throw (U"No pitch contour is visible.\nFirst choose \"Show pitch\" from the Pitch menu.");
	if (! my d_pitch) {
		TimeSoundAnalysisEditor_computePitch (me);
		if (! my d_pitch) Melder_throw (theMessage_Cannot_compute_pitch);
	}
	if (part == TimeSoundAnalysisEditor_PART_CURSOR) {
		double f0 = Pitch_getValueAtTime (my d_pitch, tmin, my p_pitch_unit, TRUE);
		f0 = Function_convertToNonlogarithmic (my d_pitch, f0, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
		Melder_information (f0, U" ", Function_getUnitText (my d_pitch, Pitch_LEVEL_FREQUENCY, my p_pitch_unit, 0),
			U" (interpolated pitch at CURSOR)");
	} else {
		double f0 = Pitch_getMean (my d_pitch, tmin, tmax, my p_pitch_unit);
		f0 = Function_convertToNonlogarithmic (my d_pitch, f0, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
		Melder_information (f0, U" ", Function_getUnitText (my d_pitch, Pitch_LEVEL_FREQUENCY, my p_pitch_unit, 0),
			U" (mean pitch ", TimeSoundAnalysisEditor_partString_locative (part), U")");
	}
}

static void menu_cb_getMinimumPitch (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	double tmin, tmax, f0;
	int part = makeQueriable (me, FALSE, & tmin, & tmax);
	if (! my p_pitch_show)
		Melder_throw (U"No pitch contour is visible.\nFirst choose \"Show pitch\" from the Pitch menu.");
	if (! my d_pitch) {
		TimeSoundAnalysisEditor_computePitch (me);
		if (! my d_pitch) Melder_throw (theMessage_Cannot_compute_pitch);
	}
	f0 = Pitch_getMinimum (my d_pitch, tmin, tmax, my p_pitch_unit, TRUE);
	f0 = Function_convertToNonlogarithmic (my d_pitch, f0, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
	Melder_information (f0, U" ", Function_getUnitText (my d_pitch, Pitch_LEVEL_FREQUENCY, my p_pitch_unit, 0),
		U" (minimum pitch ", TimeSoundAnalysisEditor_partString_locative (part), U")");
}

static void menu_cb_getMaximumPitch (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	double tmin, tmax, f0;
	int part = makeQueriable (me, FALSE, & tmin, & tmax);
	if (! my p_pitch_show)
		Melder_throw (U"No pitch contour is visible.\nFirst choose \"Show pitch\" from the Pitch menu.");
	if (! my d_pitch) {
		TimeSoundAnalysisEditor_computePitch (me);
		if (! my d_pitch) Melder_throw (theMessage_Cannot_compute_pitch);   // BUG
	}
	f0 = Pitch_getMaximum (my d_pitch, tmin, tmax, my p_pitch_unit, TRUE);
	f0 = Function_convertToNonlogarithmic (my d_pitch, f0, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
	Melder_information (f0, U" ", Function_getUnitText (my d_pitch, Pitch_LEVEL_FREQUENCY, my p_pitch_unit, 0),
		U" (maximum pitch ", TimeSoundAnalysisEditor_partString_locative (part), U")");
}

static void menu_cb_moveCursorToMinimumPitch (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	if (! my p_pitch_show)
		Melder_throw (U"No pitch contour is visible.\nFirst choose \"Show pitch\" from the View menu.");
	if (! my d_pitch) {
		TimeSoundAnalysisEditor_computePitch (me);
		if (! my d_pitch) Melder_throw (theMessage_Cannot_compute_pitch);
	}
	if (my d_startSelection == my d_endSelection) {
		Melder_throw (U"Empty selection.");
	} else {
		double time;
		Pitch_getMinimumAndTime (my d_pitch, my d_startSelection, my d_endSelection,
			my p_pitch_unit, 1, NULL, & time);
		if (! NUMdefined (time))
			Melder_throw (U"Selection is voiceless.");
		my d_startSelection = my d_endSelection = time;
		FunctionEditor_marksChanged (me, true);
	}
}

static void menu_cb_moveCursorToMaximumPitch (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	if (! my p_pitch_show)
		Melder_throw (U"No pitch contour is visible.\nFirst choose \"Show pitch\" from the View menu.");
	if (! my d_pitch) {
		TimeSoundAnalysisEditor_computePitch (me);
		if (! my d_pitch) Melder_throw (theMessage_Cannot_compute_pitch);
	}
	if (my d_startSelection == my d_endSelection) {
		Melder_throw (U"Empty selection.");
	} else {
		double time;
		Pitch_getMaximumAndTime (my d_pitch, my d_startSelection, my d_endSelection,
			my p_pitch_unit, 1, NULL, & time);
		if (! NUMdefined (time))
			Melder_throw (U"Selection is voiceless.");
		my d_startSelection = my d_endSelection = time;
		FunctionEditor_marksChanged (me, true);
	}
}

static void menu_cb_extractVisiblePitchContour (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	if (! my p_pitch_show)
		Melder_throw (U"No pitch contour is visible.\nFirst choose \"Show pitch\" from the Pitch menu.");
	if (! my d_pitch) {
		TimeSoundAnalysisEditor_computePitch (me);
		if (! my d_pitch) Melder_throw (theMessage_Cannot_compute_pitch);
	}
	autoPitch publish = Data_copy (my d_pitch);
	Editor_broadcastPublication (me, publish.transfer());
}

static void menu_cb_drawVisiblePitchContour (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Draw visible pitch contour", 0)
		my v_form_pictureWindow (cmd);
		LABEL (U"", U"Pitch:")
		BOOLEAN (U"Speckle", 0);
		my v_form_pictureMargins (cmd);
		my v_form_pictureSelection (cmd);
		BOOLEAN (U"Garnish", 1);
	EDITOR_OK
		my v_ok_pictureWindow (cmd);
		SET_INTEGER (U"Speckle", my p_pitch_picture_speckle);
		my v_ok_pictureMargins (cmd);
		my v_ok_pictureSelection (cmd);
		SET_INTEGER (U"Garnish", my p_pitch_picture_garnish);
	EDITOR_DO
		my v_do_pictureWindow (cmd);
		my pref_pitch_picture_speckle () = my p_pitch_picture_speckle = GET_INTEGER (U"Speckle");
		my v_do_pictureMargins (cmd);
		my v_do_pictureSelection (cmd);
		my pref_pitch_picture_garnish () = my p_pitch_picture_garnish = GET_INTEGER (U"Garnish");
		if (! my p_pitch_show)
			Melder_throw (U"No pitch contour is visible.\nFirst choose \"Show pitch\" from the Pitch menu.");
		if (! my d_pitch) {
			TimeSoundAnalysisEditor_computePitch (me);
			if (! my d_pitch) Melder_throw (theMessage_Cannot_compute_pitch);
		}
		Editor_openPraatPicture (me);
		double pitchFloor_hidden = Function_convertStandardToSpecialUnit (my d_pitch, my p_pitch_floor, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
		double pitchCeiling_hidden = Function_convertStandardToSpecialUnit (my d_pitch, my p_pitch_ceiling, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
		double pitchFloor_overt = Function_convertToNonlogarithmic (my d_pitch, pitchFloor_hidden, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
		double pitchCeiling_overt = Function_convertToNonlogarithmic (my d_pitch, pitchCeiling_hidden, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
		double pitchViewFrom_overt = my p_pitch_viewFrom < my p_pitch_viewTo ? my p_pitch_viewFrom : pitchFloor_overt;
		double pitchViewTo_overt = my p_pitch_viewFrom < my p_pitch_viewTo ? my p_pitch_viewTo : pitchCeiling_overt;
		Pitch_draw (my d_pitch, my pictureGraphics, my d_startWindow, my d_endWindow, pitchViewFrom_overt, pitchViewTo_overt,
			my p_pitch_picture_garnish, my p_pitch_picture_speckle, my p_pitch_unit);
		FunctionEditor_garnish (me);
		Editor_closePraatPicture (me);
	EDITOR_END
}

/***** INTENSITY MENU *****/

static void menu_cb_showIntensity (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	my pref_intensity_show () = my p_intensity_show = ! my p_intensity_show;
	GuiMenuItem_check (my intensityToggle, my p_intensity_show);   // in case we're called from a script
	FunctionEditor_redraw (me);
}

static void menu_cb_intensitySettings (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Intensity settings", U"Intro 6.2. Configuring the intensity contour")
		REAL (U"left View range (dB)",  my default_intensity_viewFrom ())
		REAL (U"right View range (dB)", my default_intensity_viewTo   ())
		RADIO_ENUM (U"Averaging method", kTimeSoundAnalysisEditor_intensity_averagingMethod, my default_intensity_averagingMethod ())
		BOOLEAN (U"Subtract mean pressure", my default_intensity_subtractMeanPressure ())
		LABEL (U"", U"Note: the pitch floor is taken from the pitch settings.")
		LABEL (U"note2", U"")
	EDITOR_OK
		SET_REAL (U"left View range",  my p_intensity_viewFrom)
		SET_REAL (U"right View range", my p_intensity_viewTo)
		SET_ENUM (U"Averaging method", kTimeSoundAnalysisEditor_intensity_averagingMethod, my p_intensity_averagingMethod)
		SET_INTEGER (U"Subtract mean pressure", my p_intensity_subtractMeanPressure)
		if (my p_timeStepStrategy != my default_timeStepStrategy ()) {
			SET_STRING (U"note2", U"Warning: you have a non-standard \"time step strategy\".")
		} else {
			SET_STRING (U"note2", U"(your \"time step strategy\" has its standard value: automatic)")
		}
	EDITOR_DO
		my pref_intensity_viewFrom             () = my p_intensity_viewFrom             = GET_REAL    (U"left View range");
		my pref_intensity_viewTo               () = my p_intensity_viewTo               = GET_REAL    (U"right View range");
		my pref_intensity_averagingMethod      () = my p_intensity_averagingMethod      = GET_ENUM    (kTimeSoundAnalysisEditor_intensity_averagingMethod, U"Averaging method");
		my pref_intensity_subtractMeanPressure () = my p_intensity_subtractMeanPressure = GET_INTEGER (U"Subtract mean pressure");
		forget (my d_intensity);
		FunctionEditor_redraw (me);
	EDITOR_END
}

static void menu_cb_extractVisibleIntensityContour (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	if (! my p_intensity_show)
		Melder_throw (U"No intensity contour is visible.\nFirst choose \"Show intensity\" from the Intensity menu.");
	if (! my d_intensity) {
		TimeSoundAnalysisEditor_computeIntensity (me);
		if (! my d_intensity) Melder_throw (theMessage_Cannot_compute_intensity);
	}
	autoIntensity publish = Data_copy (my d_intensity);
	Editor_broadcastPublication (me, publish.transfer());
}

static void menu_cb_drawVisibleIntensityContour (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Draw visible intensity contour", 0)
		my v_form_pictureWindow (cmd);
		my v_form_pictureMargins (cmd);
		my v_form_pictureSelection (cmd);
		BOOLEAN (U"Garnish", 1);
	EDITOR_OK
		my v_ok_pictureWindow (cmd);
		my v_ok_pictureMargins (cmd);
		my v_ok_pictureSelection (cmd);
		SET_INTEGER (U"Garnish", my p_intensity_picture_garnish);
	EDITOR_DO
		my v_do_pictureWindow (cmd);
		my v_do_pictureMargins (cmd);
		my v_do_pictureSelection (cmd);
		my pref_intensity_picture_garnish () = my p_intensity_picture_garnish = GET_INTEGER (U"Garnish");
		if (! my p_intensity_show)
			Melder_throw (U"No intensity contour is visible.\nFirst choose \"Show intensity\" from the Intensity menu.");
		if (! my d_intensity) {
			TimeSoundAnalysisEditor_computeIntensity (me);
			if (! my d_intensity) Melder_throw (theMessage_Cannot_compute_intensity);
		}
		Editor_openPraatPicture (me);
		Intensity_draw (my d_intensity, my pictureGraphics, my d_startWindow, my d_endWindow, my p_intensity_viewFrom, my p_intensity_viewTo,
			my p_intensity_picture_garnish);
		FunctionEditor_garnish (me);
		Editor_closePraatPicture (me);
	EDITOR_END
}

static void menu_cb_intensityListing (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	double tmin, tmax;
	int part = makeQueriable (me, TRUE, & tmin, & tmax);
	if (! my p_intensity_show)
		Melder_throw (U"No intensity contour is visible.\nFirst choose \"Show intensity\" from the Intensity menu.");
	if (! my d_intensity) {
		TimeSoundAnalysisEditor_computeIntensity (me);
		if (! my d_intensity) Melder_throw (theMessage_Cannot_compute_intensity);
	}
	MelderInfo_open ();
	MelderInfo_writeLine (U"Time_s   Intensity_dB");
	if (part == TimeSoundAnalysisEditor_PART_CURSOR) {
		double intensity = Vector_getValueAtX (my d_intensity, tmin, Vector_CHANNEL_1, Vector_VALUE_INTERPOLATION_LINEAR);
		MelderInfo_writeLine (Melder_fixed (tmin, 6), U"   ", Melder_fixed (intensity, 6));
	} else {
		long i, i1, i2;
		Sampled_getWindowSamples (my d_intensity, tmin, tmax, & i1, & i2);
		for (i = i1; i <= i2; i ++) {
			double t = Sampled_indexToX (my d_intensity, i);
			double intensity = Vector_getValueAtX (my d_intensity, t, Vector_CHANNEL_1, Vector_VALUE_INTERPOLATION_NEAREST);
			MelderInfo_writeLine (Melder_fixed (t, 6), U"   ", Melder_fixed (intensity, 6));
		}
	}
	MelderInfo_close ();
}

static void menu_cb_getIntensity (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	double tmin, tmax;
	int part = makeQueriable (me, TRUE, & tmin, & tmax);
	if (! my p_intensity_show)
		Melder_throw (U"No intensity contour is visible.\nFirst choose \"Show intensity\" from the Intensity menu.");
	if (! my d_intensity) {
		TimeSoundAnalysisEditor_computeIntensity (me);
		if (! my d_intensity) Melder_throw (theMessage_Cannot_compute_intensity);
	}
	if (part == TimeSoundAnalysisEditor_PART_CURSOR) {
		Melder_information (Vector_getValueAtX (my d_intensity, tmin, Vector_CHANNEL_1, Vector_VALUE_INTERPOLATION_LINEAR), U" dB (intensity at CURSOR)");
	} else {
		static const char32 *methodString [] = { U"median", U"mean-energy", U"mean-sones", U"mean-dB" };
		Melder_information (Intensity_getAverage (my d_intensity, tmin, tmax, my p_intensity_averagingMethod),
			U" dB (", methodString [my p_intensity_averagingMethod], U" intensity ", TimeSoundAnalysisEditor_partString_locative (part), U")");
	}
}

static void menu_cb_getMinimumIntensity (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	double tmin, tmax;
	int part = makeQueriable (me, FALSE, & tmin, & tmax);
	if (! my p_intensity_show)
		Melder_throw (U"No intensity contour is visible.\nFirst choose \"Show intensity\" from the Intensity menu.");
	if (! my d_intensity) {
		TimeSoundAnalysisEditor_computeIntensity (me);
		if (! my d_intensity) Melder_throw (theMessage_Cannot_compute_intensity);
	}
	double intensity = Vector_getMinimum (my d_intensity, tmin, tmax, NUM_PEAK_INTERPOLATE_PARABOLIC);
	Melder_information (intensity, U" dB (minimum intensity ", TimeSoundAnalysisEditor_partString_locative (part), U")");
}

static void menu_cb_getMaximumIntensity (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	double tmin, tmax;
	int part = makeQueriable (me, FALSE, & tmin, & tmax);
	if (! my p_intensity_show)
		Melder_throw (U"No intensity contour is visible.\nFirst choose \"Show intensity\" from the Intensity menu.");
	if (! my d_intensity) {
		TimeSoundAnalysisEditor_computeIntensity (me);
		if (! my d_intensity) Melder_throw (theMessage_Cannot_compute_intensity);
	}
	double intensity = Vector_getMaximum (my d_intensity, tmin, tmax, NUM_PEAK_INTERPOLATE_PARABOLIC);
	Melder_information (intensity, U" dB (maximum intensity ", TimeSoundAnalysisEditor_partString_locative (part), U")");
}

/***** FORMANT MENU *****/

static void menu_cb_showFormants (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	my pref_formant_show () = my p_formant_show = ! my p_formant_show;
	GuiMenuItem_check (my formantToggle, my p_formant_show);   // in case we're called from a script
	FunctionEditor_redraw (me);
}

static void menu_cb_formantSettings (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Formant settings", U"Intro 5.2. Configuring the formant contours")
		POSITIVE (U"Maximum formant (Hz)", my default_formant_maximumFormant   ())
		POSITIVE (U"Number of formants",   my default_formant_numberOfFormants ())
		POSITIVE (U"Window length (s)",    my default_formant_windowLength     ())
		REAL     (U"Dynamic range (dB)",   my default_formant_dynamicRange     ())
		POSITIVE (U"Dot size (mm)",        my default_formant_dotSize          ())
		LABEL    (U"note1", U"")
		LABEL    (U"note2", U"")
	EDITOR_OK
		SET_REAL (U"Maximum formant",    my p_formant_maximumFormant)
		SET_REAL (U"Number of formants", my p_formant_numberOfFormants)
		SET_REAL (U"Window length",      my p_formant_windowLength)
		SET_REAL (U"Dynamic range",      my p_formant_dynamicRange)
		SET_REAL (U"Dot size",           my p_formant_dotSize)
		if (my p_formant_method != my default_formant_method () || my p_formant_preemphasisFrom != Melder_atof (my default_formant_preemphasisFrom ())) {
			SET_STRING (U"note1", U"Warning: you have non-standard \"advanced settings\".")
		} else {
			SET_STRING (U"note1", U"(all of your \"advanced settings\" have their standard values)")
		}
		if (my p_timeStepStrategy != my default_timeStepStrategy ()) {
			SET_STRING (U"note2", U"Warning: you have a non-standard \"time step strategy\".")
		} else {
			SET_STRING (U"note2", U"(your \"time step strategy\" has its standard value: automatic)")
		}
	EDITOR_DO
		my pref_formant_maximumFormant   () = my p_formant_maximumFormant   = GET_REAL (U"Maximum formant");
		my pref_formant_numberOfFormants () = my p_formant_numberOfFormants = GET_REAL (U"Number of formants");
		my pref_formant_windowLength     () = my p_formant_windowLength     = GET_REAL (U"Window length");
		my pref_formant_dynamicRange     () = my p_formant_dynamicRange     = GET_REAL (U"Dynamic range");
		my pref_formant_dotSize          () = my p_formant_dotSize          = GET_REAL (U"Dot size");
		forget (my d_formant);
		FunctionEditor_redraw (me);
	EDITOR_END
}

static void menu_cb_advancedFormantSettings (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Advanced formant settings", U"Advanced formant settings...")
		RADIO_ENUM (U"Method", kTimeSoundAnalysisEditor_formant_analysisMethod, my default_formant_method ())
		POSITIVE (U"Pre-emphasis from (Hz)", my default_formant_preemphasisFrom ())
	EDITOR_OK
		SET_ENUM (U"Method", kTimeSoundAnalysisEditor_formant_analysisMethod, my p_formant_method)
		SET_REAL (U"Pre-emphasis from", my p_formant_preemphasisFrom)
	EDITOR_DO
		my pref_formant_method          () = my p_formant_method          = GET_ENUM (kTimeSoundAnalysisEditor_formant_analysisMethod, U"Method");
		my pref_formant_preemphasisFrom () = my p_formant_preemphasisFrom = GET_REAL (U"Pre-emphasis from");
		forget (my d_formant);
		FunctionEditor_redraw (me);
	EDITOR_END
}

static void menu_cb_extractVisibleFormantContour (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	if (! my p_formant_show)
		Melder_throw (U"No formant contour is visible.\nFirst choose \"Show formants\" from the Formant menu.");
	if (! my d_formant) {
		TimeSoundAnalysisEditor_computeFormants (me);
		if (! my d_formant) Melder_throw (theMessage_Cannot_compute_formant);
	}
	autoFormant publish = Data_copy (my d_formant);
	Editor_broadcastPublication (me, publish.transfer());
}

static void menu_cb_drawVisibleFormantContour (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Draw visible formant contour", 0)
		my v_form_pictureWindow (cmd);
		my v_form_pictureMargins (cmd);
		my v_form_pictureSelection (cmd);
		BOOLEAN (U"Garnish", 1);
	EDITOR_OK
		my v_ok_pictureWindow (cmd);
		my v_ok_pictureMargins (cmd);
		my v_ok_pictureSelection (cmd);
		SET_INTEGER (U"Garnish", my p_formant_picture_garnish);
	EDITOR_DO
		my v_do_pictureWindow (cmd);
		my v_do_pictureMargins (cmd);
		my v_do_pictureSelection (cmd);
		my pref_formant_picture_garnish () = my p_formant_picture_garnish = GET_INTEGER (U"Garnish");
		if (! my p_formant_show)
			Melder_throw (U"No formant contour is visible.\nFirst choose \"Show formant\" from the Formant menu.");
		if (! my d_formant) {
			TimeSoundAnalysisEditor_computeFormants (me);
			if (! my d_formant) Melder_throw (theMessage_Cannot_compute_formant);
		}
		Editor_openPraatPicture (me);
		Formant_drawSpeckles (my d_formant, my pictureGraphics, my d_startWindow, my d_endWindow,
			my p_spectrogram_viewTo, my p_formant_dynamicRange,
			my p_formant_picture_garnish);
		FunctionEditor_garnish (me);
		Editor_closePraatPicture (me);
	EDITOR_END
}

static void menu_cb_formantListing (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	double tmin, tmax;
	int part = makeQueriable (me, TRUE, & tmin, & tmax);
	if (! my p_formant_show)
		Melder_throw (U"No formant contour is visible.\nFirst choose \"Show formants\" from the Formant menu.");
	if (! my d_formant) {
		TimeSoundAnalysisEditor_computeFormants (me);
		if (! my d_formant) Melder_throw (theMessage_Cannot_compute_formant);
	}
	MelderInfo_open ();
	MelderInfo_writeLine (U"Time_s   F1_Hz   F2_Hz   F3_Hz   F4_Hz");
	if (part == TimeSoundAnalysisEditor_PART_CURSOR) {
		double f1 = Formant_getValueAtTime (my d_formant, 1, tmin, 0);
		double f2 = Formant_getValueAtTime (my d_formant, 2, tmin, 0);
		double f3 = Formant_getValueAtTime (my d_formant, 3, tmin, 0);
		double f4 = Formant_getValueAtTime (my d_formant, 4, tmin, 0);
		MelderInfo_writeLine (Melder_fixed (tmin, 6), U"   ", Melder_fixed (f1, 6), U"   ", Melder_fixed (f2, 6), U"   ", Melder_fixed (f3, 6), U"   ", Melder_fixed (f4, 6));
	} else {
		long i, i1, i2;
		Sampled_getWindowSamples (my d_formant, tmin, tmax, & i1, & i2);
		for (i = i1; i <= i2; i ++) {
			double t = Sampled_indexToX (my d_formant, i);
			double f1 = Formant_getValueAtTime (my d_formant, 1, t, 0);
			double f2 = Formant_getValueAtTime (my d_formant, 2, t, 0);
			double f3 = Formant_getValueAtTime (my d_formant, 3, t, 0);
			double f4 = Formant_getValueAtTime (my d_formant, 4, t, 0);
			MelderInfo_writeLine (Melder_fixed (t, 6), U"   ", Melder_fixed (f1, 6), U"   ", Melder_fixed (f2, 6), U"   ", Melder_fixed (f3, 6), U"   ", Melder_fixed (f4, 6));
		}
	}
	MelderInfo_close ();
}

static void do_getFormant (TimeSoundAnalysisEditor me, int iformant) {
	double tmin, tmax;
	int part = makeQueriable (me, TRUE, & tmin, & tmax);
	if (! my p_formant_show)
		Melder_throw (U"No formant contour is visible.\nFirst choose \"Show formants\" from the Formant menu.");
	if (! my d_formant) {
		TimeSoundAnalysisEditor_computeFormants (me);
		if (! my d_formant) Melder_throw (theMessage_Cannot_compute_formant);
	}
	if (part == TimeSoundAnalysisEditor_PART_CURSOR) {
		Melder_information (Formant_getValueAtTime (my d_formant, iformant, tmin, 0),
			U" Hz (nearest F", iformant, U" to CURSOR)");
	} else {
		Melder_information (Formant_getMean (my d_formant, iformant, tmin, tmax, 0),
			U" Hz (mean F", iformant, U" ", TimeSoundAnalysisEditor_partString_locative (part), U")");
	}
}
static void do_getBandwidth (TimeSoundAnalysisEditor me, int iformant) {
	double tmin, tmax;
	int part = makeQueriable (me, TRUE, & tmin, & tmax);
	if (! my p_formant_show)
		Melder_throw (U"No formant contour is visible.\nFirst choose \"Show formants\" from the Formant menu.");
	if (! my d_formant) {
		TimeSoundAnalysisEditor_computeFormants (me);
		if (! my d_formant) Melder_throw (theMessage_Cannot_compute_formant);
	}
	if (part == TimeSoundAnalysisEditor_PART_CURSOR) {
		Melder_information (Formant_getBandwidthAtTime (my d_formant, iformant, tmin, 0),
			U" Hz (nearest B", iformant, U" to CURSOR)");
	} else {
		Melder_information (Formant_getBandwidthAtTime (my d_formant, iformant, 0.5 * (tmin + tmax), 0),
			U" Hz (B", iformant, U" in centre of ", TimeSoundAnalysisEditor_partString (part), U")");
	}
}
static void menu_cb_getFirstFormant (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor); do_getFormant (me, 1); }
static void menu_cb_getFirstBandwidth (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor); do_getBandwidth (me, 1); }
static void menu_cb_getSecondFormant (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor); do_getFormant (me, 2); }
static void menu_cb_getSecondBandwidth (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor); do_getBandwidth (me, 2); }
static void menu_cb_getThirdFormant (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor); do_getFormant (me, 3); }
static void menu_cb_getThirdBandwidth (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor); do_getBandwidth (me, 3); }
static void menu_cb_getFourthFormant (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor); do_getFormant (me, 4); }
static void menu_cb_getFourthBandwidth (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor); do_getBandwidth (me, 4); }

static void menu_cb_getFormant (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Get formant", 0)
		NATURAL (U"Formant number", U"5")
	EDITOR_OK
	EDITOR_DO
		do_getFormant (me, GET_INTEGER (U"Formant number"));
	EDITOR_END
}

static void menu_cb_getBandwidth (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Get bandwidth", 0)
		NATURAL (U"Formant number", U"5")
	EDITOR_OK
	EDITOR_DO
		do_getBandwidth (me, GET_INTEGER (U"Formant number"));
	EDITOR_END
}

/***** PULSE MENU *****/

static void menu_cb_showPulses (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	my pref_pulses_show () = my p_pulses_show = ! my p_pulses_show;
	GuiMenuItem_check (my pulsesToggle, my p_pulses_show);   // in case we're called from a script
	FunctionEditor_redraw (me);
}

static void menu_cb_advancedPulsesSettings (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Advanced pulses settings", U"Advanced pulses settings...")
		POSITIVE (U"Maximum period factor",    my default_pulses_maximumPeriodFactor    ())
		POSITIVE (U"Maximum amplitude factor", my default_pulses_maximumAmplitudeFactor ())
	EDITOR_OK
		SET_REAL (U"Maximum period factor",    my p_pulses_maximumPeriodFactor)
		SET_REAL (U"Maximum amplitude factor", my p_pulses_maximumAmplitudeFactor)
	EDITOR_DO
		my pref_pulses_maximumPeriodFactor    () = my p_pulses_maximumPeriodFactor    = GET_REAL (U"Maximum period factor");
		my pref_pulses_maximumAmplitudeFactor () = my p_pulses_maximumAmplitudeFactor = GET_REAL (U"Maximum amplitude factor");
		forget (my d_pulses);
		FunctionEditor_redraw (me);
	EDITOR_END
}

static void menu_cb_extractVisiblePulses (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	if (! my p_pulses_show)
		Melder_throw (U"No pulses are visible.\nFirst choose \"Show pulses\" from the Pulses menu.");
	if (! my d_pulses) {
		TimeSoundAnalysisEditor_computePulses (me);
		if (! my d_pulses) Melder_throw (theMessage_Cannot_compute_pulses);
	}
	autoPointProcess publish = Data_copy (my d_pulses);
	Editor_broadcastPublication (me, publish.transfer());
}

static void menu_cb_drawVisiblePulses (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	EDITOR_FORM (U"Draw visible pulses", 0)
		my v_form_pictureWindow (cmd);
		my v_form_pictureMargins (cmd);
		my v_form_pictureSelection (cmd);
		BOOLEAN (U"Garnish", 1);
	EDITOR_OK
		my v_ok_pictureWindow (cmd);
		my v_ok_pictureMargins (cmd);
		my v_ok_pictureSelection (cmd);
		SET_INTEGER (U"Garnish", my p_pulses_picture_garnish);
	EDITOR_DO
		my v_do_pictureWindow (cmd);
		my v_do_pictureMargins (cmd);
		my v_do_pictureSelection (cmd);
		my pref_pulses_picture_garnish () = my p_pulses_picture_garnish = GET_INTEGER (U"Garnish");
		if (! my p_pulses_show)
			Melder_throw (U"No pulses are visible.\nFirst choose \"Show pulses\" from the Pulses menu.");
		if (! my d_pulses) {
			TimeSoundAnalysisEditor_computePulses (me);
			if (! my d_pulses) Melder_throw (theMessage_Cannot_compute_pulses);
		}
		Editor_openPraatPicture (me);
		PointProcess_draw (my d_pulses, my pictureGraphics, my d_startWindow, my d_endWindow,
			my p_pulses_picture_garnish);
		FunctionEditor_garnish (me);
		Editor_closePraatPicture (me);
	EDITOR_END
}

static void menu_cb_voiceReport (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	time_t today = time (NULL);
	double tmin, tmax;
	int part = makeQueriable (me, FALSE, & tmin, & tmax);
	if (! my p_pulses_show)
		Melder_throw (U"No pulses are visible.\nFirst choose \"Show pulses\" from the Pulses menu.");
	if (! my d_pulses) {
		TimeSoundAnalysisEditor_computePulses (me);
		if (! my d_pulses) Melder_throw (theMessage_Cannot_compute_pulses);
	}
	autoSound sound = extractSound (me, tmin, tmax);
	MelderInfo_open ();
	MelderInfo_writeLine (U"-- Voice report for ", my name, U" --\nDate: ", Melder_peek8to32 (ctime (& today)));
	if (my p_pitch_method != kTimeSoundAnalysisEditor_pitch_analysisMethod_CROSS_CORRELATION)
		MelderInfo_writeLine (U"WARNING: some of the following measurements may be imprecise.\n"
			"For more precision, go to \"Pitch settings\" and choose \"Optimize for voice analysis\".\n");
	MelderInfo_writeLine (U"Time range of ", TimeSoundAnalysisEditor_partString (part));
	Sound_Pitch_PointProcess_voiceReport (sound.peek(), my d_pitch, my d_pulses, tmin, tmax,
		my p_pitch_floor, my p_pitch_ceiling, my p_pulses_maximumPeriodFactor, my p_pulses_maximumAmplitudeFactor, my p_pitch_silenceThreshold, my p_pitch_voicingThreshold);
	MelderInfo_close ();
}

static void menu_cb_pulseListing (EDITOR_ARGS) {
	EDITOR_IAM (TimeSoundAnalysisEditor);
	long i, i1, i2;
	double tmin, tmax;
	makeQueriable (me, FALSE, & tmin, & tmax);
	if (! my p_pulses_show)
		Melder_throw (U"No pulses are visible.\nFirst choose \"Show pulses\" from the Pulses menu.");
	if (! my d_pulses) {
		TimeSoundAnalysisEditor_computePulses (me);
		if (! my d_pulses) Melder_throw (theMessage_Cannot_compute_pulses);
	}
	MelderInfo_open ();
	MelderInfo_writeLine (U"Time_s");
	i1 = PointProcess_getHighIndex (my d_pulses, tmin);
	i2 = PointProcess_getLowIndex (my d_pulses, tmax);
	for (i = i1; i <= i2; i ++) {
		double t = my d_pulses -> t [i];
		MelderInfo_writeLine (Melder_fixed (t, 12));
	}
	MelderInfo_close ();
}

/*
static void cb_getJitter_xx (TimeSoundAnalysisEditor me, double (*PointProcess_getJitter_xx) (PointProcess, double, double, double, double, double)) {
	double minimumPeriod = 0.8 / my p_pitch_ceiling, maximumPeriod = 1.25 / my p_pitch_floor;
	if (! my p_pulses_show)
		Melder_throw (U"No pulses are visible.\nFirst choose \"Show pulses\" from the Pulses menu.");
	if (! my d_pulses) {
		computePulses (me);
		if (! my d_pulses) Melder_throw (theMessage_Cannot_compute_pulses);
	}
	if (my startSelection == my endSelection)
		Melder_throw (U"Make a selection first.");
	makeQueriable
	Melder_informationReal (PointProcess_getJitter_xx (my d_pulses, my startSelection, my endSelection,
		minimumPeriod, maximumPeriod, my p_pulses_maximumPeriodFactor), NULL);
}
DIRECT (TimeSoundAnalysisEditor, cb_getJitter_local) cb_getJitter_xx (me, PointProcess_getJitter_local); END
DIRECT (TimeSoundAnalysisEditor, cb_getJitter_local_absolute) cb_getJitter_xx (me, PointProcess_getJitter_local_absolute); END
DIRECT (TimeSoundAnalysisEditor, cb_getJitter_rap) cb_getJitter_xx (me, PointProcess_getJitter_rap); END
DIRECT (TimeSoundAnalysisEditor, cb_getJitter_ppq5) cb_getJitter_xx (me, PointProcess_getJitter_ppq5); END
DIRECT (TimeSoundAnalysisEditor, cb_getJitter_ddp) cb_getJitter_xx (me, PointProcess_getJitter_ddp); END

static void cb_getShimmer_xx (TimeSoundAnalysisEditor me, double (*PointProcess_Sound_getShimmer_xx) (PointProcess, Sound, double, double, double, double, double)) {
	double minimumPeriod = 0.8 / my p_pitch_ceiling, maximumPeriod = 1.25 / my p_pitch_floor;
	if (! my p_pulses_show)
		Melder_throw (U"No pulses are visible.\nFirst choose \"Show pulses\" from the Pulses menu.");
	if (! my d_pulses) {
		computePulses (me);
		if (! my d_pulses) Melder_throw (theMessage_Cannot_compute_pulses);
	}
	if (my startSelection == my endSelection)
		Melder_throw (U"Make a selection first.");
	makeQueriable
	autoSound sound = extractSound (me, my startSelection, my endSelection);
	Melder_informationReal (PointProcess_Sound_getShimmer_xx (my d_pulses, sound, my startSelection, my endSelection,
		minimumPeriod, maximumPeriod, my p_pulses_maximumAmplitudeFactor), NULL);
}
DIRECT (TimeSoundAnalysisEditor, cb_getShimmer_local) cb_getShimmer_xx (me, PointProcess_Sound_getShimmer_local); END
DIRECT (TimeSoundAnalysisEditor, cb_getShimmer_local_dB) cb_getShimmer_xx (me, PointProcess_Sound_getShimmer_local_dB); END
DIRECT (TimeSoundAnalysisEditor, cb_getShimmer_apq3) cb_getShimmer_xx (me, PointProcess_Sound_getShimmer_apq3); END
DIRECT (TimeSoundAnalysisEditor, cb_getShimmer_apq5) cb_getShimmer_xx (me, PointProcess_Sound_getShimmer_apq5); END
DIRECT (TimeSoundAnalysisEditor, cb_getShimmer_apq11) cb_getShimmer_xx (me, PointProcess_Sound_getShimmer_apq11); END
DIRECT (TimeSoundAnalysisEditor, cb_getShimmer_dda) cb_getShimmer_xx (me, PointProcess_Sound_getShimmer_dda); END
*/

void structTimeSoundAnalysisEditor :: v_createMenuItems_view_sound (EditorMenu menu) {
	TimeSoundAnalysisEditor_Parent :: v_createMenuItems_view_sound (menu);
	v_createMenuItems_view_sound_analysis (menu);
}

void structTimeSoundAnalysisEditor :: v_createMenuItems_view_sound_analysis (EditorMenu menu) {
	EditorMenu_addCommand (menu, U"Analysis window:", GuiMenu_INSENSITIVE, menu_cb_showAnalyses);
	EditorMenu_addCommand (menu, U"Show analyses...", 0, menu_cb_showAnalyses);
	EditorMenu_addCommand (menu, U"Time step settings...", 0, menu_cb_timeStepSettings);
	EditorMenu_addCommand (menu, U"-- sound analysis --", 0, 0);
}

void structTimeSoundAnalysisEditor :: v_createMenuItems_query (EditorMenu menu) {
	TimeSoundAnalysisEditor_Parent :: v_createMenuItems_query (menu);
	if (d_sound.data || d_longSound.data) {
		v_createMenuItems_query_log (menu);
	}
}

void structTimeSoundAnalysisEditor :: v_createMenuItems_query_log (EditorMenu menu) {
	EditorMenu_addCommand (menu, U"-- query log --", 0, NULL);
	EditorMenu_addCommand (menu, U"Log settings...", 0, menu_cb_logSettings);
	EditorMenu_addCommand (menu, U"Delete log file 1", 0, menu_cb_deleteLogFile1);
	EditorMenu_addCommand (menu, U"Delete log file 2", 0, menu_cb_deleteLogFile2);
	EditorMenu_addCommand (menu, U"Log 1", GuiMenu_F12, menu_cb_log1);
	EditorMenu_addCommand (menu, U"Log 2", GuiMenu_F12 + GuiMenu_SHIFT, menu_cb_log2);
	EditorMenu_addCommand (menu, U"Log script 3 (...)", GuiMenu_F12 + GuiMenu_OPTION, menu_cb_logScript3);
	EditorMenu_addCommand (menu, U"Log script 4 (...)", GuiMenu_F12 + GuiMenu_COMMAND, menu_cb_logScript4);
}

void structTimeSoundAnalysisEditor :: v_createMenus_analysis () {
	EditorMenu menu;

	if (v_hasSpectrogram ()) {
		menu = Editor_addMenu (this, U"Spectrum", 0);
		spectrogramToggle = EditorMenu_addCommand (menu, U"Show spectrogram",
			GuiMenu_CHECKBUTTON | (pref_spectrogram_show () ? GuiMenu_TOGGLE_ON : 0), menu_cb_showSpectrogram);
		EditorMenu_addCommand (menu, U"Spectrogram settings...", 0, menu_cb_spectrogramSettings);
		EditorMenu_addCommand (menu, U"Advanced spectrogram settings...", 0, menu_cb_advancedSpectrogramSettings);
		EditorMenu_addCommand (menu, U"-- spectrum query --", 0, NULL);
		EditorMenu_addCommand (menu, U"Query:", GuiMenu_INSENSITIVE, menu_cb_getFrequency /* dummy */);
		EditorMenu_addCommand (menu, U"Get frequency at frequency cursor", 0, menu_cb_getFrequency);
		EditorMenu_addCommand (menu, U"Get spectral power at cursor cross", GuiMenu_F7, menu_cb_getSpectralPowerAtCursorCross);
		EditorMenu_addCommand (menu, U"-- spectrum select --", 0, NULL);
		EditorMenu_addCommand (menu, U"Select:", GuiMenu_INSENSITIVE, menu_cb_moveFrequencyCursorTo/* dummy */);
		EditorMenu_addCommand (menu, U"Move frequency cursor to...", 0, menu_cb_moveFrequencyCursorTo);
		v_createMenuItems_spectrum_picture (menu);
		EditorMenu_addCommand (menu, U"-- spectrum extract --", 0, NULL);
		EditorMenu_addCommand (menu, U"Extract to objects window:", GuiMenu_INSENSITIVE, menu_cb_extractVisibleSpectrogram /* dummy */);
		EditorMenu_addCommand (menu, U"Extract visible spectrogram", 0, menu_cb_extractVisibleSpectrogram);
		EditorMenu_addCommand (menu, U"View spectral slice", 'L', menu_cb_viewSpectralSlice);
	}

	if (v_hasPitch ()) {
		menu = Editor_addMenu (this, U"Pitch", 0);
		pitchToggle = EditorMenu_addCommand (menu, U"Show pitch",
			GuiMenu_CHECKBUTTON | (pref_pitch_show () ? GuiMenu_TOGGLE_ON : 0), menu_cb_showPitch);
		EditorMenu_addCommand (menu, U"Pitch settings...", 0, menu_cb_pitchSettings);
		EditorMenu_addCommand (menu, U"Advanced pitch settings...", 0, menu_cb_advancedPitchSettings);
		EditorMenu_addCommand (menu, U"-- pitch query --", 0, NULL);
		EditorMenu_addCommand (menu, U"Query:", GuiMenu_INSENSITIVE, menu_cb_getFrequency /* dummy */);
		EditorMenu_addCommand (menu, U"Pitch listing", 0, menu_cb_pitchListing);
		EditorMenu_addCommand (menu, U"Get pitch", GuiMenu_F5, menu_cb_getPitch);
		EditorMenu_addCommand (menu, U"Get minimum pitch", GuiMenu_F5 + GuiMenu_COMMAND, menu_cb_getMinimumPitch);
		EditorMenu_addCommand (menu, U"Get maximum pitch", GuiMenu_F5 + GuiMenu_SHIFT, menu_cb_getMaximumPitch);
		EditorMenu_addCommand (menu, U"-- pitch select --", 0, NULL);
		EditorMenu_addCommand (menu, U"Select:", GuiMenu_INSENSITIVE, menu_cb_moveCursorToMinimumPitch /* dummy */);
		EditorMenu_addCommand (menu, U"Move cursor to minimum pitch", GuiMenu_COMMAND + GuiMenu_SHIFT + 'L', menu_cb_moveCursorToMinimumPitch);
		EditorMenu_addCommand (menu, U"Move cursor to maximum pitch", GuiMenu_COMMAND + GuiMenu_SHIFT + 'H', menu_cb_moveCursorToMaximumPitch);
		v_createMenuItems_pitch_picture (menu);
		EditorMenu_addCommand (menu, U"-- pitch extract --", 0, NULL);
		EditorMenu_addCommand (menu, U"Extract to objects window:", GuiMenu_INSENSITIVE, menu_cb_extractVisiblePitchContour /* dummy */);
		EditorMenu_addCommand (menu, U"Extract visible pitch contour", 0, menu_cb_extractVisiblePitchContour);
	}

	if (v_hasIntensity ()) {
		menu = Editor_addMenu (this, U"Intensity", 0);
		intensityToggle = EditorMenu_addCommand (menu, U"Show intensity",
			GuiMenu_CHECKBUTTON | (pref_intensity_show () ? GuiMenu_TOGGLE_ON : 0), menu_cb_showIntensity);
		EditorMenu_addCommand (menu, U"Intensity settings...", 0, menu_cb_intensitySettings);
		EditorMenu_addCommand (menu, U"-- intensity query --", 0, NULL);
		EditorMenu_addCommand (menu, U"Query:", GuiMenu_INSENSITIVE, menu_cb_getFrequency /* dummy */);
		EditorMenu_addCommand (menu, U"Intensity listing", 0, menu_cb_intensityListing);
		EditorMenu_addCommand (menu, U"Get intensity", GuiMenu_F8, menu_cb_getIntensity);
		EditorMenu_addCommand (menu, U"Get minimum intensity", GuiMenu_F8 + GuiMenu_COMMAND, menu_cb_getMinimumIntensity);
		EditorMenu_addCommand (menu, U"Get maximum intensity", GuiMenu_F8 + GuiMenu_SHIFT, menu_cb_getMaximumIntensity);
		v_createMenuItems_intensity_picture (menu);
		EditorMenu_addCommand (menu, U"-- intensity extract --", 0, NULL);
		EditorMenu_addCommand (menu, U"Extract to objects window:", GuiMenu_INSENSITIVE, menu_cb_extractVisibleIntensityContour /* dummy */);
		EditorMenu_addCommand (menu, U"Extract visible intensity contour", 0, menu_cb_extractVisibleIntensityContour);
	}

	if (v_hasFormants ()) {
		menu = Editor_addMenu (this, U"Formant", 0);
		formantToggle = EditorMenu_addCommand (menu, U"Show formants",
			GuiMenu_CHECKBUTTON | (pref_formant_show () ? GuiMenu_TOGGLE_ON : 0), menu_cb_showFormants);
		EditorMenu_addCommand (menu, U"Formant settings...", 0, menu_cb_formantSettings);
		EditorMenu_addCommand (menu, U"Advanced formant settings...", 0, menu_cb_advancedFormantSettings);
		EditorMenu_addCommand (menu, U"-- formant query --", 0, NULL);
		EditorMenu_addCommand (menu, U"Query:", GuiMenu_INSENSITIVE, menu_cb_getFrequency /* dummy */);
		EditorMenu_addCommand (menu, U"Formant listing", 0, menu_cb_formantListing);
		EditorMenu_addCommand (menu, U"Get first formant", GuiMenu_F1, menu_cb_getFirstFormant);
		EditorMenu_addCommand (menu, U"Get first bandwidth", 0, menu_cb_getFirstBandwidth);
		EditorMenu_addCommand (menu, U"Get second formant", GuiMenu_F2, menu_cb_getSecondFormant);
		EditorMenu_addCommand (menu, U"Get second bandwidth", 0, menu_cb_getSecondBandwidth);
		EditorMenu_addCommand (menu, U"Get third formant", GuiMenu_F3, menu_cb_getThirdFormant);
		EditorMenu_addCommand (menu, U"Get third bandwidth", 0, menu_cb_getThirdBandwidth);
		EditorMenu_addCommand (menu, U"Get fourth formant", GuiMenu_F4, menu_cb_getFourthFormant);
		EditorMenu_addCommand (menu, U"Get fourth bandwidth", 0, menu_cb_getFourthBandwidth);
		EditorMenu_addCommand (menu, U"Get formant...", 0, menu_cb_getFormant);
		EditorMenu_addCommand (menu, U"Get bandwidth...", 0, menu_cb_getBandwidth);
		v_createMenuItems_formant_picture (menu);
		EditorMenu_addCommand (menu, U"-- formant extract --", 0, NULL);
		EditorMenu_addCommand (menu, U"Extract to objects window:", GuiMenu_INSENSITIVE, menu_cb_extractVisibleFormantContour /* dummy */);
		EditorMenu_addCommand (menu, U"Extract visible formant contour", 0, menu_cb_extractVisibleFormantContour);
	}

	if (v_hasPulses ()) {
		menu = Editor_addMenu (this, U"Pulses", 0);
		pulsesToggle = EditorMenu_addCommand (menu, U"Show pulses",
			GuiMenu_CHECKBUTTON | (pref_pulses_show () ? GuiMenu_TOGGLE_ON : 0), menu_cb_showPulses);
		EditorMenu_addCommand (menu, U"Advanced pulses settings...", 0, menu_cb_advancedPulsesSettings);
		EditorMenu_addCommand (menu, U"-- pulses query --", 0, NULL);
		EditorMenu_addCommand (menu, U"Query:", GuiMenu_INSENSITIVE, menu_cb_getFrequency /* dummy */);
		EditorMenu_addCommand (menu, U"Voice report", 0, menu_cb_voiceReport);
		EditorMenu_addCommand (menu, U"Pulse listing", 0, menu_cb_pulseListing);
		/*
		EditorMenu_addCommand (menu, U"Get jitter (local)", 0, cb_getJitter_local);
		EditorMenu_addCommand (menu, U"Get jitter (local, absolute)", 0, cb_getJitter_local_absolute);
		EditorMenu_addCommand (menu, U"Get jitter (rap)", 0, cb_getJitter_rap);
		EditorMenu_addCommand (menu, U"Get jitter (ppq5)", 0, cb_getJitter_ppq5);
		EditorMenu_addCommand (menu, U"Get jitter (ddp)", 0, cb_getJitter_ddp);
		EditorMenu_addCommand (menu, U"Get shimmer (local)", 0, cb_getShimmer_local);
		EditorMenu_addCommand (menu, U"Get shimmer (local_dB)", 0, cb_getShimmer_local_dB);
		EditorMenu_addCommand (menu, U"Get shimmer (apq3)", 0, cb_getShimmer_apq3);
		EditorMenu_addCommand (menu, U"Get shimmer (apq5)", 0, cb_getShimmer_apq5);
		EditorMenu_addCommand (menu, U"Get shimmer (apq11)", 0, cb_getShimmer_apq11);
		EditorMenu_addCommand (menu, U"Get shimmer (dda)", 0, cb_getShimmer_dda);
		*/
		v_createMenuItems_pulses_picture (menu);
		EditorMenu_addCommand (menu, U"-- pulses extract --", 0, NULL);
		EditorMenu_addCommand (menu, U"Extract to objects window:", GuiMenu_INSENSITIVE, menu_cb_extractVisiblePulses /* dummy */);
		EditorMenu_addCommand (menu, U"Extract visible pulses", 0, menu_cb_extractVisiblePulses);
	}
}

void structTimeSoundAnalysisEditor :: v_createMenuItems_spectrum_picture (EditorMenu menu) {
	EditorMenu_addCommand (menu, U"-- spectrum draw --", 0, NULL);
	EditorMenu_addCommand (menu, U"Draw to picture window:", GuiMenu_INSENSITIVE, menu_cb_paintVisibleSpectrogram /* dummy */);
	EditorMenu_addCommand (menu, U"Paint visible spectrogram...", 0, menu_cb_paintVisibleSpectrogram);
}

void structTimeSoundAnalysisEditor :: v_createMenuItems_pitch_picture (EditorMenu menu) {
	EditorMenu_addCommand (menu, U"-- pitch draw --", 0, NULL);
	EditorMenu_addCommand (menu, U"Draw to picture window:", GuiMenu_INSENSITIVE, menu_cb_drawVisiblePitchContour /* dummy */);
	EditorMenu_addCommand (menu, U"Draw visible pitch contour...", 0, menu_cb_drawVisiblePitchContour);
}

void structTimeSoundAnalysisEditor :: v_createMenuItems_intensity_picture (EditorMenu menu) {
	EditorMenu_addCommand (menu, U"-- intensity draw --", 0, NULL);
	EditorMenu_addCommand (menu, U"Draw to picture window:", GuiMenu_INSENSITIVE, menu_cb_drawVisibleIntensityContour /* dummy */);
	EditorMenu_addCommand (menu, U"Draw visible intensity contour...", 0, menu_cb_drawVisibleIntensityContour);
}

void structTimeSoundAnalysisEditor :: v_createMenuItems_formant_picture (EditorMenu menu) {
	EditorMenu_addCommand (menu, U"-- formant draw --", 0, NULL);
	EditorMenu_addCommand (menu, U"Draw to picture window:", GuiMenu_INSENSITIVE, menu_cb_drawVisibleFormantContour /* dummy */);
	EditorMenu_addCommand (menu, U"Draw visible formant contour...", 0, menu_cb_drawVisibleFormantContour);
}

void structTimeSoundAnalysisEditor :: v_createMenuItems_pulses_picture (EditorMenu menu) {
	EditorMenu_addCommand (menu, U"-- pulses draw --", 0, NULL);
	EditorMenu_addCommand (menu, U"Draw to picture window:", GuiMenu_INSENSITIVE, menu_cb_drawVisiblePulses /* dummy */);
	EditorMenu_addCommand (menu, U"Draw visible pulses...", 0, menu_cb_drawVisiblePulses);
}

void TimeSoundAnalysisEditor_computeSpectrogram (TimeSoundAnalysisEditor me) {
	autoMelderProgressOff progress;
	if (my p_spectrogram_show && my d_endWindow - my d_startWindow <= my p_longestAnalysis &&
		(my d_spectrogram == NULL || my d_spectrogram -> xmin != my d_startWindow || my d_spectrogram -> xmax != my d_endWindow))
	{
		double margin = my p_spectrogram_windowShape == kSound_to_Spectrogram_windowShape_GAUSSIAN ? my p_spectrogram_windowLength : 0.5 * my p_spectrogram_windowLength;
		forget (my d_spectrogram);
		try {
			autoSound sound = extractSound (me, my d_startWindow - margin, my d_endWindow + margin);
			my d_spectrogram = Sound_to_Spectrogram (sound.peek(), my p_spectrogram_windowLength,
				my p_spectrogram_viewTo, (my d_endWindow - my d_startWindow) / my p_spectrogram_timeSteps,
				my p_spectrogram_viewTo / my p_spectrogram_frequencySteps, my p_spectrogram_windowShape, 8.0, 8.0);
			my d_spectrogram -> xmin = my d_startWindow;
			my d_spectrogram -> xmax = my d_endWindow;
		} catch (MelderError) {
			Melder_clearError ();
		}
	}
}

static void computePitch_inside (TimeSoundAnalysisEditor me) {
	double margin = my p_pitch_veryAccurate ? 3.0 / my p_pitch_floor : 1.5 / my p_pitch_floor;
	forget (my d_pitch);
	try {
		autoSound sound = extractSound (me, my d_startWindow - margin, my d_endWindow + margin);
		double pitchTimeStep =
			my p_timeStepStrategy == kTimeSoundAnalysisEditor_timeStepStrategy_FIXED ? my p_fixedTimeStep :
			my p_timeStepStrategy == kTimeSoundAnalysisEditor_timeStepStrategy_VIEW_DEPENDENT ? (my d_endWindow - my d_startWindow) / my p_numberOfTimeStepsPerView :
			0.0;   // the default: determined by pitch floor
		my d_pitch = Sound_to_Pitch_any (sound.peek(), pitchTimeStep,
			my p_pitch_floor,
			my p_pitch_method == kTimeSoundAnalysisEditor_pitch_analysisMethod_AUTOCORRELATION ? 3.0 : 1.0,
			my p_pitch_maximumNumberOfCandidates,
			(my p_pitch_method - 1) * 2 + my p_pitch_veryAccurate,
			my p_pitch_silenceThreshold, my p_pitch_voicingThreshold,
			my p_pitch_octaveCost, my p_pitch_octaveJumpCost, my p_pitch_voicedUnvoicedCost, my p_pitch_ceiling);
		my d_pitch -> xmin = my d_startWindow;
		my d_pitch -> xmax = my d_endWindow;
	} catch (MelderError) {
		Melder_clearError ();
	}
}

void TimeSoundAnalysisEditor_computePitch (TimeSoundAnalysisEditor me) {
	autoMelderProgressOff progress;
	if (my p_pitch_show && my d_endWindow - my d_startWindow <= my p_longestAnalysis &&
		(my d_pitch == NULL || my d_pitch -> xmin != my d_startWindow || my d_pitch -> xmax != my d_endWindow))
	{
		computePitch_inside (me);
	}
}

void TimeSoundAnalysisEditor_computeIntensity (TimeSoundAnalysisEditor me) {
	autoMelderProgressOff progress;
	if (my p_intensity_show && my d_endWindow - my d_startWindow <= my p_longestAnalysis &&
		(my d_intensity == NULL || my d_intensity -> xmin != my d_startWindow || my d_intensity -> xmax != my d_endWindow))
	{
		double margin = 3.2 / my p_pitch_floor;
		forget (my d_intensity);
		try {
			autoSound sound = extractSound (me, my d_startWindow - margin, my d_endWindow + margin);
			my d_intensity = Sound_to_Intensity (sound.peek(), my p_pitch_floor,
				my d_endWindow - my d_startWindow > my p_longestAnalysis ? (my d_endWindow - my d_startWindow) / 100 : 0.0,
				my p_intensity_subtractMeanPressure);
			my d_intensity -> xmin = my d_startWindow;
			my d_intensity -> xmax = my d_endWindow;
		} catch (MelderError) {
			Melder_clearError ();
		}
	}
}

void TimeSoundAnalysisEditor_computeFormants (TimeSoundAnalysisEditor me) {
	autoMelderProgressOff progress;
	if (my p_formant_show && my d_endWindow - my d_startWindow <= my p_longestAnalysis &&
		(my d_formant == NULL || my d_formant -> xmin != my d_startWindow || my d_formant -> xmax != my d_endWindow))
	{
		double margin = my p_formant_windowLength;
		forget (my d_formant);
		try {
			autoSound sound =
				my d_endWindow - my d_startWindow > my p_longestAnalysis ?
					extractSound (me,
						0.5 * (my d_startWindow + my d_endWindow - my p_longestAnalysis) - margin,
						0.5 * (my d_startWindow + my d_endWindow + my p_longestAnalysis) + margin) :
					extractSound (me, my d_startWindow - margin, my d_endWindow + margin);
			double formantTimeStep =
				my p_timeStepStrategy == kTimeSoundAnalysisEditor_timeStepStrategy_FIXED ? my p_fixedTimeStep :
				my p_timeStepStrategy == kTimeSoundAnalysisEditor_timeStepStrategy_VIEW_DEPENDENT ? (my d_endWindow - my d_startWindow) / my p_numberOfTimeStepsPerView :
				0.0;   // the default: determined by analysis window length
			my d_formant = Sound_to_Formant_any (sound.peek(), formantTimeStep,
				lround (my p_formant_numberOfFormants * 2), my p_formant_maximumFormant,
				my p_formant_windowLength, my p_formant_method, my p_formant_preemphasisFrom, 50.0);
			my d_formant -> xmin = my d_startWindow;
			my d_formant -> xmax = my d_endWindow;
		} catch (MelderError) {
			Melder_clearError ();
		}
	}
}

void TimeSoundAnalysisEditor_computePulses (TimeSoundAnalysisEditor me) {
	autoMelderProgressOff progress;
	if (my p_pulses_show && my d_endWindow - my d_startWindow <= my p_longestAnalysis &&
		(my d_pulses == NULL || my d_pulses -> xmin != my d_startWindow || my d_pulses -> xmax != my d_endWindow))
	{
		forget (my d_pulses);
		if (my d_pitch == NULL || my d_pitch -> xmin != my d_startWindow || my d_pitch -> xmax != my d_endWindow) {
			computePitch_inside (me);
		}
		if (my d_pitch != NULL) {
			try {
				autoSound sound = extractSound (me, my d_startWindow, my d_endWindow);
				my d_pulses = Sound_Pitch_to_PointProcess_cc (sound.peek(), my d_pitch);
			} catch (MelderError) {
				Melder_clearError ();
			}
		}
	}
}

static void TimeSoundAnalysisEditor_v_draw_analysis (TimeSoundAnalysisEditor me) {
	/*
	 * d_pitch may not exist yet (if shown at all, it may be going to be created in TimeSoundAnalysisEditor_computePitch (),
	 * and even if if that fails we should see the pitch settings. So we use a dummy object.
	 */
	double pitchFloor_hidden = Function_convertStandardToSpecialUnit (Thing_dummyObject (Pitch), my p_pitch_floor, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
	double pitchCeiling_hidden = Function_convertStandardToSpecialUnit (Thing_dummyObject (Pitch), my p_pitch_ceiling, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
	double pitchFloor_overt = Function_convertToNonlogarithmic (Thing_dummyObject (Pitch), pitchFloor_hidden, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
	double pitchCeiling_overt = Function_convertToNonlogarithmic (Thing_dummyObject (Pitch), pitchCeiling_hidden, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
	double pitchViewFrom_overt = my p_pitch_viewFrom < my p_pitch_viewTo ? my p_pitch_viewFrom : pitchFloor_overt;
	double pitchViewTo_overt = my p_pitch_viewFrom < my p_pitch_viewTo ? my p_pitch_viewTo : pitchCeiling_overt;
	double pitchViewFrom_hidden = Function_isUnitLogarithmic (Thing_dummyObject (Pitch), Pitch_LEVEL_FREQUENCY, my p_pitch_unit) ? log10 (pitchViewFrom_overt) : pitchViewFrom_overt;
	double pitchViewTo_hidden = Function_isUnitLogarithmic (Thing_dummyObject (Pitch), Pitch_LEVEL_FREQUENCY, my p_pitch_unit) ? log10 (pitchViewTo_overt) : pitchViewTo_overt;

	Graphics_setWindow (my d_graphics, 0.0, 1.0, 0.0, 1.0);
	Graphics_setColour (my d_graphics, Graphics_WHITE);
	Graphics_fillRectangle (my d_graphics, 0.0, 1.0, 0.0, 1.0);
	Graphics_setColour (my d_graphics, Graphics_BLACK);
	Graphics_rectangle (my d_graphics, 0.0, 1.0, 0.0, 1.0);

	if (my d_endWindow - my d_startWindow > my p_longestAnalysis) {
		Graphics_setFont (my d_graphics, kGraphics_font_HELVETICA);
		Graphics_setFontSize (my d_graphics, 10);
		Graphics_setTextAlignment (my d_graphics, Graphics_CENTRE, Graphics_HALF);
		Graphics_text (my d_graphics, 0.5, 0.67,   U"(To see the analyses, zoom in to at most ", Melder_half (my p_longestAnalysis), U" seconds,");
		Graphics_text (my d_graphics, 0.5, 0.33, U"or raise the \"longest analysis\" setting with \"Show analyses\" in the View menu.)");
		Graphics_setFontSize (my d_graphics, 12);
		return;
	}
	TimeSoundAnalysisEditor_computeSpectrogram (me);
	if (my p_spectrogram_show && my d_spectrogram != NULL) {
		Spectrogram_paintInside (my d_spectrogram, my d_graphics, my d_startWindow, my d_endWindow,
			my p_spectrogram_viewFrom, my p_spectrogram_viewTo, my p_spectrogram_maximum, my p_spectrogram_autoscaling,
			my p_spectrogram_dynamicRange, my p_spectrogram_preemphasis, my p_spectrogram_dynamicCompression);
	}
	TimeSoundAnalysisEditor_computePitch (me);
	if (my p_pitch_show && my d_pitch != NULL) {
		double periodsPerAnalysisWindow = my p_pitch_method == kTimeSoundAnalysisEditor_pitch_analysisMethod_AUTOCORRELATION ? 3.0 : 1.0;
		double greatestNonUndersamplingTimeStep = 0.5 * periodsPerAnalysisWindow / my p_pitch_floor;
		double defaultTimeStep = 0.5 * greatestNonUndersamplingTimeStep;
		double timeStep =
			my p_timeStepStrategy == kTimeSoundAnalysisEditor_timeStepStrategy_FIXED ? my p_fixedTimeStep :
			my p_timeStepStrategy == kTimeSoundAnalysisEditor_timeStepStrategy_VIEW_DEPENDENT ? (my d_endWindow - my d_startWindow) / my p_numberOfTimeStepsPerView :
			defaultTimeStep;
		int undersampled = timeStep > greatestNonUndersamplingTimeStep;
		long numberOfVisiblePitchPoints = (long) ((my d_endWindow - my d_startWindow) / timeStep);
		Graphics_setColour (my d_graphics, Graphics_CYAN);
		Graphics_setLineWidth (my d_graphics, 3.0);
		if ((my p_pitch_drawingMethod == kTimeSoundAnalysisEditor_pitch_drawingMethod_AUTOMATIC && (undersampled || numberOfVisiblePitchPoints < 101)) ||
		    my p_pitch_drawingMethod == kTimeSoundAnalysisEditor_pitch_drawingMethod_SPECKLE)
		{
			Pitch_drawInside (my d_pitch, my d_graphics, my d_startWindow, my d_endWindow, pitchViewFrom_overt, pitchViewTo_overt, 2, my p_pitch_unit);
		}
		if ((my p_pitch_drawingMethod == kTimeSoundAnalysisEditor_pitch_drawingMethod_AUTOMATIC && ! undersampled) ||
		    my p_pitch_drawingMethod == kTimeSoundAnalysisEditor_pitch_drawingMethod_CURVE)
		{
			Pitch_drawInside (my d_pitch, my d_graphics, my d_startWindow, my d_endWindow, pitchViewFrom_overt, pitchViewTo_overt, FALSE, my p_pitch_unit);
		}
		Graphics_setColour (my d_graphics, Graphics_BLUE);
		Graphics_setLineWidth (my d_graphics, 1.0);
		if ((my p_pitch_drawingMethod == kTimeSoundAnalysisEditor_pitch_drawingMethod_AUTOMATIC && (undersampled || numberOfVisiblePitchPoints < 101)) ||
		    my p_pitch_drawingMethod == kTimeSoundAnalysisEditor_pitch_drawingMethod_SPECKLE)
		{
			Pitch_drawInside (my d_pitch, my d_graphics, my d_startWindow, my d_endWindow, pitchViewFrom_overt, pitchViewTo_overt, 1, my p_pitch_unit);
		}
		if ((my p_pitch_drawingMethod == kTimeSoundAnalysisEditor_pitch_drawingMethod_AUTOMATIC && ! undersampled) ||
		    my p_pitch_drawingMethod == kTimeSoundAnalysisEditor_pitch_drawingMethod_CURVE)
		{
			Pitch_drawInside (my d_pitch, my d_graphics, my d_startWindow, my d_endWindow, pitchViewFrom_overt, pitchViewTo_overt, FALSE, my p_pitch_unit);
		}
		Graphics_setColour (my d_graphics, Graphics_BLACK);
	}
	TimeSoundAnalysisEditor_computeIntensity (me);
	if (my p_intensity_show && my d_intensity != NULL) {
		Graphics_setColour (my d_graphics, my p_spectrogram_show ? Graphics_YELLOW : Graphics_LIME);
		Graphics_setLineWidth (my d_graphics, my p_spectrogram_show ? 1.0 : 3.0);
		Intensity_drawInside (my d_intensity, my d_graphics, my d_startWindow, my d_endWindow,
			my p_intensity_viewFrom, my p_intensity_viewTo);
		Graphics_setLineWidth (my d_graphics, 1.0);
		Graphics_setColour (my d_graphics, Graphics_BLACK);
	}
	TimeSoundAnalysisEditor_computeFormants (me);
	if (my p_formant_show && my d_formant != NULL) {
		Graphics_setColour (my d_graphics, Graphics_RED);
		Graphics_setSpeckleSize (my d_graphics, my p_formant_dotSize);
		Formant_drawSpeckles_inside (my d_formant, my d_graphics, my d_startWindow, my d_endWindow,
			my p_spectrogram_viewFrom, my p_spectrogram_viewTo, my p_formant_dynamicRange);
		Graphics_setColour (my d_graphics, Graphics_BLACK);
	}
	/*
	 * Draw vertical scales.
	 */
	if (my p_pitch_show) {
		double pitchCursor_overt = NUMundefined, pitchCursor_hidden = NUMundefined;
		Graphics_setWindow (my d_graphics, my d_startWindow, my d_endWindow, pitchViewFrom_hidden, pitchViewTo_hidden);
		Graphics_setColour (my d_graphics, Graphics_BLUE);
		if (my d_pitch) {
			if (my d_startSelection == my d_endSelection)
				pitchCursor_hidden = Pitch_getValueAtTime (my d_pitch, my d_startSelection, my p_pitch_unit, 1);
			else
				pitchCursor_hidden = Pitch_getMean (my d_pitch, my d_startSelection, my d_endSelection, my p_pitch_unit);
			pitchCursor_overt = Function_convertToNonlogarithmic (my d_pitch, pitchCursor_hidden, Pitch_LEVEL_FREQUENCY, my p_pitch_unit);
			if (NUMdefined (pitchCursor_hidden)) {
				Graphics_setTextAlignment (my d_graphics, Graphics_LEFT, Graphics_HALF);
				Graphics_text (my d_graphics, my d_endWindow, pitchCursor_hidden,
					Melder_float (Melder_half (pitchCursor_overt)), U" ",
					Function_getUnitText (my d_pitch, Pitch_LEVEL_FREQUENCY, my p_pitch_unit, Function_UNIT_TEXT_SHORT | Function_UNIT_TEXT_GRAPHICAL));
			}
			if (! NUMdefined (pitchCursor_hidden) || Graphics_dyWCtoMM (my d_graphics, pitchCursor_hidden - pitchViewFrom_hidden) > 5.0) {
				Graphics_setTextAlignment (my d_graphics, Graphics_LEFT, Graphics_BOTTOM);
				Graphics_text (my d_graphics, my d_endWindow, pitchViewFrom_hidden - Graphics_dyMMtoWC (my d_graphics, 0.5),
					Melder_float (Melder_half (pitchViewFrom_overt)), U" ",
					Function_getUnitText (my d_pitch, Pitch_LEVEL_FREQUENCY, my p_pitch_unit, Function_UNIT_TEXT_SHORT | Function_UNIT_TEXT_GRAPHICAL));
			}
			if (! NUMdefined (pitchCursor_hidden) || Graphics_dyWCtoMM (my d_graphics, pitchViewTo_hidden - pitchCursor_hidden) > 5.0) {
				Graphics_setTextAlignment (my d_graphics, Graphics_LEFT, Graphics_TOP);
				Graphics_text (my d_graphics, my d_endWindow, pitchViewTo_hidden,
					Melder_float (Melder_half (pitchViewTo_overt)), U" ",
					Function_getUnitText (my d_pitch, Pitch_LEVEL_FREQUENCY, my p_pitch_unit, Function_UNIT_TEXT_SHORT | Function_UNIT_TEXT_GRAPHICAL));
			}
		} else {
			Graphics_setTextAlignment (my d_graphics, Graphics_CENTRE, Graphics_HALF);
			Graphics_setFontSize (my d_graphics, 10);
			Graphics_text (my d_graphics, 0.5 * (my d_startWindow + my d_endWindow), 0.5 * (pitchViewFrom_hidden + pitchViewTo_hidden),
				U"(Cannot show pitch contour. Zoom out or change bottom of pitch range in pitch settings.)");
			Graphics_setFontSize (my d_graphics, 12);
		}
		Graphics_setColour (my d_graphics, Graphics_BLACK);
	}
	if (my p_intensity_show) {
		double intensityCursor = NUMundefined;
		int intensityCursorVisible;
		Graphics_Colour textColour;
		int alignment;
		double y;
		if (! my p_pitch_show) textColour = Graphics_GREEN, alignment = Graphics_LEFT, y = my d_endWindow;
		else if (! my p_spectrogram_show && ! my p_formant_show) textColour = Graphics_GREEN, alignment = Graphics_RIGHT, y = my d_startWindow;
		else textColour = my p_spectrogram_show ? Graphics_LIME : Graphics_GREEN, alignment = Graphics_RIGHT, y = my d_endWindow;
		if (my p_intensity_viewTo > my p_intensity_viewFrom) {
			Graphics_setWindow (my d_graphics, my d_startWindow, my d_endWindow, my p_intensity_viewFrom, my p_intensity_viewTo);
			if (my d_intensity) {
				if (my d_startSelection == my d_endSelection) {
					intensityCursor = Vector_getValueAtX (my d_intensity, my d_startSelection, Vector_CHANNEL_1, Vector_VALUE_INTERPOLATION_LINEAR);
				} else {
					intensityCursor = Intensity_getAverage (my d_intensity, my d_startSelection, my d_endSelection, my p_intensity_averagingMethod);
				}
			}
			Graphics_setColour (my d_graphics, textColour);
			intensityCursorVisible = NUMdefined (intensityCursor) && intensityCursor > my p_intensity_viewFrom && intensityCursor < my p_intensity_viewTo;
			if (intensityCursorVisible) {
				static const char32 *methodString [] = { U" (.5)", U" (μE)", U" (μS)", U" (μ)" };
				Graphics_setTextAlignment (my d_graphics, alignment, Graphics_HALF);
				Graphics_text (my d_graphics, y, intensityCursor,
					Melder_float (Melder_half (intensityCursor)), U" dB",
					my d_startSelection == my d_endSelection ? U"" : methodString [my p_intensity_averagingMethod]);
			}
			if (! intensityCursorVisible || Graphics_dyWCtoMM (my d_graphics, intensityCursor - my p_intensity_viewFrom) > 5.0) {
				Graphics_setTextAlignment (my d_graphics, alignment, Graphics_BOTTOM);
				Graphics_text (my d_graphics, y, my p_intensity_viewFrom - Graphics_dyMMtoWC (my d_graphics, 0.5),
					Melder_float (Melder_half (my p_intensity_viewFrom)), U" dB");
			}
			if (! intensityCursorVisible || Graphics_dyWCtoMM (my d_graphics, my p_intensity_viewTo - intensityCursor) > 5.0) {
				Graphics_setTextAlignment (my d_graphics, alignment, Graphics_TOP);
				Graphics_text (my d_graphics, y, my p_intensity_viewTo,
					Melder_float (Melder_half (my p_intensity_viewTo)), U" dB");
			}
			Graphics_setColour (my d_graphics, Graphics_BLACK);
		}
	}
	if (my p_spectrogram_show || my p_formant_show) {
		int frequencyCursorVisible = my d_spectrogram_cursor > my p_spectrogram_viewFrom && my d_spectrogram_cursor < my p_spectrogram_viewTo;
		Graphics_setWindow (my d_graphics, my d_startWindow, my d_endWindow, my p_spectrogram_viewFrom, my p_spectrogram_viewTo);
		/*
		 * Range marks.
		 */
		Graphics_setLineType (my d_graphics, Graphics_DRAWN);
		Graphics_setColour (my d_graphics, Graphics_BLACK);
		if (! frequencyCursorVisible || Graphics_dyWCtoMM (my d_graphics, my d_spectrogram_cursor - my p_spectrogram_viewFrom) > 5.0) {
			Graphics_setTextAlignment (my d_graphics, Graphics_RIGHT, Graphics_BOTTOM);
			Graphics_text (my d_graphics, my d_startWindow, my p_spectrogram_viewFrom - Graphics_dyMMtoWC (my d_graphics, 0.5),
				Melder_float (Melder_half (my p_spectrogram_viewFrom)), U" Hz");
		}
		if (! frequencyCursorVisible || Graphics_dyWCtoMM (my d_graphics, my p_spectrogram_viewTo - my d_spectrogram_cursor) > 5.0) {
			Graphics_setTextAlignment (my d_graphics, Graphics_RIGHT, Graphics_TOP);
			Graphics_text (my d_graphics, my d_startWindow, my p_spectrogram_viewTo,
				Melder_float (Melder_half (my p_spectrogram_viewTo)), U" Hz");
		}
		/*
		 * Cursor lines.
		 */
		Graphics_setLineType (my d_graphics, Graphics_DOTTED);
		Graphics_setColour (my d_graphics, Graphics_RED);
		if (frequencyCursorVisible) {
			double x = my d_startWindow, y = my d_spectrogram_cursor;
			Graphics_setTextAlignment (my d_graphics, Graphics_RIGHT, Graphics_HALF);
			Graphics_text (my d_graphics, x, y,   Melder_float (Melder_half (y)), U" Hz");
			Graphics_line (my d_graphics, x, y, my d_endWindow, y);
		}
		/*
		if (our startSelection >= our startWindow && our startSelection <= our endWindow)
			Graphics_line (our graphics, our startSelection, our p_spectrogram_viewFrom, our startSelection, our p_spectrogram_viewTo);
		if (our endSelection > our startWindow && our endSelection < our endWindow && our endSelection != our startSelection)
			Graphics_line (our graphics, our endSelection, our p_spectrogram_viewFrom, our endSelection, our p_spectrogram_viewTo);*/
		/*
		 * Cadre.
		 */
		Graphics_setLineType (my d_graphics, Graphics_DRAWN);
		Graphics_setColour (my d_graphics, Graphics_BLACK);
		Graphics_rectangle (my d_graphics, my d_startWindow, my d_endWindow, my p_spectrogram_viewFrom, my p_spectrogram_viewTo);
	}
}
void structTimeSoundAnalysisEditor :: v_draw_analysis () {
	TimeSoundAnalysisEditor_v_draw_analysis (this);
}

void structTimeSoundAnalysisEditor :: v_draw_analysis_pulses () {
	TimeSoundAnalysisEditor_computePulses (this);
	if (our p_pulses_show && our d_endWindow - our d_startWindow <= our p_longestAnalysis && our d_pulses != NULL) {
		PointProcess point = d_pulses;
		Graphics_setWindow (our d_graphics, our d_startWindow, our d_endWindow, -1.0, 1.0);
		Graphics_setColour (our d_graphics, Graphics_BLUE);
		if (point -> nt < 2000) for (long i = 1; i <= point -> nt; i ++) {
			double t = point -> t [i];
			if (t >= our d_startWindow && t <= our d_endWindow)
				Graphics_line (our d_graphics, t, -0.9, t, 0.9);
		}
		Graphics_setColour (our d_graphics, Graphics_BLACK);
	}
}

int structTimeSoundAnalysisEditor :: v_click (double xbegin, double ybegin, bool shiftKeyPressed) {
	if (our p_pitch_show) {
		//Melder_warning (xbegin, U" ", ybegin);
		if (xbegin >= our d_endWindow && ybegin > 0.48 && ybegin <= 0.50) {
			our pref_pitch_ceiling () = our p_pitch_ceiling = our p_pitch_ceiling * 1.26;
			forget (our d_pitch);
			forget (our d_intensity);
			forget (our d_pulses);
			return 1;
		}
		if (xbegin >= our d_endWindow && ybegin > 0.46 && ybegin <= 0.48) {
			our pref_pitch_ceiling () = our p_pitch_ceiling = our p_pitch_ceiling / 1.26;
			forget (our d_pitch);
			forget (our d_intensity);
			forget (our d_pulses);
			return 1;
		}
	}
	return TimeSoundAnalysisEditor_Parent :: v_click (xbegin, ybegin, shiftKeyPressed);
}

void TimeSoundAnalysisEditor_init (TimeSoundAnalysisEditor me, const char32 *title, Function data, Sampled sound, bool ownSound) {
	TimeSoundEditor_init (me, title, data, sound, ownSound);
	if (my v_hasAnalysis ()) {
		if (my p_log1_toLogFile == false && my p_log1_toInfoWindow == false) {
			my pref_log1_toLogFile    () = my p_log1_toLogFile    = true;
			my pref_log1_toInfoWindow () = my p_log1_toInfoWindow = true;
		}
		if (my p_log2_toLogFile == false && my p_log2_toInfoWindow == false) {
			my pref_log2_toLogFile    () = my p_log2_toLogFile    = true;
			my pref_log2_toInfoWindow () = my p_log2_toInfoWindow = true;
		}
		if (! my v_hasSpectrogram ())
			my p_spectrogram_show = false;
		if (! my v_hasPitch ())
			my p_pitch_show = false;
		if (! my v_hasIntensity ())
			my p_intensity_show = false;
		if (! my v_hasFormants ())
			my p_formant_show = false;
		if (! my v_hasPulses ())
			my p_pulses_show = false;
	}
}

/* End of file TimeSoundAnalysisEditor.cpp */
