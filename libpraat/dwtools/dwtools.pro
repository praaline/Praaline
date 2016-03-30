! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = dwtools

INCLUDEPATH += ../num ../LPC ../fon ../sys ../stat ../dwsys ../external/portaudio ../external/espeak ../EEG ../kar

SOURCES = Activation.cpp AffineTransform.cpp \
        Categories.cpp CategoriesEditor.cpp \
        Categories_and_Strings.cpp CCA.cpp CCA_and_Correlation.cpp \
        CC.cpp CCs_to_DTW.cpp \
        ClassificationTable.cpp Confusion.cpp \
        ComplexSpectrogram.cpp Configuration.cpp ContingencyTable.cpp \
        Configuration_AffineTransform.cpp \
        Configuration_and_Procrustes.cpp  DataModeler.cpp Distance.cpp \
        DTW.cpp DTW_and_TextGrid.cpp \
        Discriminant.cpp  Discriminant_Pattern_Categories.cpp \
        EditDistanceTable.cpp EEG_extensions.cpp \
        Eigen_and_Matrix.cpp Eigen_and_Procrustes.cpp \
        Eigen_and_TableOfReal.cpp\
        Eigen_and_SSCP.cpp Excitations.cpp \
        FilterBank.cpp FormantGrid_extensions.cpp \
        GaussianMixture.cpp  \
        HMM.cpp \
        ICA.cpp Intensity_extensions.cpp \
        LFCC.cpp LongSound_extensions.cpp \
        KlattGrid.cpp KlattGridEditors.cpp KlattTable.cpp \
        Ltas_extensions.cpp \
        MFCC.cpp \
        manual_DataModeler.cpp manual_dwtools.cpp manual_BSS.cpp manual_HMM.cpp \
        manual_KlattGrid.cpp manual_MDS.cpp manual_Permutation.cpp \
        Minimizers.cpp \
        Matrix_extensions.cpp \
        Matrix_Categories.cpp MDS.cpp \
        Pattern.cpp PCA.cpp \
        Pitch_extensions.cpp Polynomial.cpp \
        Polygon_extensions.cpp Procrustes.cpp \
        Proximity.cpp \
        Resonator.cpp \
        Sampled2.cpp \
        Sound_and_Spectrogram_extensions.cpp Sound_and_PCA.cpp Sound_extensions.cpp \
        Sound_to_MFCC.cpp Sounds_to_DTW.cpp \
        Sound_to_Pitch2.cpp Sound_to_SPINET.cpp SPINET.cpp SPINET_to_Pitch.cpp \
        Spectrogram_extensions.cpp Spectrum_extensions.cpp SSCP.cpp Strings_extensions.cpp \
        SpeechSynthesizer.cpp SpeechSynthesizer_and_TextGrid.cpp\
        Table_extensions.cpp TableOfReal_and_SVD.cpp\
        TableOfReal_extensions.cpp \
        TableOfReal_and_Permutation.cpp \
        TextGrid_extensions.cpp \
        VowelEditor.cpp \
        praat_MDS_init.cpp praat_BSS_init.cpp praat_HMM_init.cpp \
        praat_KlattGrid_init.cpp praat_DataModeler_init.cpp praat_David_init.cpp

HEADERS += \
    Activation.h \
    AffineTransform.h \
    AffineTransform_def.h \
    Categories.h \
    Categories_and_Strings.h \
    Categories_def.h \
    CategoriesEditor.h \
    CC.h \
    CC_def.h \
    CCA.h \
    CCA_and_Correlation.h \
    CCA_def.h \
    CCs_to_DTW.h \
    ClassificationTable.h \
    ComplexSpectrogram.h \
    ComplexSpectrogram_def.h \
    Configuration.h \
    Configuration_AffineTransform.h \
    Configuration_and_Procrustes.h \
    Configuration_def.h \
    Confusion.h \
    ContingencyTable.h \
    DataModeler.h \
    DataModeler_def.h \
    Discriminant.h \
    Discriminant_def.h \
    Discriminant_Pattern_Categories.h \
    Distance.h \
    DTW.h \
    DTW_and_TextGrid.h \
    DTW_def.h \
    EditDistanceTable.h \
    EditDistanceTable_def.h \
    EEG_extensions.h \
    Eigen_and_Matrix.h \
    Eigen_and_Procrustes.h \
    Eigen_and_SSCP.h \
    Eigen_and_TableOfReal.h \
    Excitations.h \
    FilterBank.h \
    FormantGrid_extensions.h \
    GaussianMixture.h \
    GaussianMixture_def.h \
    HMM.h \
    HMM_def.h \
    ICA.h \
    Intensity_extensions.h \
    KlattGrid.h \
    KlattGrid_def.h \
    KlattGridEditors.h \
    KlattTable.h \
    LFCC.h \
    LongSound_extensions.h \
    Ltas_extensions.h \
    Matrix_Categories.h \
    Matrix_extensions.h \
    MDS.h \
    MFCC.h \
    Minimizers.h \
    Pattern.h \
    PCA.h \
    PCA_def.h \
    Pitch_extensions.h \
    Polygon_extensions.h \
    Polynomial.h \
    Polynomial_def.h \
    Procrustes.h \
    Procrustes_def.h \
    Proximity.h \
    Resonator.h \
    Sampled2.h \
    Sampled2_def.h \
    Sound_and_FilterBank.h \
    Sound_and_PCA.h \
    Sound_and_Spectrogram_extensions.h \
    Sound_extensions.h \
    Sound_to_DTW.h \
    Sound_to_MFCC.h \
    Sound_to_Pitch2.h \
    Sound_to_SPINET.h \
    Sounds_to_DTW.h \
    Spectrogram_extensions.h \
    Spectrum_extensions.h \
    SpeechSynthesizer.h \
    SpeechSynthesizer_and_TextGrid.h \
    SpeechSynthesizer_def.h \
    SPINET.h \
    SPINET_def.h \
    SPINET_to_Pitch.h \
    SSCP.h \
    SSCP_def.h \
    Strings_extensions.h \
    Table_extensions.h \
    TableOfReal_and_Permutation.h \
    TableOfReal_and_SVD.h \
    TableOfReal_extensions.h \
    TextGrid_extensions.h \
    Vowel_def.h \
    VowelEditor.h

