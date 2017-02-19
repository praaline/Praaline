# Praaline
# (c) George Christodoulides 2012-2014

! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib qt thread warn_on stl rtti exceptions c++14

QT += xml sql gui printsupport multimedia multimediawidgets concurrent
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
include(../libs/qtilities/src/Qtilities.pri)
include(../libs/qtilities/src/Dependencies.pri)
INCLUDEPATH += libs/qtilities/include

# For QXlsx
QT += core gui gui-private
DEFINES += XLSX_NO_LIB

# For Node Editor
DEFINES += NODE_EDITOR_STATIC

FORMS += \
    widgets/TimelineEditorConfigWidget.ui \
    widgets/GridViewWidget.ui \
    widgets/GridViewFindDialog.ui \
    widgets/CorpusItemSelectorWidget.ui \
    grid/QAdvancedTableView.ui \
    grid/QConditionalDecorationDialog.ui \
    widgets/ImportDataPreviewWidget.ui

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
    model/annotation/AnnotationGridPointModel.h \
    model/annotation/AnnotationGridModel.h \
    layer/AnnotationGridLayer.h \
    layer/PraalineLayersInitialiser.h \
    model/annotation/ProsogramTonalSegmentModel.h \
    model/annotation/ProsogramModel.h \
    layer/ProsogramLayer.h \
    epsengine/EpsEngine.h \
    xlsx/xlsxzipwriter_p.h \
    xlsx/xlsxzipreader_p.h \
    xlsx/xlsxworksheet.h \
    xlsx/xlsxworksheet_p.h \
    xlsx/xlsxworkbook.h \
    xlsx/xlsxworkbook_p.h \
    xlsx/xlsxutility_p.h \
    xlsx/xlsxtheme_p.h \
    xlsx/xlsxstyles_p.h \
    xlsx/xlsxsimpleooxmlfile_p.h \
    xlsx/xlsxsharedstrings_p.h \
    xlsx/xlsxrichstring.h \
    xlsx/xlsxrichstring_p.h \
    xlsx/xlsxrelationships_p.h \
    xlsx/xlsxnumformatparser_p.h \
    xlsx/xlsxmediafile_p.h \
    xlsx/xlsxglobal.h \
    xlsx/xlsxformat.h \
    xlsx/xlsxformat_p.h \
    xlsx/xlsxdrawinganchor_p.h \
    xlsx/xlsxdrawing_p.h \
    xlsx/xlsxdocument.h \
    xlsx/xlsxdocument_p.h \
    xlsx/xlsxdocpropscore_p.h \
    xlsx/xlsxdocpropsapp_p.h \
    xlsx/xlsxdatavalidation.h \
    xlsx/xlsxdatavalidation_p.h \
    xlsx/xlsxcontenttypes_p.h \
    xlsx/xlsxconditionalformatting.h \
    xlsx/xlsxconditionalformatting_p.h \
    xlsx/xlsxcolor_p.h \
    xlsx/xlsxchartsheet.h \
    xlsx/xlsxchartsheet_p.h \
    xlsx/xlsxchart.h \
    xlsx/xlsxchart_p.h \
    xlsx/xlsxcellreference.h \
    xlsx/xlsxcellrange.h \
    xlsx/xlsxcellformula.h \
    xlsx/xlsxcellformula_p.h \
    xlsx/xlsxcell.h \
    xlsx/xlsxcell_p.h \
    xlsx/xlsxabstractsheet.h \
    xlsx/xlsxabstractsheet_p.h \
    xlsx/xlsxabstractooxmlfile.h \
    xlsx/xlsxabstractooxmlfile_p.h \
    model/corpus/CorpusCommunicationTableModel.h \
    model/corpus/AnnotationStructureTreeModel.h \
    model/corpus/CorpusAnnotationTableModel.h \
    model/corpus/CorpusBookmarkModel.h \
    model/corpus/CorpusExplorerTreeModel.h \
    model/corpus/CorpusParticipationTableModel.h \
    model/corpus/CorpusRecordingTableModel.h \
    model/corpus/CorpusSpeakerTableModel.h \
    model/corpus/MetadataStructureTreeModel.h \
    model/query/QueryOccurrenceTableModel.h \
    widgets/MetadataEditorWidget.h \
    widgets/AnnotationTimelineEditor.h \
    widgets/CheckBoxList.h \
    widgets/CorpusItemSelectorWidget.h \
    widgets/CorpusLevelsAttributesSelector.h \
    widgets/GridViewFindDialog.h \
    widgets/GridViewWidget.h \
    widgets/SelectionDialog.h \
    widgets/SelectionListDataProviderProxy.h \
    widgets/TimelineEditorConfigWidget.h \
    widgets/WaitingSpinnerWidget.h \
    widgets/CorpusItemPreview.h \
    widgets/SelectReorderItemsWidget.h \
    observers/CorpusObserver.h \
    model/DelayedExecutionTimer.h \
    model/CheckableProxyModel.h \
    model/diff/DiffSESForIntervalsTableModel.h \
    model/query/QueryDefinitionTreeModel.h \
    model/query/QueryFilterSequenceTableModel.h \
    widgets/MediaPlayerControls.h \
    widgets/MediaPlayerVideoWidget.h \
    widgets/MiniTranscriptionWidget.h \
    model/annotation/AnnotationTierModel.h \
    model/annotation/AnnotationTableModel.h \
    model/NameValueListTableModel.h \
    nodeeditor/Compiler.h \
    nodeeditor/Connection.h \
    nodeeditor/ConnectionBlurEffect.h \
    nodeeditor/ConnectionGeometry.h \
    nodeeditor/ConnectionGraphicsObject.h \
    nodeeditor/ConnectionPainter.h \
    nodeeditor/ConnectionState.h \
    nodeeditor/ConnectionStyle.h \
    nodeeditor/DataModelRegistry.h \
    nodeeditor/Export.h \
    nodeeditor/FlowItemInterface.h \
    nodeeditor/FlowScene.h \
    nodeeditor/FlowView.h \
    nodeeditor/FlowViewStyle.h \
    nodeeditor/Node.h \
    nodeeditor/NodeConnectionInteraction.h \
    nodeeditor/NodeData.h \
    nodeeditor/NodeDataModel.h \
    nodeeditor/NodeGeometry.h \
    nodeeditor/NodeGraphicsObject.h \
    nodeeditor/NodePainter.h \
    nodeeditor/NodeState.h \
    nodeeditor/NodeStyle.h \
    nodeeditor/OperatingSystem.h \
    nodeeditor/PortType.h \
    nodeeditor/Properties.h \
    nodeeditor/QStringStdHash.h \
    nodeeditor/Serializable.h \
    nodeeditor/Style.h \
    nodeeditor/StyleCollection.h \
    model/corpus/CorpusTableModel.h \
    FlowLayout.h \
    widgets/ImportDataPreviewWidget.h

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
    model/annotation/AnnotationGridModel.cpp \
    layer/AnnotationGridLayer.cpp \
    layer/PraalineLayersInitialiser.cpp \
    model/annotation/ProsogramModel.cpp \
    layer/ProsogramLayer.cpp \
    epsengine/EpsPaintEngine.cpp \
    epsengine/EpsPaintDevice.cpp \
    xlsx/xlsxzipwriter.cpp \
    xlsx/xlsxzipreader.cpp \
    xlsx/xlsxworksheet.cpp \
    xlsx/xlsxworkbook.cpp \
    xlsx/xlsxutility.cpp \
    xlsx/xlsxtheme.cpp \
    xlsx/xlsxstyles.cpp \
    xlsx/xlsxsimpleooxmlfile.cpp \
    xlsx/xlsxsharedstrings.cpp \
    xlsx/xlsxrichstring.cpp \
    xlsx/xlsxrelationships.cpp \
    xlsx/xlsxnumformatparser.cpp \
    xlsx/xlsxmediafile.cpp \
    xlsx/xlsxformat.cpp \
    xlsx/xlsxdrawinganchor.cpp \
    xlsx/xlsxdrawing.cpp \
    xlsx/xlsxdocument.cpp \
    xlsx/xlsxdocpropscore.cpp \
    xlsx/xlsxdocpropsapp.cpp \
    xlsx/xlsxdatavalidation.cpp \
    xlsx/xlsxcontenttypes.cpp \
    xlsx/xlsxconditionalformatting.cpp \
    xlsx/xlsxcolor.cpp \
    xlsx/xlsxchartsheet.cpp \
    xlsx/xlsxchart.cpp \
    xlsx/xlsxcellreference.cpp \
    xlsx/xlsxcellrange.cpp \
    xlsx/xlsxcellformula.cpp \
    xlsx/xlsxcell.cpp \
    xlsx/xlsxabstractsheet.cpp \
    xlsx/xlsxabstractooxmlfile.cpp \
    model/corpus/CorpusCommunicationTableModel.cpp \
    model/corpus/AnnotationStructureTreeModel.cpp \
    model/corpus/CorpusAnnotationTableModel.cpp \
    model/corpus/CorpusBookmarkModel.cpp \
    model/corpus/CorpusExplorerTreeModel.cpp \
    model/corpus/CorpusParticipationTableModel.cpp \
    model/corpus/CorpusRecordingTableModel.cpp \
    model/corpus/CorpusSpeakerTableModel.cpp \
    model/corpus/MetadataStructureTreeModel.cpp \
    model/query/QueryOccurrenceTableModel.cpp \
    widgets/MetadataEditorWidget.cpp \
    widgets/AnnotationTimelineEditor.cpp \
    widgets/CheckBoxList.cpp \
    widgets/CorpusItemSelectorWidget.cpp \
    widgets/CorpusLevelsAttributesSelector.cpp \
    widgets/GridViewFindDialog.cpp \
    widgets/GridViewWidget.cpp \
    widgets/SelectionDialog.cpp \
    widgets/SelectionListDataProviderProxy.cpp \
    widgets/TimelineEditorConfigWidget.cpp \
    widgets/WaitingSpinnerWidget.cpp \
    widgets/CorpusItemPreview.cpp \
    widgets/SelectReorderItemsWidget.cpp \
    observers/CorpusObserver.cpp \
    model/DelayedExecutionTimer.cpp \
    model/CheckableProxyModel.cpp \
    model/diff/DiffSESForIntervalsTableModel.cpp \
    model/query/QueryFilterSequenceTableModel.cpp \
    model/query/QueryDefinitionTreeModel.cpp \
    widgets/MediaPlayerControls.cpp \
    widgets/MediaPlayerVideoWidget.cpp \
    widgets/MiniTranscriptionWidget.cpp \
    model/annotation/AnnotationTierModel.cpp \
    model/annotation/AnnotationTableModel.cpp \
    model/NameValueListTableModel.cpp \
    nodeeditor/Connection.cpp \
    nodeeditor/ConnectionBlurEffect.cpp \
    nodeeditor/ConnectionGeometry.cpp \
    nodeeditor/ConnectionGraphicsObject.cpp \
    nodeeditor/ConnectionPainter.cpp \
    nodeeditor/ConnectionState.cpp \
    nodeeditor/ConnectionStyle.cpp \
    nodeeditor/DataModelRegistry.cpp \
    nodeeditor/FlowItemInterface.cpp \
    nodeeditor/FlowScene.cpp \
    nodeeditor/FlowView.cpp \
    nodeeditor/FlowViewStyle.cpp \
    nodeeditor/Node.cpp \
    nodeeditor/NodeConnectionInteraction.cpp \
    nodeeditor/NodeGeometry.cpp \
    nodeeditor/NodeGraphicsObject.cpp \
    nodeeditor/NodePainter.cpp \
    nodeeditor/NodeState.cpp \
    nodeeditor/NodeStyle.cpp \
    nodeeditor/Properties.cpp \
    nodeeditor/StyleCollection.cpp \
    model/corpus/CorpusTableModel.cpp \
    FlowLayout.cpp \
    widgets/ImportDataPreviewWidget.cpp

RESOURCES += \
    nodeeditor/resources/NodeEditorResources.qrc
