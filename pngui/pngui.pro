# Praaline
# (c) George Christodoulides 2012-2014

! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib qt thread warn_on stl rtti exceptions c++11

QT += xml sql gui printsupport
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

TARGET = pngui

INCLUDEPATH += . .. ../pncore ../svcore ../svgui
DEPENDPATH += . .. ../pncore ../svcore ../svgui

# Qtilities configuration
QTILITIES += logging
QTILITIES += core
QTILITIES += coregui
include(../external/qtilities/src/Qtilities.pri)
include(../external/qtilities/src/Dependencies.pri)
INCLUDEPATH += external/qtilities/include

FORMS += \
    grid/qadvancedtableview.ui \
    grid/qconditionaldecorationdialog.ui \
    widgets/annotationverticaltimelineeditor.ui \
    widgets/corpuslevelsattributesselector.ui \
    widgets/gridviewwidget.ui \
    widgets/corpusitemselectorwidget.ui \
    widgets/annotationtimelineeditor.ui \
    widgets/timelineeditorconfigwidget.ui \
    widgets/gridviewfinddialog.ui

HEADERS += \
    grid/qabstractfilter.h \
    grid/qabstractfiltermodel.h \
    grid/qabstractfilterproxymodel.h \
    grid/qabstractmodelitemdecoration.h \
    grid/qadvancedheaderview.h \
    grid/qadvancedtableview.h \
    grid/qadvancedtableview_p.h \
    grid/qaiv.h \
    grid/qaivlib_global.h \
    grid/qautofilter.h \
    grid/qautofilter_p.h \
    grid/qcheckstateproxymodel.h \
    grid/qclickablelabel.h \
    grid/qconditionaldecoration.h \
    grid/qconditionaldecorationdialog.h \
    grid/qconditionaldecorationdialog_p.h \
    grid/qconditionaldecorationproxymodel.h \
    grid/qfiltereditorpopupwidget.h \
    grid/qfiltereditorwidget.h \
    grid/qfiltergroup.h \
    grid/qfiltermodel.h \
    grid/qfiltermodelproxy.h \
    grid/qfilterview.h \
    grid/qfilterviewconnector.h \
    grid/qfilterviewitemdelegate.h \
    grid/qfixedrowsheaderview.h \
    grid/qfixedrowstableview.h \
    grid/qgroupingproxymodel.h \
    grid/qgroupingproxymodel_p.h \
    grid/qheaderviewconnector.h \
    grid/qmimedatautil.h \
    grid/qrangefilter.h \
    grid/qrangefilter_p.h \
    grid/qregexpfilter.h \
    grid/qregexpfilter_p.h \
    grid/qselectionlistfilter.h \
    grid/qselectionlistfilter_p.h \
    grid/qshareditemselectionmodel.h \
    grid/qsinglecolumnproxymodel.h \
    grid/qspecialfilter.h \
    grid/qspecialfilter_p.h \
    grid/qtextfilter.h \
    grid/qtextfilter_p.h \
    grid/quniquevaluesproxymodel.h \
    grid/qvaluefilter.h \
    grid/qvaluefilter_p.h \
    grid/spinboxitemdelegate.h \
    model/checkableproxymodel.h \
    model/delayedexecutiontimer.h \
    model/corpus/metadatastructuretreemodel.h \
    model/corpus/annotationstructuretreemodel.h \
    model/corpus/corpuscommunicationtablemodel.h \
    model/corpus/corpusspeakertablemodel.h \
    model/corpus/corpusrecordingtablemodel.h \
    model/corpus/corpusannotationtablemodel.h \
    model/corpus/corpusparticipationtablemodel.h \
    model/corpus/corpusbookmarkmodel.h \
    model/corpus/corpusexplorertreemodel.h \
    widgets/selectiondialog.h \
    widgets/corpuslevelsattributesselector.h \
    widgets/selectionlistdataproviderproxy.h \
    widgets/metadataeditorwidget.h \
    observers/corpusobserver.h \
    model/query/queryoccurrencetablemodel.h \
    model/query/queryfiltersequencetablemodel.h \
    model/query/querydefinitiontreemodel.h \
    model/annotation/annotationtiermodel.h \
    model/diff/diffsesforintervalstablemodel.h \
    widgets/gridviewwidget.h \
    widgets/checkboxlist.h \
    widgets/corpusitemselectorwidget.h \
    widgets/annotationtimelineeditor.h \
    widgets/timelineeditorconfigwidget.h \
    widgets/gridviewfinddialog.h \
    widgets/waitingspinnerwidget.h \
    model/annotation/AnnotationGridPointModel.h \
    model/annotation/AnnotationGridModel.h \
    layer/AnnotationGridLayer.h \
    layer/PraalineLayersInitialiser.h \
    model/annotation/ProsogramTonalSegmentModel.h \
    model/annotation/ProsogramModel.h \
    layer/ProsogramLayer.h \
    epsengine/EpsEngine.h

SOURCES += \
    grid/qabstractfilter.cpp \
    grid/qabstractfiltermodel.cpp \
    grid/qabstractfilterproxymodel.cpp \
    grid/qabstractmodelitemdecoration.cpp \
    grid/qadvancedheaderview.cpp \
    grid/qadvancedtableview.cpp \
    grid/qaiv.cpp \
    grid/qautofilter.cpp \
    grid/qcheckstateproxymodel.cpp \
    grid/qclickablelabel.cpp \
    grid/qconditionaldecoration.cpp \
    grid/qconditionaldecorationdialog.cpp \
    grid/qconditionaldecorationproxymodel.cpp \
    grid/qfiltereditorpopupwidget.cpp \
    grid/qfiltereditorwidget.cpp \
    grid/qfiltergroup.cpp \
    grid/qfiltermodel.cpp \
    grid/qfiltermodelproxy.cpp \
    grid/qfilterview.cpp \
    grid/qfilterviewconnector.cpp \
    grid/qfilterviewitemdelegate.cpp \
    grid/qfixedrowsheaderview.cpp \
    grid/qfixedrowstableview.cpp \
    grid/qgroupingproxymodel.cpp \
    grid/qheaderviewconnector.cpp \
    grid/qmimedatautil.cpp \
    grid/qrangefilter.cpp \
    grid/qregexpfilter.cpp \
    grid/qselectionlistfilter.cpp \
    grid/qshareditemselectionmodel.cpp \
    grid/qsinglecolumnproxymodel.cpp \
    grid/qspecialfilter.cpp \
    grid/qtextfilter.cpp \
    grid/quniquevaluesproxymodel.cpp \
    grid/qvaluefilter.cpp \
    grid/spinboxitemdelegate.cpp \
    model/checkableproxymodel.cpp \
    model/delayedexecutiontimer.cpp \
    model/corpus/metadatastructuretreemodel.cpp \
    model/corpus/annotationstructuretreemodel.cpp \
    model/corpus/corpuscommunicationtablemodel.cpp \
    model/corpus/corpusspeakertablemodel.cpp \
    model/corpus/corpusrecordingtablemodel.cpp \
    model/corpus/corpusannotationtablemodel.cpp \
    model/corpus/corpusparticipationtablemodel.cpp \
    model/corpus/corpusbookmarkmodel.cpp \
    model/corpus/corpusexplorertreemodel.cpp \
    widgets/selectiondialog.cpp \
    widgets/corpuslevelsattributesselector.cpp \
    widgets/selectionlistdataproviderproxy.cpp \
    widgets/metadataeditorwidget.cpp \
    observers/corpusobserver.cpp \
    model/query/queryoccurrencetablemodel.cpp \
    model/query/queryfiltersequencetablemodel.cpp \
    model/query/querydefinitiontreemodel.cpp \
    model/annotation/annotationtiermodel.cpp \
    model/diff/diffsesforintervalstablemodel.cpp \
    widgets/gridviewwidget.cpp \
    widgets/checkboxlist.cpp \
    widgets/corpusitemselectorwidget.cpp \
    widgets/annotationtimelineeditor.cpp \
    widgets/timelineeditorconfigwidget.cpp \
    widgets/gridviewfinddialog.cpp \
    widgets/waitingspinnerwidget.cpp \
    model/annotation/AnnotationGridModel.cpp \
    layer/AnnotationGridLayer.cpp \
    layer/PraalineLayersInitialiser.cpp \
    model/annotation/ProsogramModel.cpp \
    layer/ProsogramLayer.cpp \
    epsengine/EpsPaintEngine.cpp \
    epsengine/EpsPaintDevice.cpp
