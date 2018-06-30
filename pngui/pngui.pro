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
include(../dependencies/qtilities/src/Qtilities.pri)
include(../dependencies/qtilities/src/Dependencies.pri)
INCLUDEPATH += dependencies/qtilities/include

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
    model/visualiser/AnnotationGridPointModel.h \
    model/visualiser/AnnotationGridModel.h \
    model/visualiser/ProsogramTonalSegmentModel.h \
    model/visualiser/ProsogramModel.h \
    model/visualiser/MovingAverageModel.h \
    layer/MovingAverageLayer.h \
    layer/AnnotationGridLayer.h \
    layer/PraalineLayersInitialiser.h \
    layer/ProsogramLayer.h \
    layer/AnnotationDiffLayer.h \
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
    widgets/ImportDataPreviewWidget.h \
    delegate/IntervalSequenceDelegate.h \
    delegate/IntervalSequenceEditor.h \
    model/annotation/AnnotationMultiTierTableModel.h \
    model/annotation/AnnotationElementTableModel.h \
    model/annotation/TimelineTableModelBase.h \
    model/annotation/AnnotationGroupingTierTableModel.h \
    widgets/AnnotationMultiTierEditorWidget.h \
    widgets/TimelineEditorWidgetBase.h \
    widgets/GroupingAnnotationEditorWidget.h \
    grid/QAbstractFilter.h \
    grid/QAbstractFilterModel.h \
    grid/QAbstractFilterProxyModel.h \
    grid/QAbstractModelItemDecoration.h \
    grid/QAdvancedHeaderView.h \
    grid/QAdvancedTableView.h \
    grid/QAdvancedTableView_p.h \
    grid/QAIV.h \
    grid/QAIVlib_global.h \
    grid/QAutoFilter.h \
    grid/QAutoFilter_p.h \
    grid/QCheckStateProxyModel.h \
    grid/QClickableLabel.h \
    grid/QConditionalDecoration.h \
    grid/QConditionalDecorationDialog.h \
    grid/QConditionalDecorationDialog_p.h \
    grid/QConditionalDecorationProxyModel.h \
    grid/QFilterEditorPopupWidget.h \
    grid/QFilterEditorWidget.h \
    grid/QFilterGroup.h \
    grid/QFilterModel.h \
    grid/QFilterModelProxy.h \
    grid/QFilterView.h \
    grid/QFilterViewConnector.h \
    grid/QFilterViewItemDelegate.h \
    grid/QFixedRowsHeaderView.h \
    grid/QFixedRowsTableView.h \
    grid/QGroupingProxyModel.h \
    grid/QGroupingProxyModel_p.h \
    grid/QHeaderViewConnector.h \
    grid/QMimeDataUtil.h \
    grid/QRangeFilter.h \
    grid/QRangeFilter_p.h \
    grid/QRegExpFilter.h \
    grid/QRegExpFilter_p.h \
    grid/QSelectionListFilter.h \
    grid/QSelectionListFilter_p.h \
    grid/QSharedItemSelectionModel.h \
    grid/QSingleColumnProxyModel.h \
    grid/QSpecialFilter.h \
    grid/QSpecialFilter_p.h \
    grid/QTextFilter.h \
    grid/QTextFilter_p.h \
    grid/QUniqueValuesProxyModel.h \
    grid/QValueFilter.h \
    grid/QValueFilter_p.h \
    grid/SpinboxItemDelegate.h \
    layer/AnnotationTimeInstantLayer.h \
    layer/AnnotationTimeValueLayer.h \
    layer/AnnotationRegionLayer.h \
    model/visualiser/AnnotationTimeValueModel.h \
    layer/RegionAverageLayer.h \
    model/visualiser/RegionAverageModel.h \
    widgets/StatusMessagesWidget.h \
    pngui/PraalineUserInterfaceOptions.h.h \
    model/annotation/PhonetisationTableModel.h \
    model/annotation/TranscriptionTableModel.h \
    layer/ProZedLayer.h

SOURCES += \
    model/visualiser/AnnotationGridModel.cpp \
    model/visualiser/ProsogramModel.cpp \
    model/visualiser/MovingAverageModel.cpp \
    layer/PraalineLayersInitialiser.cpp \
    layer/AnnotationGridLayer.cpp \
    layer/ProsogramLayer.cpp \
    layer/AnnotationDiffLayer.cpp \
    layer/MovingAverageLayer.cpp \
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
    widgets/ImportDataPreviewWidget.cpp \
    delegate/IntervalSequenceDelegate.cpp \
    delegate/IntervalSequenceEditor.cpp \
    model/annotation/AnnotationMultiTierTableModel.cpp \
    model/annotation/AnnotationElementTableModel.cpp \
    model/annotation/TimelineTableModelBase.cpp \
    model/annotation/AnnotationGroupingTierTableModel.cpp \
    widgets/AnnotationMultiTierEditorWidget.cpp \
    widgets/TimelineEditorWidgetBase.cpp \
    widgets/GroupingAnnotationEditorWidget.cpp \
    grid/QAbstractFilter.cpp \
    grid/QAbstractFilterModel.cpp \
    grid/QAbstractFilterProxyModel.cpp \
    grid/QAbstractModelItemDecoration.cpp \
    grid/QAdvancedHeaderView.cpp \
    grid/QAdvancedTableView.cpp \
    grid/QAIV.cpp \
    grid/QAutoFilter.cpp \
    grid/QCheckStateProxyModel.cpp \
    grid/QClickableLabel.cpp \
    grid/QConditionalDecoration.cpp \
    grid/QConditionalDecorationDialog.cpp \
    grid/QConditionalDecorationProxyModel.cpp \
    grid/QFilterEditorPopupWidget.cpp \
    grid/QFilterEditorWidget.cpp \
    grid/QFilterGroup.cpp \
    grid/QFilterModel.cpp \
    grid/QFilterModelProxy.cpp \
    grid/QFilterView.cpp \
    grid/QFilterViewConnector.cpp \
    grid/QFilterViewItemDelegate.cpp \
    grid/QFixedRowsHeaderView.cpp \
    grid/QFixedRowsTableView.cpp \
    grid/QGroupingProxyModel.cpp \
    grid/QHeaderViewConnector.cpp \
    grid/QMimeDataUtil.cpp \
    grid/QRangeFilter.cpp \
    grid/QRegExpFilter.cpp \
    grid/QSelectionListFilter.cpp \
    grid/QSharedItemSelectionModel.cpp \
    grid/QSingleColumnProxyModel.cpp \
    grid/QSpecialFilter.cpp \
    grid/QTextFilter.cpp \
    grid/QUniqueValuesProxyModel.cpp \
    grid/QValueFilter.cpp \
    grid/SpinboxItemDelegate.cpp \
    layer/AnnotationTimeInstantLayer.cpp \
    layer/AnnotationTimeValueLayer.cpp \
    layer/AnnotationRegionLayer.cpp \
    model/visualiser/AnnotationTimeValueModel.cpp \
    layer/RegionAverageLayer.cpp \
    model/visualiser/RegionAverageModel.cpp \
    widgets/StatusMessagesWidget.cpp \
    model/annotation/PhonetisationTableModel.cpp \
    model/annotation/TranscriptionTableModel.cpp \
    layer/ProZedLayer.cpp

RESOURCES += \
    nodeeditor/resources/NodeEditorResources.qrc
