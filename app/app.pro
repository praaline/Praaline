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
win* {
    CONFIG( debug, debug|release ) {
        # debug
        LIBPATH_QSCINTILLA=$$OUT_PWD/../libs/qscintilla/Qt4Qt5/debug
    } else {
        # release
        LIBPATH_QSCINTILLA=$$OUT_PWD/../libs/qscintilla/Qt4Qt5/release
    }
}
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
    corpus/AddNewCorpusItemDialog.cpp \
    corpus/CheckMediaFilesDialog.cpp \
    corpus/ConnectToCorpusDatabaseDialog.cpp \
    corpus/CorpusDatabaseConnectionDialog.cpp \
    corpus/CorpusExplorerOptionsDialog.cpp \
    corpus/CorpusMode.cpp \
    corpus/CorpusModeWidget.cpp \
    corpus/SplitCommunicationsDialog.cpp \
    statistics/InterraterAgreement.cpp \
    statistics/StatisticsMode.cpp \
    statistics/StatisticsModeWidget.cpp \
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
    query/dataseteditor/DatasetEditorWidget.cpp \
    CorpusRepositoriesManager.cpp \
    corpus/NewCorpusRepositoryWizard.cpp \
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
    corpus/importannotations/ImportAnnotationsWizard.cpp

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
    corpus/AddNewCorpusItemDialog.h \
    corpus/CheckMediaFilesDialog.h \
    corpus/ConnectToCorpusDatabaseDialog.h \
    corpus/CorpusDatabaseConnectionDialog.h \
    corpus/CorpusExplorerOptionsDialog.h \
    corpus/CorpusMode.h \
    corpus/CorpusModeWidget.h \
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
    query/dataseteditor/DatasetEditorModels.h \
    CorpusRepositoriesManager.h \
    corpus/NewCorpusRepositoryWizard.h \
    corpus/exportannotations/ExportAnnotationsWizard.h \
    corpus/exportannotations/ExportAnnotationsWizardPraatPage.h \
    corpus/exportmetadata/ExportMetadataWizard.h \
    corpus/importcorpusitems/ImportAnnotations.h \
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
    corpus/importannotations/ImportAnnotationsWizard.h

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
    corpus/CorpusModeWidget.ui \
    corpus/CorpusExplorerWidget.ui \
    corpus/CorpusExplorerTableWidget.ui \
    corpus/CorpusExplorerOptionsDialog.ui \
    corpus/CorpusDatabaseConnectionDialog.ui \
    corpus/ConnectToCorpusDatabaseDialog.ui \
    corpus/CheckMediaFilesDialog.ui \
    corpus/AddNewCorpusItemDialog.ui \
    annotation/BatchEditorWidget.ui \
    corpus/structureeditors/MetadataStructureEditor.ui \
    corpus/structureeditors/AnnotationStructureEditor.ui \
    help/ContentWidget.ui \
    help/HelpModeConfig.ui \
    corpus/structureeditors/AddAttributeDialog.ui \
    corpus/structureeditors/AddLevelDialog.ui \
    corpus/NewCorpusRepositoryWizard.ui \
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
    corpus/importannotations/ImportAnnotationsWizard.ui

RESOURCES += \
    praaline.qrc \
    ../pngui/grid/qaiv.qrc

DISTFILES +=
