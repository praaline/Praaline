# Praaline
# (c) George Christodoulides 2012-2014

! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

TARGET = Praaline
linux*:TARGET = praaline
solaris*:TARGET = praaline

TEMPLATE = app

CONFIG += qt thread warn_on stl rtti exceptions c++11

QT += core gui network xml sql svg opengl multimedia
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets printsupport
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
include(../external/qtilities/src/Qtilities.pri)
include(../external/qtilities/src/Dependencies.pri)
INCLUDEPATH += external/qtilities/include

# Application components
LIBS +=  \
        -L../pngui/$${COMPONENTSPATH} -lpngui \
        -L../pnlib/$${COMPONENTSPATH} -lpnlib \
        -L../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../pngui/$${COMPONENTSPATH}/libpngui.a \
        ../pnlib/$${COMPONENTSPATH}/libpnlib.a \
        ../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

# Will build the final executable in the main project directory.
#!mac*:
TARGET = ../praaline

SOURCES += main.cpp \
    corporamanager.cpp \
    corpus/corpusmode.cpp \
    corpus/corpusmodewidget.cpp \
    annotation/annotationmode.cpp \
    annotation/annotationmodewidget.cpp \
    visualisation/visualisationmode.cpp \
    visualisation/visualisationmodewidget.cpp \
    query/querymode.cpp \
    query/querymodewidget.cpp \
    statistics/statisticsmode.cpp \
    statistics/statisticsmodewidget.cpp \
    scripting/scriptingmode.cpp \
    scripting/scriptingmodewidget.cpp \
    corpus/corpusexplorerwidget.cpp \
    corpus/corpusexplorertablewidget.cpp \
    corpus/newcorpuswizard.cpp \
    corpus/connecttocorpusdatabasedialog.cpp \
    corpus/addnewcorpusitemdialog.cpp \
    corpus/corpusexploreroptionsdialog.cpp \
    corpus/checkmediafilesdialog.cpp \
    corpus/splitcommunicationsdialog.cpp \
    corpus/corpusstructureeditor/corpusstructureeditorwidget.cpp \
    corpus/corpusstructureeditor/addattributedialog.cpp \
    corpus/corpusstructureeditor/addleveldialog.cpp \
    corpus/importcorpusitemswizard/importcorpusitemswizard.cpp \
    corpus/importcorpusitemswizard/importcorpusitemswizardanalysepage.cpp \
    corpus/importcorpusitemswizard/importcorpusitemswizardcorrespondancespage.cpp \
    corpus/importcorpusitemswizard/importcorpusitemswizardfinalpage.cpp \
    corpus/importcorpusitemswizard/importcorpusitemswizardprocessmediapage.cpp \
    corpus/importcorpusitemswizard/importcorpusitemswizardselectionpage.cpp \
    corpus/exportannotationswizard/exportannotationswizard.cpp \
    corpus/exportannotationswizard/exportannotationswizardpraatpage.cpp \
    corpus/exportmetadatawizard/exportmetadatawizard.cpp \
    annotation/automaticannotationwidget.cpp \
    annotation/manualannotationwidget.cpp \
    annotation/dis/annotationcontrolsdisfluencies.cpp \
    annotation/dis/disfluencyanalyser.cpp \
    annotation/transcriberwidget.cpp \
    annotation/batcheditorwidget.cpp \
    annotation/annotatecorpusitemswindowbase.cpp \
    #visualisation/pitchanalyser.cpp \
    visualisation/audioenabledwidget.cpp \
    query/concordancerwidget.cpp \
    query/createdatasetwidget.cpp \
    query/advancedquerieswidget.cpp \
    statistics/interrateragreement.cpp \
    corpus/importmetadatawizard/importmetadatawizard.cpp \
    corpus/importmetadatawizard/importmetadatawizardbasicinfopage.cpp \
    corpus/importmetadatawizard/importmetadatawizardcolumnspage.cpp \
    corpus/importmetadatawizard/importmetadatawizardfinalpage.cpp \
    query/concordancerquickwidget.cpp \
    annotation/compareannotationswidget.cpp \
    visualisation/pitchanalyser.cpp \
    praalinesplash.cpp \
    networkpermissiontester.cpp \
    visualisation/timelinevisualisationwidget.cpp \
    visualisation/globalvisualisationwidget.cpp \
    visualisation/pitchanalyserwidget.cpp \
    visualisation/visualiserwidget.cpp \
    query/extractsoundbiteswidget.cpp \
    corpus/MergeCorporaDialog.cpp \
    statistics/CountStatisticsWidget.cpp \
    corpus/corpusdatabaseconnectiondialog.cpp \
    PraalineMainWindow.cpp \
    visualisation/SimpleVisualiserWidget.cpp

HEADERS  += \
    corporamanager.h \
    corpus/corpusmode.h \
    corpus/corpusmodewidget.h \
    annotation/annotationmode.h \
    annotation/annotationmodewidget.h \
    visualisation/visualisationmode.h \
    visualisation/visualisationmodewidget.h \
    query/querymode.h \
    query/querymodewidget.h \
    statistics/statisticsmode.h \
    statistics/statisticsmodewidget.h \
    scripting/scriptingmode.h \
    scripting/scriptingmodewidget.h \
    corpus/corpusexplorerwidget.h \
    corpus/corpusexplorertablewidget.h \
    corpus/newcorpuswizard.h \
    corpus/connecttocorpusdatabasedialog.h \
    corpus/addnewcorpusitemdialog.h \
    corpus/corpusexploreroptionsdialog.h \
    corpus/checkmediafilesdialog.h \
    corpus/splitcommunicationsdialog.h \
    corpus/corpusstructureeditor/corpusstructureeditorwidget.h \
    corpus/corpusstructureeditor/addattributedialog.h \
    corpus/corpusstructureeditor/addleveldialog.h \
    corpus/importcorpusitemswizard/importcorpusitemswizard.h \
    corpus/importcorpusitemswizard/importcorpusitemswizardanalysepage.h \
    corpus/importcorpusitemswizard/importcorpusitemswizardcorrespondancespage.h \
    corpus/importcorpusitemswizard/importcorpusitemswizardfinalpage.h \
    corpus/importcorpusitemswizard/importcorpusitemswizardprocessmediapage.h \
    corpus/importcorpusitemswizard/importcorpusitemswizardselectionpage.h \
    corpus/exportannotationswizard/exportannotationswizard.h \
    corpus/exportannotationswizard/exportannotationswizardpraatpage.h \
    annotation/automaticannotationwidget.h \
    annotation/iannotationplugin.h \
    annotation/manualannotationwidget.h \
    annotation/dis/annotationcontrolsdisfluencies.h \
    annotation/dis/disfluencyanalyser.h \
    annotation/transcriberwidget.h \
    annotation/batcheditorwidget.h \
    annotation/annotatecorpusitemswindowbase.h \
    #visualisation/pitchanalyser.h \
    visualisation/audioenabledwidget.h \
    query/concordancerwidget.h \
    query/createdatasetwidget.h \
    query/advancedquerieswidget.h \
    statistics/interrateragreement.h \
    corpus/exportmetadatawizard/exportmetadatawizard.h \
    corpus/importmetadatawizard/importmetadatawizard.h \
    corpus/importmetadatawizard/importmetadatawizardbasicinfopage.h \
    corpus/importmetadatawizard/importmetadatawizardcolumnspage.h \
    corpus/importmetadatawizard/importmetadatawizardfinalpage.h \
    query/concordancerquickwidget.h \
    annotation/compareannotationswidget.h \
    visualisation/pitchanalyser.h \
    praalinesplash.h \
    networkpermissiontester.h \
    version.h \
    visualisation/timelinevisualisationwidget.h \
    visualisation/globalvisualisationwidget.h \
    visualisation/pitchanalyserwidget.h \
    visualisation/visualiserwidget.h \
    query/extractsoundbiteswidget.h \
    corpus/MergeCorporaDialog.h \
    statistics/CountStatisticsWidget.h \
    corpus/importcorpusitemswizard/importannotations.h \
    corpus/corpusdatabaseconnectiondialog.h \
    PraalineMainWindow.h \
    visualisation/SimpleVisualiserWidget.h

FORMS    += mainwindow.ui \
    corpus/corpusmodewidget.ui \
    annotation/annotationmodewidget.ui \
    visualisation/visualisationmodewidget.ui \
    query/querymodewidget.ui \
    statistics/statisticsmodewidget.ui \
    scripting/scriptingmodewidget.ui \
    corpus/corpusexplorerwidget.ui \
    corpus/corpusexplorertablewidget.ui \
    corpus/newcorpuswizard.ui \
    corpus/connecttocorpusdatabasedialog.ui \
    corpus/addnewcorpusitemdialog.ui \
    corpus/corpusexploreroptionsdialog.ui \
    corpus/checkmediafilesdialog.ui \
    corpus/splitcommunicationsdialog.ui \
    corpus/corpusstructureeditor/corpusstructureeditorwidget.ui \
    corpus/corpusstructureeditor/addattributedialog.ui \
    corpus/corpusstructureeditor/addleveldialog.ui \
    corpus/importcorpusitemswizard/importcorpusitemswizard.ui \
    corpus/importcorpusitemswizard/importcorpusitemswizardanalysepage.ui \
    corpus/importcorpusitemswizard/importcorpusitemswizardcorrespondancespage.ui \
    corpus/importcorpusitemswizard/importcorpusitemswizardfinalpage.ui \
    corpus/importcorpusitemswizard/importcorpusitemswizardprocessmediapage.ui \
    corpus/importcorpusitemswizard/importcorpusitemswizardselectionpage.ui \
    corpus/exportannotationswizard/exportannotationswizard.ui \
    corpus/exportannotationswizard/exportannotationswizardpraatpage.ui \
    annotation/transcriberwidget.ui \
    annotation/automaticannotationwidget.ui \
    annotation/manualannotationwidget.ui \
    annotation/dis/annotationcontrolsdisfluencies.ui \
    annotation/batcheditwidget.ui \
    query/concordancerwidget.ui \
    query/createdatasetwidget.ui \
    query/advancedquerieswidget.ui \
    corpus/exportmetadatawizard/exportmetadatawizard.ui \
    corpus/importmetadatawizard/importmetadatawizard.ui \
    corpus/importmetadatawizard/importmetadatawizardbasicinfopage.ui \
    corpus/importmetadatawizard/importmetadatawizardcolumnspage.ui \
    corpus/importmetadatawizard/importmetadatawizardfinalpage.ui \
    query/concordancerquickwidget.ui \
    annotation/compareannotationswidget.ui \
    visualisation/timelinevisualisationwidget.ui \
    visualisation/globalvisualisationwidget.ui \
    query/extractsoundbiteswidget.ui \
    corpus/MergeCorporaDialog.ui \
    statistics/CountStatisticsWidget.ui \
    corpus/corpusdatabaseconnectiondialog.ui

RESOURCES += \
    praaline.qrc \
    ../pngui/grid/qaiv.qrc

DISTFILES +=
