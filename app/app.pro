# Praaline
# (c) George Christodoulides 2012-2017

! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

TARGET = Praaline
linux*:TARGET = praaline
solaris*:TARGET = praaline

TEMPLATE = app

CONFIG += qt thread warn_on stl rtti exceptions c++14

QT += core gui network xml sql svg opengl multimedia multimediawidgets help
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets printsupport
    macx:QT += macextras
}

macx {
    # QMAKE_INFO_PLIST = ../deploy/osx/Info.plist
}

DEPENDPATH += . ../pncore ../pnlib ../pngui
INCLUDEPATH += . .. ../.. ../pncore ../pnlib ../pngui

# Build folder organisation
CONFIG( debug, debug|release ) {
    # debug
    COMPONENTSPATH = build/debug
} else {
    # release
    COMPONENTSPATH = build/release
}

# Sonic Visualiser libraries
include (svinclude.pri)

# Qtilities configuration
QTILITIES += logging
QTILITIES += core
QTILITIES += coregui
QTILITIES += extension_system
QTILITIES += project_management
include(../libs/qtilities/src/Qtilities.pri)
include(../libs/qtilities/src/Dependencies.pri)
INCLUDEPATH += libs/qtilities/include

# QScintilla
INCLUDEPATH += $$PWD/../libs/qscintilla/Qt4Qt5
LIBPATH_QSCINTILLA=$$OUT_PWD/../libs/qscintilla/Qt4Qt5
win* { LIBPATH_QSCINTILLA=$$OUT_PWD/../libs/qscintilla/Qt4Qt5/release }
LIBS += -L$$LIBPATH_QSCINTILLA -lqscintilla2
DEPENDPATH += $$PWD/../libs/qscintilla/Qt4Qt5

# Node editor
DEFINES += NODE_EDITOR_STATIC

# Application components
LIBS +=  \
        -L../pngui/$${COMPONENTSPATH} -lpngui \
        -L../pnlib/asr/$${COMPONENTSPATH} -lpraaline-asr \
        -L../pnlib/crf/$${COMPONENTSPATH} -lpraaline-crf \
        -L../pnlib/diff/$${COMPONENTSPATH} -lpraaline-diff \
        -L../pnlib/featextract/$${COMPONENTSPATH} -lpraaline-featextract \
        -L../pnlib/mediautil/$${COMPONENTSPATH} -lpraaline-mediautil \
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

# Will build the final executable in the main project directory.
#!mac*:
TARGET = ../praaline

SOURCES += main.cpp \
    #visualisation/pitchanalyser.cpp \
    corpus/MergeCorporaDialog.cpp \
    PraalineMainWindow.cpp \
    visualisation/SimpleVisualiserWidget.cpp \
    annotation/asr/LanguageModelBuilderWidget.cpp \
    visualisation/ExportVisualisationDialog.cpp \
    query/sql/SqlTextEdit.cpp \
    query/sql/SqlUiLexer.cpp \
    query/AdvancedQueriesWidget.cpp \
    corpus/CorpusExplorerTableWidget.cpp \
    corpus/CorpusExplorerWidget.cpp \
    query/ConcordancerQuickWidget.cpp \
    query/ConcordancerWidget.cpp \
    query/CreateDatasetWidget.cpp \
    query/ExtractSoundBitesWidget.cpp \
    query/QueryMode.cpp \
    query/QueryModeWidget.cpp \
    scripting/ScriptingMode.cpp \
    annotation/AnnotateCorpusItemsWindowBase.cpp \
    annotation/AnnotationMode.cpp \
    annotation/AnnotationModeWidget.cpp \
    annotation/AutomaticAnnotationWidget.cpp \
    annotation/BatchEditorWidget.cpp \
    annotation/CompareAnnotationsWidget.cpp \
    annotation/ManualAnnotationWidget.cpp \
    annotation/TranscriberWidget.cpp \
    scripting/ScriptingModeWidget.cpp \
    corpus/exportannotationswizard/ExportAnnotationsWizard.cpp \
    corpus/exportannotationswizard/ExportAnnotationsWizardPraatPage.cpp \
    corpus/exportmetadatawizard/ExportMetadataWizard.cpp \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizard.cpp \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardAnalysePage.cpp \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardCorrespondancesPage.cpp \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardFinalPage.cpp \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardProcessMediaPage.cpp \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardSelectionPage.cpp \
    corpus/importmetadatawizard/ImportMetadataWizard.cpp \
    corpus/importmetadatawizard/ImportMetadataWizardBasicInfoPage.cpp \
    corpus/importmetadatawizard/ImportMetadataWizardColumnsPage.cpp \
    corpus/importmetadatawizard/ImportMetadataWizardFinalPage.cpp \
    corpus/AddNewCorpusItemDialog.cpp \
    corpus/CheckMediaFilesDialog.cpp \
    corpus/ConnectToCorpusDatabaseDialog.cpp \
    corpus/CorpusDatabaseConnectionDialog.cpp \
    corpus/CorpusExplorerOptionsDialog.cpp \
    corpus/CorpusMode.cpp \
    corpus/CorpusModeWidget.cpp \
    corpus/NewCorpusWizard.cpp \
    corpus/SplitCommunicationsDialog.cpp \
    statistics/InterraterAgreement.cpp \
    statistics/StatisticsMode.cpp \
    statistics/StatisticsModeWidget.cpp \
    CorporaManager.cpp \
    NetworkPermissionTester.cpp \
    PraalineSplash.cpp \
    visualisation/AudioEnabledWidget.cpp \
    visualisation/GlobalVisualisationWidget.cpp \
    visualisation/PitchAnalyser.cpp \
    visualisation/PitchAnalyserWidget.cpp \
    visualisation/TimelineVisualisationWidget.cpp \
    visualisation/VisualisationMode.cpp \
    visualisation/VisualisationModeWidget.cpp \
    visualisation/VisualiserWidget.cpp \
    statistics/AnalyserTemporal.cpp \
    statistics/AnalyserSpeechRate.cpp \
    annotation/dis/AnnotationControlsDisfluencies.cpp \
    annotation/dis/DisfluencyAnalyser.cpp \
    statistics/AnalyserSegmentDuration.cpp \
    statistics/AnalyserPitch.cpp \
    corpus/structureeditors/MetadataStructureEditor.cpp \
    corpus/structureeditors/AnnotationStructureEditor.cpp \
    help/SearchWidgetFactory.cpp \
    help/IndexWidgetFactory.cpp \
    help/HelpMode.cpp \
    help/HelpModeWidget.cpp \
    help/ContentWidgetFactory.cpp \
    help/HelpBrowser.cpp \
    help/HelpModeConfig.cpp \
    corpus/structureeditors/AddAttributeDialog.cpp \
    corpus/structureeditors/AddLevelDialog.cpp \
    scripting/ScriptEditorWidget.cpp \
    corpus/structureeditors/NameValueListEditor.cpp \
    query/AnnotationBrowserWidget.cpp \
    query/dataseteditor/DatasetEditorWidget.cpp

HEADERS  += \
    #visualisation/pitchanalyser.h \
    corpus/MergeCorporaDialog.h \
    PraalineMainWindow.h \
    visualisation/SimpleVisualiserWidget.h \
    annotation/asr/LanguageModelBuilderWidget.h \
    visualisation/ExportVisualisationDialog.h \
    query/sql/SqlTextEdit.h \
    query/sql/SqlUiLexer.h \
    query/AdvancedQueriesWidget.h \
    corpus/CorpusExplorerTableWidget.h \
    corpus/CorpusExplorerWidget.h \
    query/ConcordancerQuickWidget.h \
    query/ConcordancerWidget.h \
    query/CreateDatasetWidget.h \
    query/ExtractSoundBitesWidget.h \
    query/QueryMode.h \
    query/QueryModeWidget.h \
    annotation/dis/AnnotationControlsDisfluencies.h \
    annotation/dis/DisfluencyAnalyser.h \
    annotation/AnnotateCorpusItemsWindowBase.h \
    annotation/AnnotationMode.h \
    annotation/AnnotationModeWidget.h \
    annotation/AutomaticAnnotationWidget.h \
    annotation/BatchEditorWidget.h \
    annotation/CompareAnnotationsWidget.h \
    annotation/ManualAnnotationWidget.h \
    annotation/TranscriberWidget.h \
    corpus/exportannotationswizard/ExportAnnotationsWizard.h \
    corpus/exportannotationswizard/ExportAnnotationsWizardPraatPage.h \
    corpus/exportmetadatawizard/ExportMetadataWizard.h \
    corpus/importcorpusitemswizard/ImportAnnotations.h \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizard.h \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardAnalysePage.h \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardCorrespondancesPage.h \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardFinalPage.h \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardProcessMediaPage.h \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardSelectionPage.h \
    corpus/importmetadatawizard/ImportMetadataWizard.h \
    corpus/importmetadatawizard/ImportMetadataWizardBasicInfoPage.h \
    corpus/importmetadatawizard/ImportMetadataWizardColumnsPage.h \
    corpus/importmetadatawizard/ImportMetadataWizardFinalPage.h \
    corpus/AddNewCorpusItemDialog.h \
    corpus/CheckMediaFilesDialog.h \
    corpus/ConnectToCorpusDatabaseDialog.h \
    corpus/CorpusDatabaseConnectionDialog.h \
    corpus/CorpusExplorerOptionsDialog.h \
    corpus/CorpusMode.h \
    corpus/CorpusModeWidget.h \
    corpus/NewCorpusWizard.h \
    corpus/SplitCommunicationsDialog.h \
    scripting/ScriptingMode.h \
    scripting/ScriptingModeWidget.h \
    statistics/InterraterAgreement.h \
    statistics/StatisticsMode.h \
    statistics/StatisticsModeWidget.h \
    visualisation/AudioEnabledWidget.h \
    visualisation/GlobalVisualisationWidget.h \
    visualisation/PitchAnalyser.h \
    visualisation/PitchAnalyserWidget.h \
    visualisation/TimelineVisualisationWidget.h \
    visualisation/VisualisationMode.h \
    visualisation/VisualisationModeWidget.h \
    visualisation/VisualiserWidget.h \
    CorporaManager.h \
    NetworkPermissionTester.h \
    PraalineSplash.h \
    Version.h \
    statistics/AnalyserTemporal.h \
    statistics/AnalyserSpeechRate.h \
    statistics/AnalyserSegmentDuration.h \
    statistics/AnalyserPitch.h \
    corpus/structureeditors/MetadataStructureEditor.h \
    corpus/structureeditors/AnnotationStructureEditor.h \
    help/SearchWidgetFactory.h \
    help/IndexWidgetFactory.h \
    help/HelpMode.h \
    help/HelpModeWidget.h \
    help/ContentWidgetFactory.h \
    help/HelpBrowser.h \
    help/HelpModeConfig.h \
    interfaces/IAnnotationPlugin.h \
    interfaces/IStatisticsPlugin.h \
    corpus/structureeditors/AddAttributeDialog.h \
    corpus/structureeditors/AddLevelDialog.h \
    scripting/ScriptEditorWidget.h \
    corpus/structureeditors/NameValueListEditor.h \
    query/AnnotationBrowserWidget.h \
    query/dataseteditor/DatasetEditorWidget.h \
    query/dataseteditor/DatasetEditorModels.h

FORMS    += \
    corpus/MergeCorporaDialog.ui \
    statistics/CountStatisticsWidget.ui \
    annotation/asr/LanguageModelBuilderWidget.ui \
    query/AdvancedQueriesWidget.ui \
    query/ConcordancerQuickWidget.ui \
    query/ConcordancerWidget.ui \
    query/CreateDatasetWidget.ui \
    query/ExtractSoundBitesWidget.ui \
    query/QueryModeWidget.ui \
    annotation/dis/AnnotationControlsDisfluencies.ui \
    annotation/AnnotationModeWidget.ui \
    annotation/AutomaticAnnotationWidget.ui \
    annotation/CompareAnnotationsWidget.ui \
    annotation/ManualAnnotationWidget.ui \
    annotation/TranscriberWidget.ui \
    MainWindow.ui \
    visualisation/ExportVisualisationDialog.ui \
    visualisation/GlobalVisualisationWidget.ui \
    visualisation/TimelineVisualisationWidget.ui \
    visualisation/VisualisationModeWidget.ui \
    statistics/StatisticsModeWidget.ui \
    scripting/ScriptingModeWidget.ui \
    corpus/SplitCommunicationsDialog.ui \
    corpus/NewCorpusWizard.ui \
    corpus/CorpusModeWidget.ui \
    corpus/CorpusExplorerWidget.ui \
    corpus/CorpusExplorerTableWidget.ui \
    corpus/CorpusExplorerOptionsDialog.ui \
    corpus/CorpusDatabaseConnectionDialog.ui \
    corpus/ConnectToCorpusDatabaseDialog.ui \
    corpus/CheckMediaFilesDialog.ui \
    corpus/AddNewCorpusItemDialog.ui \
    corpus/importmetadatawizard/ImportMetadataWizard.ui \
    corpus/importmetadatawizard/ImportMetadataWizardBasicInfoPage.ui \
    corpus/importmetadatawizard/ImportMetadataWizardColumnsPage.ui \
    corpus/importmetadatawizard/ImportMetadataWizardFinalPage.ui \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizard.ui \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardAnalysePage.ui \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardCorrespondancesPage.ui \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardFinalPage.ui \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardProcessMediaPage.ui \
    corpus/importcorpusitemswizard/ImportCorpusItemsWizardSelectionPage.ui \
    corpus/exportmetadatawizard/ExportMetadataWizard.ui \
    corpus/exportannotationswizard/ExportAnnotationsWizard.ui \
    corpus/exportannotationswizard/ExportAnnotationsWizardPraatPage.ui \
    annotation/BatchEditorWidget.ui \
    corpus/structureeditors/MetadataStructureEditor.ui \
    corpus/structureeditors/AnnotationStructureEditor.ui \
    help/ContentWidget.ui \
    help/HelpModeConfig.ui \
    corpus/structureeditors/AddAttributeDialog.ui \
    corpus/structureeditors/AddLevelDialog.ui

RESOURCES += \
    praaline.qrc \
    ../pngui/grid/qaiv.qrc

DISTFILES +=
