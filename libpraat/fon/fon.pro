! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = fon

INCLUDEPATH += ../num ../kar ../sys ../dwsys ../stat ../dwtools ../LPC ../fon ../external/portaudio ../external/flac ../external/mp3

SOURCES = Transition.cpp Distributions_and_Transition.cpp \
   Function.cpp Sampled.cpp SampledXY.cpp Matrix.cpp Vector.cpp Polygon.cpp PointProcess.cpp \
   Matrix_and_PointProcess.cpp Matrix_and_Polygon.cpp AnyTier.cpp RealTier.cpp \
   Sound.cpp LongSound.cpp Sound_files.cpp Sound_audio.cpp PointProcess_and_Sound.cpp Sound_PointProcess.cpp ParamCurve.cpp \
   Pitch.cpp Harmonicity.cpp Intensity.cpp Matrix_and_Pitch.cpp Sound_to_Pitch.cpp \
   Sound_to_Intensity.cpp Sound_to_Harmonicity.cpp Sound_to_Harmonicity_GNE.cpp Sound_to_PointProcess.cpp \
   Pitch_to_PointProcess.cpp Pitch_to_Sound.cpp Pitch_Intensity.cpp \
   PitchTier.cpp Pitch_to_PitchTier.cpp PitchTier_to_PointProcess.cpp PitchTier_to_Sound.cpp Manipulation.cpp \
   Pitch_AnyTier_to_PitchTier.cpp IntensityTier.cpp DurationTier.cpp AmplitudeTier.cpp \
   Spectrum.cpp Ltas.cpp Spectrogram.cpp SpectrumTier.cpp Ltas_to_SpectrumTier.cpp \
   Formant.cpp Image.cpp Sound_to_Formant.cpp Sound_and_Spectrogram.cpp \
   Sound_and_Spectrum.cpp Spectrum_and_Spectrogram.cpp Spectrum_to_Formant.cpp \
   FormantTier.cpp TextGrid.cpp TextGrid_Sound.cpp Label.cpp FormantGrid.cpp \
   Excitation.cpp Cochleagram.cpp Cochleagram_and_Excitation.cpp Excitation_to_Formant.cpp \
   Sound_to_Cochleagram.cpp Spectrum_to_Excitation.cpp \
   VocalTract.cpp VocalTract_to_Spectrum.cpp \
   SoundRecorder.cpp Sound_enhance.cpp VoiceAnalysis.cpp \
   FunctionEditor.cpp TimeSoundEditor.cpp TimeSoundAnalysisEditor.cpp \
   PitchEditor.cpp SoundEditor.cpp SpectrumEditor.cpp SpectrogramEditor.cpp PointEditor.cpp \
   RealTierEditor.cpp PitchTierEditor.cpp IntensityTierEditor.cpp \
   DurationTierEditor.cpp AmplitudeTierEditor.cpp \
   ManipulationEditor.cpp TextGridEditor.cpp FormantGridEditor.cpp \
   WordList.cpp SpellingChecker.cpp \
   FujisakiPitch.cpp \
   ExperimentMFC.cpp RunnerMFC.cpp manual_Exp.cpp praat_Exp.cpp \
   Photo.cpp Movie.cpp MovieWindow.cpp \
   Corpus.cpp \
   manual_Picture.cpp manual_Manual.cpp manual_Script.cpp \
   manual_soundFiles.cpp manual_tutorials.cpp manual_references.cpp \
   manual_programming.cpp manual_Fon.cpp manual_voice.cpp Praat_tests.cpp \
   manual_glossary.cpp manual_Sampling.cpp manual_exampleSound.cpp \
   manual_sound.cpp manual_pitch.cpp manual_spectrum.cpp manual_formant.cpp manual_annotation.cpp \
   praat_Sound_init.cpp praat_TextGrid_init.cpp praat_Fon.cpp

HEADERS += \
    AmplitudeTier.h \
    AmplitudeTierEditor.h \
    AnyTier.h \
    AnyTier_def.h \
    Cochleagram.h \
    Cochleagram_and_Excitation.h \
    Corpus.h \
    Corpus_def.h \
    Distributions_and_Transition.h \
    DurationTier.h \
    DurationTierEditor.h \
    Excitation.h \
    Excitation_to_Formant.h \
    Experiment_enums.h \
    ExperimentMFC.h \
    ExperimentMFC_def.h \
    Formant.h \
    Formant_def.h \
    FormantGrid.h \
    FormantGrid_def.h \
    FormantGridEditor.h \
    FormantGridEditor_prefs.h \
    FormantTier.h \
    FormantTier_def.h \
    FujisakiPitch.h \
    FujisakiPitch_def.h \
    Function.h \
    Function_def.h \
    FunctionEditor.h \
    FunctionEditor_prefs.h \
    Harmonicity.h \
    Harmonics.h \
    Harmonics_def.h \
    Image.h \
    Image_def.h \
    Intensity.h \
    IntensityTier.h \
    IntensityTierEditor.h \
    Label.h \
    LongSound.h \
    Ltas.h \
    Ltas_to_SpectrumTier.h \
    Manipulation.h \
    Manipulation_def.h \
    ManipulationEditor.h \
    ManipulationEditor_enums.h \
    ManipulationEditor_prefs.h \
    manual_exampleSound.h \
    Matrix.h \
    Matrix_and_Pitch.h \
    Matrix_and_PointProcess.h \
    Matrix_and_Polygon.h \
    Matrix_def.h \
    Movie.h \
    Movie_def.h \
    MovieWindow.h \
    ParamCurve.h \
    ParamCurve_def.h \
    Photo.h \
    Photo_def.h \
    Pitch.h \
    Pitch_AnyTier_to_PitchTier.h \
    Pitch_def.h \
    Pitch_enums.h \
    Pitch_Intensity.h \
    Pitch_to_PitchTier.h \
    Pitch_to_PointProcess.h \
    Pitch_to_Sound.h \
    PitchEditor.h \
    PitchTier.h \
    PitchTier_to_PointProcess.h \
    PitchTier_to_Sound.h \
    PitchTierEditor.h \
    PointEditor.h \
    PointProcess.h \
    PointProcess_and_Sound.h \
    PointProcess_def.h \
    Polygon.h \
    Polygon_def.h \
    Praat_tests.h \
    Praat_tests_enums.h \
    RealTier.h \
    RealTier_def.h \
    RealTierEditor.h \
    RunnerMFC.h \
    Sampled.h \
    Sampled_def.h \
    SampledXY.h \
    SampledXY_def.h \
    Sound.h \
    Sound_and_Spectrogram.h \
    Sound_and_Spectrogram_enums.h \
    Sound_and_Spectrum.h \
    Sound_enums.h \
    Sound_PointProcess.h \
    Sound_to_Cochleagram.h \
    Sound_to_Formant.h \
    Sound_to_Harmonicity.h \
    Sound_to_Intensity.h \
    Sound_to_Pitch.h \
    Sound_to_PointProcess.h \
    SoundEditor.h \
    SoundRecorder.h \
    SoundRecorder_enums.h \
    SoundRecorder_prefs.h \
    Spectrogram.h \
    SpectrogramEditor.h \
    Spectrum.h \
    Spectrum_and_Spectrogram.h \
    Spectrum_def.h \
    Spectrum_to_Excitation.h \
    Spectrum_to_Formant.h \
    SpectrumEditor.h \
    SpectrumEditor_prefs.h \
    SpectrumTier.h \
    SpellingChecker.h \
    SpellingChecker_def.h \
    TextGrid.h \
    TextGrid_def.h \
    TextGrid_Sound.h \
    TextGridEditor.h \
    TextGridEditor_enums.h \
    TextGridEditor_prefs.h \
    TimeSoundAnalysisEditor.h \
    TimeSoundAnalysisEditor_enums.h \
    TimeSoundAnalysisEditor_prefs.h \
    TimeSoundEditor.h \
    TimeSoundEditor_enums.h \
    TimeSoundEditor_prefs.h \
    Transition.h \
    Transition_def.h \
    Vector.h \
    VocalTract.h \
    VocalTract_to_Spectrum.h \
    VoiceAnalysis.h \
    WordList.h \
    WordList_def.h
