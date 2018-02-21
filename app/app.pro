# Praaline
# (c) George Christodoulides 2012-2017

! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

# Will build the final executable in the main project directory.
TARGET = ../Praaline
!mac*:TARGET = ../praaline

TEMPLATE = app

CONFIG += qt thread warn_on stl rtti exceptions c++14

QT += core gui network xml sql svg opengl multimedia multimediawidgets help concurrent charts
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets printsupport
    macx:QT += macextras
}

# Application icon
win32 {
    RC_FILE = praaline.rc
}
macx* {
    # QMAKE_INFO_PLIST = ../deploy/osx/Info.plist
    ICON = praaline.icns
}

DEPENDPATH += . ../pncore ../pnlib ../pngui
INCLUDEPATH += . .. ../.. ../pncore ../pnlib ../pngui

# Sonic Visualiser libraries
include (svinclude.pri)

# Qtilities configuration
QTILITIES += logging
QTILITIES += core
QTILITIES += coregui
QTILITIES += extension_system
QTILITIES += project_management
include(../dependencies/qtilities/src/Qtilities.pri)
include(../dependencies/qtilities/src/Dependencies.pri)
INCLUDEPATH += dependencies/qtilities/include

# QScintilla
INCLUDEPATH += $$PWD/../dependencies/qscintilla/Qt4Qt5
LIBPATH_QSCINTILLA=$$OUT_PWD/../dependencies/qscintilla/Qt4Qt5
win* {
    CONFIG( debug, debug|release ) {
        LIBPATH_QSCINTILLA=$$OUT_PWD/../dependencies/qscintilla/Qt4Qt5/debug
        LIBS += -L$$LIBPATH_QSCINTILLA -lqscintilla2_qt5_debug
    } else {
        LIBPATH_QSCINTILLA=$$OUT_PWD/../dependencies/qscintilla/Qt4Qt5/release
        LIBS += -L$$LIBPATH_QSCINTILLA -lqscintilla2_qt5
    }
}
macx* {
    LIBPATH_QSCINTILLA=$$OUT_PWD/../dependencies/qscintilla/Qt4Qt5
    CONFIG( debug, debug|release ) {
        LIBS += -L$$LIBPATH_QSCINTILLA -lqscintilla2_qt5_debug
    } else {
        LIBS += -L$$LIBPATH_QSCINTILLA -lqscintilla2_qt5
    }
}
linux* {
    LIBPATH_QSCINTILLA=$$OUT_PWD/../dependencies/qscintilla/Qt4Qt5
    LIBS += -L$$LIBPATH_QSCINTILLA -lqscintilla2_qt5
}
DEPENDPATH += $$PWD/../dependencies/qscintilla/Qt4Qt5

# Node editor
DEFINES += NODE_EDITOR_STATIC

# For QXlsx
QT += core gui gui-private
DEFINES += XLSX_NO_LIB

# For QCustomPlot
INCLUDEPATH += ../dependencies/qcustomplot
LIBS += -L../dependencies/qcustomplot/$${COMPONENTSPATH} -lqcustomplot
PRE_TARGETDEPS += ../dependencies/qcustomplot/$${COMPONENTSPATH}/libqcustomplot.a

# Linking dynamically with PocketSphinx ====== this should go into a plugin
win32-g++ {
    INCLUDEPATH += $$PWD/../dependency-builds/pn/win32-mingw/include
    LIBS += -L$$PWD/../dependency-builds/pn/win32-mingw/lib -lpocketsphinx -lsphinxbase -liconv
}
win32-msvc* {
    INCLUDEPATH += $$PWD/../dependency-builds/pn/win32-msvc/include
    LIBS += -L$$PWD/dependency-builds/pn/win32-msvc/lib -lpocketsphinx -lsphinxbase -liconv
}
unix {
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib -lpocketsphinx -lsphinxbase
}


# Application components
LIBS +=  \
        -L../pngui/$${COMPONENTSPATH} -lpngui \
        -L../pnlib/asr/$${COMPONENTSPATH} -lpraaline-asr \
        -L../pnlib/crf/$${COMPONENTSPATH} -lpraaline-crf \
        -L../pnlib/diff/$${COMPONENTSPATH} -lpraaline-diff \
        -L../pnlib/featextract/$${COMPONENTSPATH} -lpraaline-featextract \
        -L../pnlib/media/$${COMPONENTSPATH} -lpraaline-media \
        -L../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../pngui/$${COMPONENTSPATH}/libpngui.a \
        ../pnlib/asr/$${COMPONENTSPATH}/libpraaline-asr.a \
        ../pnlib/crf/$${COMPONENTSPATH}/libpraaline-crf.a \
        ../pnlib/diff/$${COMPONENTSPATH}/libpraaline-diff.a \
        ../pnlib/featextract/$${COMPONENTSPATH}/libpraaline-featextract.a \
        ../pnlib/mediautil/$${COMPONENTSPATH}/libpraaline-mediautil.a \
        ../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

SOURCES += main.cpp \
    #visualisation/pitchanalyser.cpp \
    PraalineMainWindow.cpp \
    NetworkPermissionTester.cpp \
    PraalineSplash.cpp \
    CorpusRepositoriesManager.cpp \
    corpus/CorpusExplorerTableWidget.cpp \
    corpus/CorpusExplorerWidget.cpp \
    corpus/CorpusDatabaseConnectionDialog.cpp \
    corpus/CorpusExplorerOptionsDialog.cpp \
    corpus/CorpusMode.cpp \
    corpus/CorpusModeWidget.cpp \
    corpus/utilities/SplitCommunicationsDialog.cpp \
    corpus/utilities/CheckMediaFilesDialog.cpp \
    corpus/utilities/MergeCorporaDialog.cpp \
    corpus/utilities/DecodeFilenameToMetadataDialog.cpp \
    corpus/structureeditors/MetadataStructureEditor.cpp \
    corpus/structureeditors/AnnotationStructureEditor.cpp \
    corpus/structureeditors/AddAttributeDialog.cpp \
    corpus/structureeditors/AddLevelDialog.cpp \
    corpus/structureeditors/NameValueListEditor.cpp \
    corpus/exportannotations/ExportAnnotationsWizard.cpp \
    corpus/exportannotations/ExportAnnotationsWizardPraatPage.cpp \
    corpus/exportmetadata/ExportMetadataWizard.cpp \
    corpus/importcorpusitems/ImportCorpusItemsWizard.cpp \
    corpus/importcorpusitems/ImportCorpusItemsWizardAnalysePage.cpp \
    corpus/importcorpusitems/ImportCorpusItemsWizardCorrespondancesPage.cpp \
    corpus/importcorpusitems/ImportCorpusItemsWizardFinalPage.cpp \
    corpus/importcorpusitems/ImportCorpusItemsWizardProcessMediaPage.cpp \
    corpus/importcorpusitems/ImportCorpusItemsWizardSelectionPage.cpp \
    corpus/importmetadata/ImportMetadataWizard.cpp \
    corpus/importmetadata/ImportMetadataWizardBasicInfoPage.cpp \
    corpus/importmetadata/ImportMetadataWizardColumnsPage.cpp \
    corpus/importmetadata/ImportMetadataWizardFinalPage.cpp \
    corpus/importannotations/ImportAnnotationsWizard.cpp \
    corpus/CorpusRepositoryPropertiesDialog.cpp \
    corpus/CorpusRepositoryCreateWizard.cpp \
    corpus/importannotations/ImportTableDataPreviewWizardPage.cpp \
    corpus/exportannotations/ExportAnnotationsWizardTimelinePage.cpp \
    annotation/AnnotationMode.cpp \
    annotation/AnnotationModeWidget.cpp \
    annotation/AutomaticAnnotationWidget.cpp \
    annotation/BatchEditorWidget.cpp \
    annotation/CompareAnnotationsWidget.cpp \
    annotation/ManualAnnotationWidget.cpp \
    annotation/TranscriberWidget.cpp \
    annotation/editors/AnnotationEditorBase.cpp \
    annotation/editors/TranscriptAnnotationEditor.cpp \
    annotation/editors/LaTexTranscriptionBuilder.cpp \
    annotation/SpeechRecognitionWidget.cpp \
    annotation/asr/AutomaticTranscriptionWidget.cpp \
    annotation/asr/ASRModuleWidgetBase.cpp \
    annotation/editors/GroupingAnnotationEditor.cpp \
    annotation/editors/AnnotationMultiTierEditor.cpp \
    annotation/calculate/TidyUpAnnotationsDialog.cpp \
    annotation/calculate/AddCalculatedAnnotationDialog.cpp \
    annotation/calculate/ComposeTranscriptionDialog.cpp \
    annotation/calculate/CreateSequenceAnnotationDialog.cpp \
    annotation/dis/AnnotationControlsDisfluencies.cpp \
    annotation/dis/DisfluencyAnalyser.cpp \
    annotation/asr/LanguageModelBuilderWidget.cpp \
    annotation/asr/LongSoundAlignerWidget.cpp \
    annotation/asr/ASRModuleVisualiserWidgetBase.cpp \
    visualisation/AudioEnabledWidget.cpp \
    visualisation/GlobalVisualisationWidget.cpp \
    visualisation/PitchAnalyser.cpp \
    visualisation/PitchAnalyserWidget.cpp \
    visualisation/TimelineVisualisationWidget.cpp \
    visualisation/VisualisationMode.cpp \
    visualisation/VisualisationModeWidget.cpp \
    visualisation/VisualiserWidget.cpp \
    visualisation/SimpleVisualiserWidget.cpp \    
    visualisation/ExportVisualisationDialog.cpp \
    query/sql/SqlTextEdit.cpp \
    query/sql/SqlUiLexer.cpp \
    query/AdvancedQueriesWidget.cpp \
    query/ConcordancerQuickWidget.cpp \
    query/ConcordancerWidget.cpp \
    query/CreateDatasetWidget.cpp \
    query/ExtractSoundBitesWidget.cpp \
    query/QueryMode.cpp \
    query/QueryModeWidget.cpp \
    query/AnnotationBrowserWidget.cpp \
    query/dataseteditor/DatasetEditorWidget.cpp \
    scripting/ScriptingMode.cpp \
    scripting/ScriptingModeWidget.cpp \
    scripting/ScriptEditorWidget.cpp \
    statistics/StatisticsMode.cpp \
    statistics/StatisticsModeWidget.cpp \
    help/SearchWidgetFactory.cpp \
    help/IndexWidgetFactory.cpp \
    help/HelpMode.cpp \
    help/HelpModeWidget.cpp \
    help/ContentWidgetFactory.cpp \
    help/HelpBrowser.cpp \
    help/HelpModeConfig.cpp \
    statistics/StatisticalAnalysisBasicWidget.cpp \
    statistics/temporal/AnalyserSegmentDuration.cpp \
    statistics/temporal/StatisticsPluginTemporal.cpp \
    statistics/basic/StatisticsPluginBasic.cpp \
    statistics/interrater/StatisticsPluginInterrater.cpp \
    statistics/prosody/StatisticsPluginProsody.cpp \
    statistics/prosody/AnalyserMacroprosodyWidget.cpp \
    statistics/basic/StatisticsPluginBasicWidget.cpp \
    statistics/interrater/KappaStatisticsWidget.cpp \
    statistics/prosody/AnalyserMacroprosody.cpp \
    statistics/temporal/AnalyserTemporalItem.cpp \
    statistics/temporal/AnalyserTemporal.cpp \
    statistics/prosody/GlobalProsodicProfileWidget.cpp \
    statistics/temporal/AnalyserTemporalWidget.cpp \
    statistics/temporal/PauseLengthDistributionWidget.cpp \   
    statistics/interrater/KappaStatisticsCalculator.cpp \
    statistics/disfluencies/StatisticsPluginDisfluencies.cpp \
    statistics/StatisticalMeasureAggregator.cpp \
    statistics/disfluencies/AnalyserDisfluencies.cpp \
    statistics/disfluencies/AnalyserDisfluenciesItem.cpp \
    statistics/disfluencies/AnalyserDisfluenciesWidget.cpp \
    updater/Updater.cpp \
    updater/QSimpleUpdater.cpp \
    updater/Downloader.cpp \
    corpus/utilities/MergeCommunicationsDialog.cpp \
    annotation/asr/ForcedAlignmentWidget.cpp \
    web/PraalineWebsite.cpp \
    welcome/WelcomeMode.cpp \
    web/WebDesignerMode.cpp \
    welcome/WelcomeModeWidget.cpp \
    web/WebDesignerModeWidget.cpp \
    query/dataseteditor/AnnotationLevelDataModel.cpp

HEADERS  += \
    #visualisation/pitchanalyser.h \
    PraalineMainWindow.h \
    NetworkPermissionTester.h \
    PraalineSplash.h \
    Version.h \
    CorpusRepositoriesManager.h \
    interfaces/IAnnotationPlugin.h \
    interfaces/IStatisticsPlugin.h \
    corpus/CorpusExplorerTableWidget.h \
    corpus/CorpusExplorerWidget.h \
    corpus/CorpusDatabaseConnectionDialog.h \
    corpus/CorpusExplorerOptionsDialog.h \
    corpus/CorpusMode.h \
    corpus/CorpusModeWidget.h \
    corpus/structureeditors/MetadataStructureEditor.h \
    corpus/structureeditors/AnnotationStructureEditor.h \
    corpus/structureeditors/AddAttributeDialog.h \
    corpus/structureeditors/AddLevelDialog.h \
    corpus/structureeditors/NameValueListEditor.h \
    corpus/exportannotations/ExportAnnotationsWizard.h \
    corpus/exportannotations/ExportAnnotationsWizardPraatPage.h \
    corpus/exportannotations/ExportAnnotationsWizardTimelinePage.h \
    corpus/exportmetadata/ExportMetadataWizard.h \
    corpus/importcorpusitems/ImportCorpusItemsWizard.h \
    corpus/importcorpusitems/ImportCorpusItemsWizardAnalysePage.h \
    corpus/importcorpusitems/ImportCorpusItemsWizardCorrespondancesPage.h \
    corpus/importcorpusitems/ImportCorpusItemsWizardFinalPage.h \
    corpus/importcorpusitems/ImportCorpusItemsWizardProcessMediaPage.h \
    corpus/importcorpusitems/ImportCorpusItemsWizardSelectionPage.h \
    corpus/importmetadata/ImportMetadataWizard.h \
    corpus/importmetadata/ImportMetadataWizardBasicInfoPage.h \
    corpus/importmetadata/ImportMetadataWizardColumnsPage.h \
    corpus/importmetadata/ImportMetadataWizardFinalPage.h \
    corpus/importannotations/ImportAnnotationsWizard.h \
    corpus/importannotations/ImportTableDataPreviewWizardPage.h \
    corpus/CorpusRepositoryPropertiesDialog.h \
    corpus/CorpusRepositoryCreateWizard.h \
    corpus/utilities/MergeCorporaDialog.h \
    corpus/utilities/CheckMediaFilesDialog.h \
    corpus/utilities/DecodeFilenameToMetadataDialog.h \
    corpus/utilities/SplitCommunicationsDialog.h \
    annotation/asr/LanguageModelBuilderWidget.h \
    annotation/dis/AnnotationControlsDisfluencies.h \
    annotation/dis/DisfluencyAnalyser.h \
    annotation/AnnotationMode.h \
    annotation/AnnotationModeWidget.h \
    annotation/AutomaticAnnotationWidget.h \
    annotation/BatchEditorWidget.h \
    annotation/CompareAnnotationsWidget.h \
    annotation/ManualAnnotationWidget.h \
    annotation/TranscriberWidget.h \
    annotation/calculate/TidyUpAnnotationsDialog.h \
    annotation/editors/AnnotationEditorBase.h \
    annotation/editors/TranscriptAnnotationEditor.h \
    annotation/editors/LaTexTranscriptionBuilder.h \
    annotation/SpeechRecognitionWidget.h \
    annotation/asr/AutomaticTranscriptionWidget.h \
    annotation/asr/ASRModuleWidgetBase.h \
    annotation/editors/GroupingAnnotationEditor.h \
    annotation/editors/AnnotationMultiTierEditor.h \
    annotation/calculate/AddCalculatedAnnotationDialog.h \
    annotation/calculate/ComposeTranscriptionDialog.h \
    annotation/calculate/CreateSequenceAnnotationDialog.h \
    annotation/asr/LongSoundAlignerWidget.h \
    annotation/asr/ASRModuleVisualiserWidgetBase.h \
    visualisation/AudioEnabledWidget.h \
    visualisation/GlobalVisualisationWidget.h \
    visualisation/PitchAnalyser.h \
    visualisation/PitchAnalyserWidget.h \
    visualisation/TimelineVisualisationWidget.h \
    visualisation/VisualisationMode.h \
    visualisation/VisualisationModeWidget.h \
    visualisation/VisualiserWidget.h \
    visualisation/SimpleVisualiserWidget.h \
    visualisation/ExportVisualisationDialog.h \
    query/sql/SqlTextEdit.h \
    query/sql/SqlUiLexer.h \
    query/AdvancedQueriesWidget.h \
    query/ConcordancerQuickWidget.h \
    query/ConcordancerWidget.h \
    query/CreateDatasetWidget.h \
    query/ExtractSoundBitesWidget.h \
    query/QueryMode.h \
    query/QueryModeWidget.h \
    query/AnnotationBrowserWidget.h \
    query/dataseteditor/DatasetEditorWidget.h \
    query/dataseteditor/DatasetEditorModels.h \
    statistics/StatisticsMode.h \
    statistics/StatisticsModeWidget.h \
    statistics/StatisticalAnalysisBasicWidget.h \
    statistics/temporal/AnalyserSegmentDuration.h \
    statistics/temporal/StatisticsPluginTemporal.h \
    statistics/basic/StatisticsPluginBasic.h \
    statistics/interrater/StatisticsPluginInterrater.h \
    statistics/prosody/StatisticsPluginProsody.h \
    statistics/prosody/AnalyserMacroprosodyWidget.h \
    statistics/basic/StatisticsPluginBasicWidget.h \
    statistics/interrater/KappaStatisticsWidget.h \
    statistics/prosody/AnalyserMacroprosody.h \
    statistics/temporal/AnalyserTemporalItem.h \
    statistics/temporal/AnalyserTemporal.h \
    statistics/prosody/GlobalProsodicProfileWidget.h \
    statistics/temporal/AnalyserTemporalWidget.h \
    statistics/temporal/PauseLengthDistributionWidget.h \
    statistics/disfluencies/StatisticsPluginDisfluencies.h \
    statistics/StatisticalMeasureAggregator.h \
    statistics/disfluencies/AnalyserDisfluencies.h \
    statistics/disfluencies/AnalyserDisfluenciesItem.h \
    statistics/AnalyserItemBase.h \
    statistics/disfluencies/AnalyserDisfluenciesWidget.h \
    statistics/interrater/KappaStatisticsCalculator.h \
    scripting/ScriptEditorWidget.h \
    scripting/ScriptingMode.h \
    scripting/ScriptingModeWidget.h \
    help/SearchWidgetFactory.h \
    help/IndexWidgetFactory.h \
    help/HelpMode.h \
    help/HelpModeWidget.h \
    help/ContentWidgetFactory.h \
    help/HelpBrowser.h \
    help/HelpModeConfig.h \   
    updater/Updater.h \
    updater/QSimpleUpdater.h \
    updater/Downloader.h \
    corpus/utilities/MergeCommunicationsDialog.h \
    annotation/asr/ForcedAlignmentWidget.h \
    web/PraalineWebsite.h \
    welcome/WelcomeMode.h \
    web/WebDesignerMode.h \
    welcome/WelcomeModeWidget.h \
    web/WebDesignerModeWidget.h \
    query/dataseteditor/AnnotationLevelDataModel.h \
    query/dataseteditor/AnnotationAttributeNodeData.h

FORMS    += \
    MainWindow.ui \
    corpus/CorpusModeWidget.ui \
    corpus/CorpusExplorerWidget.ui \
    corpus/CorpusExplorerTableWidget.ui \
    corpus/CorpusExplorerOptionsDialog.ui \
    corpus/CorpusDatabaseConnectionDialog.ui \
    corpus/CorpusRepositoryPropertiesDialog.ui \
    corpus/CorpusRepositoryCreateWizard.ui \
    corpus/structureeditors/MetadataStructureEditor.ui \
    corpus/structureeditors/AnnotationStructureEditor.ui \
    corpus/structureeditors/AddAttributeDialog.ui \
    corpus/structureeditors/AddLevelDialog.ui \
    corpus/exportannotations/ExportAnnotationsWizard.ui \
    corpus/exportannotations/ExportAnnotationsWizardPraatPage.ui \
    corpus/exportmetadata/ExportMetadataWizard.ui \
    corpus/importcorpusitems/ImportCorpusItemsWizard.ui \
    corpus/importcorpusitems/ImportCorpusItemsWizardAnalysePage.ui \
    corpus/importcorpusitems/ImportCorpusItemsWizardCorrespondancesPage.ui \
    corpus/importcorpusitems/ImportCorpusItemsWizardFinalPage.ui \
    corpus/importcorpusitems/ImportCorpusItemsWizardProcessMediaPage.ui \
    corpus/importcorpusitems/ImportCorpusItemsWizardSelectionPage.ui \
    corpus/importmetadata/ImportMetadataWizard.ui \
    corpus/importmetadata/ImportMetadataWizardBasicInfoPage.ui \
    corpus/importmetadata/ImportMetadataWizardColumnsPage.ui \
    corpus/importmetadata/ImportMetadataWizardFinalPage.ui \
    corpus/importannotations/ImportAnnotationsWizard.ui \
    corpus/importannotations/ImportTableDataPreviewWizardPage.ui \
    corpus/exportannotations/ExportAnnotationsWizardTimelinePage.ui \
    corpus/utilities/CheckMediaFilesDialog.ui \
    corpus/utilities/MergeCorporaDialog.ui \
    corpus/utilities/SplitCommunicationsDialog.ui \
    corpus/utilities/DecodeFilenameToMetadataDialog.ui \
    annotation/BatchEditorWidget.ui \
    annotation/dis/AnnotationControlsDisfluencies.ui \
    annotation/AnnotationModeWidget.ui \
    annotation/AutomaticAnnotationWidget.ui \
    annotation/CompareAnnotationsWidget.ui \
    annotation/ManualAnnotationWidget.ui \
    annotation/TranscriberWidget.ui \
    annotation/calculate/AddCalculatedAnnotationDialog.ui \
    annotation/calculate/ComposeTranscriptionDialog.ui \
    annotation/calculate/CreateSequenceAnnotationDialog.ui \
    annotation/calculate/TidyUpAnnotationsDialog.ui \
    annotation/SpeechRecognitionWidget.ui \
    annotation/asr/AutomaticTranscriptionWidget.ui \
    annotation/asr/LongSoundAlignerWidget.ui \
    annotation/asr/LanguageModelBuilderWidget.ui \
    visualisation/ExportVisualisationDialog.ui \
    visualisation/GlobalVisualisationWidget.ui \
    visualisation/TimelineVisualisationWidget.ui \
    visualisation/VisualisationModeWidget.ui \
    query/AdvancedQueriesWidget.ui \
    query/ConcordancerQuickWidget.ui \
    query/ConcordancerWidget.ui \
    query/CreateDatasetWidget.ui \
    query/ExtractSoundBitesWidget.ui \
    query/QueryModeWidget.ui \
    statistics/StatisticsModeWidget.ui \
    statistics/StatisticalAnalysisBasicWidget.ui \
    statistics/prosody/AnalyserMacroprosodyWidget.ui \
    statistics/basic/StatisticsPluginBasicWidget.ui \
    statistics/interrater/KappaStatisticsWidget.ui \
    statistics/prosody/GlobalProsodicProfileWidget.ui \
    statistics/temporal/AnalyserTemporalWidget.ui \
    statistics/temporal/PauseLengthDistributionWidget.ui \
    statistics/disfluencies/AnalyserDisfluenciesWidget.ui \
    scripting/ScriptingModeWidget.ui \
    help/ContentWidget.ui \
    help/HelpModeConfig.ui \
    updater/Downloader.ui \
    corpus/utilities/MergeCommunicationsDialog.ui \
    annotation/asr/ForcedAlignmentWidget.ui \
    welcome/WelcomeModeWidget.ui \
    web/WebDesignerModeWidget.ui

RESOURCES += \
    praaline.qrc \
    ../pngui/grid/qaiv.qrc

DISTFILES += \
    praaline.ico \
    praaline.rc \
    ../.travis.yml \
    ../qt-installer-noninteractive.qs \
    ../appveyor.yml

