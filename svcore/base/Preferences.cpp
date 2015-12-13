/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "Preferences.h"

#include "Exceptions.h"

#include "TempDirectory.h"

#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QSettings>

Preferences *
Preferences::m_instance = 0;

Preferences *
Preferences::getInstance()
{
    if (!m_instance) m_instance = new Preferences();
    return m_instance;
}

Preferences::Preferences() :
    m_spectrogramSmoothing(SpectrogramInterpolated),
    m_spectrogramXSmoothing(SpectrogramXInterpolated),
    m_tuningFrequency(440),
    m_propertyBoxLayout(VerticallyStacked),
    m_windowType(HanningWindow),
    m_resampleQuality(1),
    m_omitRecentTemps(true),
    m_tempDirRoot(""),
    m_fixedSampleRate(0),
    m_resampleOnLoad(false),
    m_normaliseAudio(false),
    m_viewFontSize(10),
    m_backgroundMode(BackgroundFromTheme),
    m_timeToTextMode(TimeToTextMs),
    m_showHMS(true),
    m_octave(4),
    m_showSplash(true)
{
    QSettings settings;
    settings.beginGroup("Preferences");
    m_spectrogramSmoothing = SpectrogramSmoothing
        (settings.value("spectrogram-y-smoothing", int(m_spectrogramSmoothing)).toInt());
    m_spectrogramXSmoothing = SpectrogramXSmoothing
        (settings.value("spectrogram-x-smoothing", int(m_spectrogramXSmoothing)).toInt());
    m_tuningFrequency = settings.value("tuning-frequency", 440.).toDouble();
    m_propertyBoxLayout = PropertyBoxLayout
        (settings.value("property-box-layout", int(VerticallyStacked)).toInt());
    m_windowType = WindowType
        (settings.value("window-type", int(HanningWindow)).toInt());
    m_resampleQuality = settings.value("resample-quality", 1).toInt();
    m_fixedSampleRate = settings.value("fixed-sample-rate", 0).toDouble();
    m_resampleOnLoad = settings.value("resample-on-load", false).toBool();
    m_normaliseAudio = settings.value("normalise-audio", false).toBool();
    m_backgroundMode = BackgroundMode
        (settings.value("background-mode", int(BackgroundFromTheme)).toInt());
    m_timeToTextMode = TimeToTextMode
        (settings.value("time-to-text-mode", int(TimeToTextMs)).toInt());
    m_showHMS = (settings.value("show-hours-minutes-seconds", true)).toBool(); 
    m_octave = (settings.value("octave-of-middle-c", 4)).toInt();
    m_viewFontSize = settings.value("view-font-size", 10).toInt();
    m_showSplash = settings.value("show-splash", true).toBool();
    settings.endGroup();

    settings.beginGroup("TempDirectory");
    m_tempDirRoot = settings.value("create-in", "$HOME").toString();
    settings.endGroup();
}

Preferences::~Preferences()
{
}

Preferences::PropertyList
Preferences::getProperties() const
{
    PropertyList props;
    props.push_back("Spectrogram Y Smoothing");
    props.push_back("Spectrogram X Smoothing");
    props.push_back("Tuning Frequency");
    props.push_back("Property Box Layout");
    props.push_back("Window Type");
    props.push_back("Resample Quality");
    props.push_back("Omit Temporaries from Recent Files");
    props.push_back("Resample On Load");
    props.push_back("Normalise Audio");
    props.push_back("Fixed Sample Rate");
    props.push_back("Temporary Directory Root");
    props.push_back("Background Mode");
    props.push_back("Time To Text Mode");
    props.push_back("Show Hours And Minutes");
    props.push_back("Octave Numbering System");
    props.push_back("View Font Size");
    props.push_back("Show Splash Screen");
    return props;
}

QString
Preferences::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Spectrogram Y Smoothing") {
        return tr("Spectrogram y-axis interpolation:");
    }
    if (name == "Spectrogram X Smoothing") {
        return tr("Spectrogram x-axis interpolation:");
    }
    if (name == "Tuning Frequency") {
        return tr("Frequency of concert A");
    }
    if (name == "Property Box Layout") {
        return tr("Property box layout");
    }
    if (name == "Window Type") {
        return tr("Spectral analysis window shape");
    }
    if (name == "Resample Quality") {
        return tr("Playback resampler type");
    }
    if (name == "Normalise Audio") {
        return tr("Normalise audio signal when reading from audio file");
    }
    if (name == "Omit Temporaries from Recent Files") {
        return tr("Omit temporaries from Recent Files menu");
    }
    if (name == "Resample On Load") {
        return tr("Resample mismatching files on import");
    }
    if (name == "Fixed Sample Rate") {
        return tr("Single fixed sample rate to resample all files to");
    }
    if (name == "Temporary Directory Root") {
        return tr("Location for cache file directory");
    }
    if (name == "Background Mode") {
        return tr("Background colour preference");
    }
    if (name == "Time To Text Mode") {
        return tr("Time display precision");
    }
    if (name == "Show Hours And Minutes") {
        return tr("Use hours:minutes:seconds format");
    }
    if (name == "Octave Numbering System") {
        return tr("Label middle C as");
    }
    if (name == "View Font Size") {
        return tr("Font size for text overlays");
    }
    if (name == "Show Splash Screen") {
        return tr("Show splash screen on startup");
    }
    return name;
}

Preferences::PropertyType
Preferences::getPropertyType(const PropertyName &name) const
{
    if (name == "Spectrogram Y Smoothing") {
        return ValueProperty;
    }
    if (name == "Spectrogram X Smoothing") {
        return ValueProperty;
    }
    if (name == "Tuning Frequency") {
        return RangeProperty;
    }
    if (name == "Property Box Layout") {
        return ValueProperty;
    }
    if (name == "Window Type") {
        return ValueProperty;
    }
    if (name == "Resample Quality") {
        return ValueProperty;
    }
    if (name == "Normalise Audio") {
        return ToggleProperty;
    }
    if (name == "Omit Temporaries from Recent Files") {
        return ToggleProperty;
    }
    if (name == "Resample On Load") {
        return ToggleProperty;
    }
    if (name == "Fixed Sample Rate") {
        return ValueProperty;
    }
    if (name == "Temporary Directory Root") {
        // It's an arbitrary string, we don't have a set of values for this
        return InvalidProperty;
    }
    if (name == "Background Mode") {
        return ValueProperty;
    }
    if (name == "Time To Text Mode") {
        return ValueProperty;
    }
    if (name == "Show Hours And Minutes") {
        return ToggleProperty;
    }
    if (name == "Octave Numbering System") {
        return ValueProperty;
    }
    if (name == "View Font Size") {
        return RangeProperty;
    }
    if (name == "Show Splash Screen") {
        return ToggleProperty;
    }
    return InvalidProperty;
}

int
Preferences::getPropertyRangeAndValue(const PropertyName &name,
                                      int *min, int *max, int *deflt) const
{
    if (name == "Spectrogram Y Smoothing") {
        if (min) *min = 0;
        if (max) *max = 3;
        if (deflt) *deflt = int(SpectrogramInterpolated);
        return int(m_spectrogramSmoothing);
    }
    if (name == "Spectrogram X Smoothing") {
        if (min) *min = 0;
        if (max) *max = 1;
        if (deflt) *deflt = int(SpectrogramXInterpolated);
        return int(m_spectrogramXSmoothing);
    }

    //!!! freq mapping

    if (name == "Property Box Layout") {
        if (min) *min = 0;
        if (max) *max = 1;
        if (deflt) *deflt = 0;
        return m_propertyBoxLayout == Layered ? 1 : 0;
    }        

    if (name == "Window Type") {
        if (min) *min = int(RectangularWindow);
        if (max) *max = int(BlackmanHarrisWindow);
        if (deflt) *deflt = int(HanningWindow);
        return int(m_windowType);
    }

    if (name == "Resample Quality") {
        if (min) *min = 0;
        if (max) *max = 2;
        if (deflt) *deflt = 1;
        return m_resampleQuality;
    }

    if (name == "Omit Temporaries from Recent Files") {
        if (deflt) *deflt = 1;
        return m_omitRecentTemps ? 1 : 0;
    }

    if (name == "Background Mode") {
        if (min) *min = 0;
        if (max) *max = 2;
        if (deflt) *deflt = 0;
        return int(m_backgroundMode);
    }        

    if (name == "Time To Text Mode") {
        if (min) *min = 0;
        if (max) *max = 6;
        if (deflt) *deflt = 0;
        return int(m_timeToTextMode);
    }        

    if (name == "Show Hours And Minutes") {
        if (deflt) *deflt = 1;
        return m_showHMS ? 1 : 0;
    }
    
    if (name == "Octave Numbering System") {
        // we don't support arbitrary octaves in the gui, because we
        // want to be able to label what the octave system comes
        // from. so we support 0, 3, 4 and 5.
        if (min) *min = 0;
        if (max) *max = 3;
        if (deflt) *deflt = 2;
        return int(getSystemWithMiddleCInOctave(m_octave));
    }

    if (name == "View Font Size") {
        if (min) *min = 3;
        if (max) *max = 48;
        if (deflt) *deflt = 10;
        return int(m_viewFontSize);
    }

    if (name == "Show Splash Screen") {
        if (deflt) *deflt = 1;
        return m_showSplash ? 1 : 0;
    }

    return 0;
}

QString
Preferences::getPropertyValueLabel(const PropertyName &name,
                                   int value) const
{
    if (name == "Property Box Layout") {
        if (value == 0) return tr("Show boxes for all panes");
        else return tr("Show box for current pane only");
    }
    if (name == "Window Type") {
        switch (WindowType(value)) {
        case RectangularWindow: return tr("Rectangular");
        case BartlettWindow: return tr("Triangular");
        case HammingWindow: return tr("Hamming");
        case HanningWindow: return tr("Hann");
        case BlackmanWindow: return tr("Blackman");
        case GaussianWindow: return tr("Gaussian");
        case ParzenWindow: return tr("Parzen");
        case NuttallWindow: return tr("Nuttall");
        case BlackmanHarrisWindow: return tr("Blackman-Harris");
        }
    }
    if (name == "Resample Quality") {
        switch (value) {
        case 0: return tr("Fastest");
        case 1: return tr("Standard");
        case 2: return tr("Highest quality");
        }
    }
    if (name == "Spectrogram Y Smoothing") {
        switch (value) {
        case NoSpectrogramSmoothing: return tr("None");
        case SpectrogramInterpolated: return tr("Linear interpolation");
        case SpectrogramZeroPadded: return tr("4 x Oversampling");
        case SpectrogramZeroPaddedAndInterpolated: return tr("4 x Oversampling with interpolation");
        }
    }
    if (name == "Spectrogram X Smoothing") {
        switch (value) {
        case NoSpectrogramXSmoothing: return tr("None");
        case SpectrogramXInterpolated: return tr("Linear interpolation");
        }
    }
    if (name == "Background Mode") {
        switch (value) {
        case BackgroundFromTheme: return tr("Follow desktop theme");
        case DarkBackground: return tr("Dark background");
        case LightBackground: return tr("Light background");
        }
    }
    if (name == "Time To Text Mode") {
        switch (value) {
        case TimeToTextMs: return tr("Standard (to millisecond)");
        case TimeToTextUs: return tr("High resolution (to microsecond)");
        case TimeToText24Frame: return tr("24 FPS");
        case TimeToText25Frame: return tr("25 FPS");
        case TimeToText30Frame: return tr("30 FPS");
        case TimeToText50Frame: return tr("50 FPS");
        case TimeToText60Frame: return tr("60 FPS");
        }
    }
    if (name == "Octave Numbering System") {
        switch (value) {
        case C0_Centre: return tr("C0 - middle of octave scale");
        case C3_Logic: return tr("C3 - common MIDI sequencer convention");
        case C4_ASA: return tr("C4 - ASA American standard");
        case C5_Sonar: return tr("C5 - used in Cakewalk and others");
        }
    }
            
    return "";
}

QString
Preferences::getPropertyContainerName() const
{
    return tr("Preferences");
}

QString
Preferences::getPropertyContainerIconName() const
{
    return "preferences";
}

void
Preferences::setProperty(const PropertyName &name, int value) 
{
    if (name == "Spectrogram Y Smoothing") {
        setSpectrogramSmoothing(SpectrogramSmoothing(value));
    } else if (name == "Spectrogram X Smoothing") {
        setSpectrogramXSmoothing(SpectrogramXSmoothing(value));
    } else if (name == "Tuning Frequency") {
        //!!!
    } else if (name == "Property Box Layout") {
        setPropertyBoxLayout(value == 0 ? VerticallyStacked : Layered);
    } else if (name == "Window Type") {
        setWindowType(WindowType(value));
    } else if (name == "Resample Quality") {
        setResampleQuality(value);
    } else if (name == "Omit Temporaries from Recent Files") {
        setOmitTempsFromRecentFiles(value ? true : false);
    } else if (name == "Background Mode") {
        setBackgroundMode(BackgroundMode(value));
    } else if (name == "Time To Text Mode") {
        setTimeToTextMode(TimeToTextMode(value));
    } else if (name == "Show Hours And Minutes") {
        setShowHMS(value ? true : false);
    } else if (name == "Octave Numbering System") {
        setOctaveOfMiddleC(getOctaveOfMiddleCInSystem
                           (OctaveNumberingSystem(value)));
    } else if (name == "View Font Size") {
        setViewFontSize(value);
    } else if (name == "Show Splash Screen") {
        setShowSplash(value ? true : false);
    }
}

void
Preferences::setSpectrogramSmoothing(SpectrogramSmoothing smoothing)
{
    if (m_spectrogramSmoothing != smoothing) {

        // "smoothing" is one of those words that looks increasingly
        // ridiculous the more you see it.  Smoothing smoothing smoothing.
        m_spectrogramSmoothing = smoothing;

        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("spectrogram-y-smoothing", int(smoothing));
        settings.endGroup();
        emit propertyChanged("Spectrogram Y Smoothing");
    }
}

void
Preferences::setSpectrogramXSmoothing(SpectrogramXSmoothing smoothing)
{
    if (m_spectrogramXSmoothing != smoothing) {

        // "smoothing" is one of those words that looks increasingly
        // ridiculous the more you see it.  Smoothing smoothing smoothing.
        m_spectrogramXSmoothing = smoothing;

        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("spectrogram-x-smoothing", int(smoothing));
        settings.endGroup();
        emit propertyChanged("Spectrogram X Smoothing");
    }
}

void
Preferences::setTuningFrequency(double freq)
{
    if (m_tuningFrequency != freq) {
        m_tuningFrequency = freq;
        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("tuning-frequency", freq);
        settings.endGroup();
        emit propertyChanged("Tuning Frequency");
    }
}

void
Preferences::setPropertyBoxLayout(PropertyBoxLayout layout)
{
    if (m_propertyBoxLayout != layout) {
        m_propertyBoxLayout = layout;
        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("property-box-layout", int(layout));
        settings.endGroup();
        emit propertyChanged("Property Box Layout");
    }
}

void
Preferences::setWindowType(WindowType type)
{
    if (m_windowType != type) {
        m_windowType = type;
        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("window-type", int(type));
        settings.endGroup();
        emit propertyChanged("Window Type");
    }
}

void
Preferences::setResampleQuality(int q)
{
    if (m_resampleQuality != q) {
        m_resampleQuality = q;
        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("resample-quality", q);
        settings.endGroup();
        emit propertyChanged("Resample Quality");
    }
}

void
Preferences::setOmitTempsFromRecentFiles(bool omit)
{
    if (m_omitRecentTemps != omit) {
        m_omitRecentTemps = omit;
        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("omit-recent-temporaries", omit);
        settings.endGroup();
        emit propertyChanged("Omit Temporaries from Recent Files");
    }
}

void
Preferences::setTemporaryDirectoryRoot(QString root)
{
    if (root == QDir::home().absolutePath()) {
        root = "$HOME";
    }
    if (m_tempDirRoot != root) {
        m_tempDirRoot = root;
        QSettings settings;
        settings.beginGroup("TempDirectory");
        settings.setValue("create-in", root);
        settings.endGroup();
        emit propertyChanged("Temporary Directory Root");
    }
}

void
Preferences::setResampleOnLoad(bool resample)
{
    if (m_resampleOnLoad != resample) {
        m_resampleOnLoad = resample;
        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("resample-on-load", resample);
        settings.endGroup();
        emit propertyChanged("Resample On Load");
    }
}

void
Preferences::setFixedSampleRate(sv_samplerate_t rate)
{
    if (m_fixedSampleRate != rate) {
        m_fixedSampleRate = rate;
        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("fixed-sample-rate", rate);
        settings.endGroup();
        emit propertyChanged("Fixed Sample Rate");
    }
}

void
Preferences::setNormaliseAudio(bool norm)
{
    if (m_normaliseAudio != norm) {
        m_normaliseAudio = norm;
        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("normalise-audio", norm);
        settings.endGroup();
        emit propertyChanged("Normalise Audio");
    }
}

void
Preferences::setBackgroundMode(BackgroundMode mode)
{
    if (m_backgroundMode != mode) {

        m_backgroundMode = mode;

        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("background-mode", int(mode));
        settings.endGroup();
        emit propertyChanged("Background Mode");
    }
}

void
Preferences::setTimeToTextMode(TimeToTextMode mode)
{
    if (m_timeToTextMode != mode) {

        m_timeToTextMode = mode;

        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("time-to-text-mode", int(mode));
        settings.endGroup();
        emit propertyChanged("Time To Text Mode");
    }
}

void
Preferences::setShowHMS(bool show)
{
    if (m_showHMS != show) {

        m_showHMS = show;

        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("show-hours-minutes-seconds", show);
        settings.endGroup();
        emit propertyChanged("Show Hours And Minutes");
    }
}

void
Preferences::setOctaveOfMiddleC(int oct)
{
    if (m_octave != oct) {

        m_octave = oct;

        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("octave-of-middle-c", int(oct));
        settings.endGroup();
        emit propertyChanged("Octave Numbering System");
    }
}

int
Preferences::getOctaveOfMiddleCInSystem(OctaveNumberingSystem s)
{
    switch (s) {
    case C0_Centre: return 0;
    case C3_Logic: return 3;
    case C4_ASA: return 4;
    case C5_Sonar: return 5;
    default: return 4;
    }
}

Preferences::OctaveNumberingSystem
Preferences::getSystemWithMiddleCInOctave(int o)
{
    switch (o) {
    case 0: return C0_Centre;
    case 3: return C3_Logic;
    case 4: return C4_ASA;
    case 5: return C5_Sonar;
    default: return C4_ASA;
    }
}

void
Preferences::setViewFontSize(int size)
{
    if (m_viewFontSize != size) {

        m_viewFontSize = size;

        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("view-font-size", size);
        settings.endGroup();
        emit propertyChanged("View Font Size");
    }
}

void
Preferences::setShowSplash(bool show) 
{
    if (m_showSplash != show) {

        m_showSplash = show;

        QSettings settings;
        settings.beginGroup("Preferences");
        settings.setValue("show-splash", show);
        settings.endGroup();
        emit propertyChanged("Show Splash Screen");
    }
}
        
