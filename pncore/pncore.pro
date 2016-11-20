# Praaline
# Core Library
# (c) George Christodoulides 2012-2015

! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

TEMPLATE = lib

CONFIG(debug, debug|release) {
    TARGET = pncored
} else {
    TARGET = pncore
}

CONFIG += qt thread warn_on stl rtti exceptions c++11

QT += xml sql
QT -= gui

DEFINES += PRAALINE_CORE_LIBRARY

INCLUDEPATH += . ..
DEPENDPATH += . ..

HEADERS += \
    base/BaseTypes.h \
    base/RealTime.h \
    pncore_global.h \
    interfaces/praat/PraatPointTierFile.h \
    interfaces/anvil/AnvilMetadataTranscript.h \
    base/RealValueList.h \
    statistics/StatisticalSummary.h \
    statistics/StatisticalMeasureDefinition.h \
    statistics/HistogramCalculator.h \
    QSqlMigrator/Structure/Column.h \
    QSqlMigrator/Structure/Index.h \
    QSqlMigrator/Structure/LocalScheme.h \
    QSqlMigrator/Structure/Table.h \
    QSqlMigrator/Scheme/Database.h \
    QSqlMigrator/Scheme/DbColumn.h \
    QSqlMigrator/Scheme/DbTable.h \
    QSqlMigrator/Helper/ColumnService.h \
    QSqlMigrator/Helper/HelperRepository.h \
    QSqlMigrator/Helper/QuoteService.h \
    QSqlMigrator/Helper/SqlStructureService.h \
    QSqlMigrator/Helper/TypeMapperService.h \
    QSqlMigrator/Commands/AddColumn.h \
    QSqlMigrator/Commands/AlterColumnType.h \
    QSqlMigrator/Commands/BaseCommand.h \
    QSqlMigrator/Commands/CreateIndex.h \
    QSqlMigrator/Commands/CreateTable.h \
    QSqlMigrator/Commands/CustomCommandBase.h \
    QSqlMigrator/Commands/DropColumn.h \
    QSqlMigrator/Commands/DropIndex.h \
    QSqlMigrator/Commands/DropTable.h \
    QSqlMigrator/Commands/RenameColumn.h \
    QSqlMigrator/Commands/RenameTable.h \
    QSqlMigrator/Migrations/Migration.h \
    QSqlMigrator/Migrations/MigrationRepository.h \
    QSqlMigrator/Migrations/RegisterMigration.h \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlAddColumnService.h \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlAlterColumnTypeService.h \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlCreateIndexService.h \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlCreateTableService.h \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlDropColumnService.h \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlDropIndexService.h \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlDropTableService.h \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlRenameColumnService.h \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlRenameTableService.h \
    QSqlMigrator/BaseSqlMigrator/Helper/BaseSqlColumnService.h \
    QSqlMigrator/BaseSqlMigrator/Helper/BaseSqlQuoteService.h \
    QSqlMigrator/BaseSqlMigrator/Helper/BaseSqlTypeMapperService.h \
    QSqlMigrator/BaseSqlMigrator/MigrationTracker/BaseMigrationTableService.h \
    QSqlMigrator/CommandExecution/BaseCommandExecutionService.h \
    QSqlMigrator/CommandExecution/CommandExecutionContext.h \
    QSqlMigrator/CommandExecution/CommandExecutionService.h \
    QSqlMigrator/CommandExecution/CommandExecutionServiceRepository.h \
    QSqlMigrator/CommandExecution/CustomCommandService.h \
    QSqlMigrator/CommandExecution/LocalSchemeBaseCommandExecutionService.h \
    QSqlMigrator/CommandExecution/LocalSchemeCommandExecutionContext.h \
    QSqlMigrator/CommandExecution/LocalSchemeCommandExecutionService.h \
    QSqlMigrator/CommandExecution/LocalSchemeCommandExecutionServiceRepository.h \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeAddColumnService.h \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeAlterColumnTypeService.h \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeCreateIndexService.h \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeCreateTableService.h \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeDropColumnService.h \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeDropIndexService.h \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeDropTableService.h \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeRenameColumnService.h \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeRenameTableService.h \
    QSqlMigrator/LocalSchemeMigrator/LocalSchemeComparisonContext.h \
    QSqlMigrator/LocalSchemeMigrator/LocalSchemeComparisonService.h \
    QSqlMigrator/LocalSchemeMigrator/LocalSchemeMigrator.h \
    QSqlMigrator/MigrationExecution/LocalSchemeMigrationExecutionContext.h \
    QSqlMigrator/MigrationExecution/LocalSchemeMigrationExecutionService.h \
    QSqlMigrator/MigrationExecution/MigrationExecutionConfig.h \
    QSqlMigrator/MigrationExecution/MigrationExecutionContext.h \
    QSqlMigrator/MigrationExecution/MigrationExecutionService.h \
    QSqlMigrator/MigrationTracker/MigrationTrackerService.h \
    QSqlMigrator/QSqlMigrator/QSqlMigratorConfig.h \
    QSqlMigrator/QSqlMigrator/QSqlMigratorService.h \
    QSqlMigrator/QSqlMigrator.h \
    QSqlMigrator/QSqlMigratorConfig.h \
    QSqlMigrator/Databases/SqliteMigrator/SqliteMigrator.h \
    QSqlMigrator/Databases/SqliteMigrator/DatabaseLock.h \
    QSqlMigrator/Databases/SqliteMigrator/CommandExecution/SqliteAlterColumnService.h \
    QSqlMigrator/Databases/SqliteMigrator/CommandExecution/SqliteAlterColumnTypeService.h \
    QSqlMigrator/Databases/SqliteMigrator/CommandExecution/SqliteDropColumnService.h \
    QSqlMigrator/Databases/SqliteMigrator/CommandExecution/SqliteRenameColumnService.h \
    QSqlMigrator/Databases/SqliteMigrator/Helper/SqliteSqlStructureService.h \
    QSqlMigrator/Databases/SqliteMigrator/MigrationTracker/SqliteMigrationTableService.h \
    QSqlMigrator/Databases/MysqlMigrator/MysqlMigrator.h \
    QSqlMigrator/Databases/MysqlMigrator/Helper/MysqlColumnService.h \
    QSqlMigrator/Databases/MysqlMigrator/Helper/MysqlStructureService.h \
    QSqlMigrator/Databases/MysqlMigrator/Helper/MysqlTypeMapperService.h \
    QSqlMigrator/Databases/MysqlMigrator/CommandExecution/MysqlAlterColumnTypeService.h \
    QSqlMigrator/Databases/MysqlMigrator/CommandExecution/MysqlRenameColumnService.h \
    QSqlMigrator/Databases/FirebirdMigrator/FirebirdMigrator.h \
    QSqlMigrator/Databases/FirebirdMigrator/Helper/FirebirdSqlStructureService.h \
    QSqlMigrator/Databases/PostgresqlMigrator/PostgresqlMigrator.h \
    QSqlMigrator/Databases/PostgresqlMigrator/Helper/PostgresqlColumnService.h \
    QSqlMigrator/Databases/PostgresqlMigrator/Helper/PostgresqlQuoteService.h \
    QSqlMigrator/Databases/PostgresqlMigrator/Helper/PostgresqlStructureService.h \
    QSqlMigrator/Databases/PostgresqlMigrator/Helper/PostgresqlTypeMapperService.h \
    base/DataType.h \
    QSqlMigrator/Structure/SqlType.h \
    structure/StructureBase.h \
    structure/AnnotationStructureAttribute.h \
    structure/AnnotationStructureLevel.h \
    structure/AnnotationStructure.h \
    structure/MetadataStructureAttribute.h \
    structure/MetadataStructureSection.h \
    structure/MetadataStructure.h \
    serialisers/AbstractAnnotationDatastore.h \
    serialisers/AbstractMetadataDatastore.h \
    serialisers/DatastoreFactory.h \
    corpus/Corpus.h \
    corpus/CorpusCommunication.h \
    corpus/CorpusParticipation.h \
    corpus/CorpusAnnotation.h \
    corpus/CorpusObject.h \
    corpus/CorpusRecording.h \
    corpus/CorpusSpeaker.h \
    corpus/CorpusBookmark.h \
    annotation/AnnotationDataTable.h \
    annotation/AnnotationElement.h \
    annotation/AnnotationTier.h \
    annotation/AnnotationTierGroup.h \
    annotation/Interval.h \
    annotation/IntervalTier.h \
    annotation/Point.h \
    annotation/PointTier.h \
    annotation/RelationTier.h \
    annotation/SpeakerTimeline.h \
    annotation/TreeTier.h \
    interfaces/praat/PraatTextFile.h \
    interfaces/praat/PraatTextGrid.h \
    interfaces/exmaralda/ExmaraldaBasicTranscription.h \
    interfaces/exmaralda/ExmaraldaTranscriptionBridge.h \
    interfaces/subtitles/SubtitlesFile.h \
    interfaces/transcriber/TranscriberAnnotationGraph.h \
    interfaces/InterfaceTextFile.h \
    serialisers/DatastoreInfo.h \
    serialisers/CorpusDefinition.h \
    serialisers/xml/XMLAnnotationDatastore.h \
    serialisers/xml/XMLMetadataDatastore.h \
    serialisers/xml/XMLSerialiserAnnotation.h \
    serialisers/xml/XMLSerialiserAnnotationStructure.h \
    serialisers/xml/XMLSerialiserBase.h \
    serialisers/xml/XMLSerialiserCorpus.h \
    serialisers/xml/XMLSerialiserCorpusBookmark.h \
    serialisers/xml/XMLSerialiserMetadataStructure.h \
    query/QueryDefinition.h \
    query/QueryFilterGroup.h \
    query/QueryFilterSequence.h \
    query/QueryOccurrence.h \
    serialisers/sql/SQLAnnotationDatastore.h \
    serialisers/sql/SQLMetadataDatastore.h \
    serialisers/sql/SQLQueryEngineAnnotation.h \
    serialisers/sql/SQLSchemaProxyAnnotation.h \
    serialisers/sql/SQLSchemaProxyBase.h \
    serialisers/sql/SQLSchemaProxyMetadata.h \
    serialisers/sql/SQLSerialiserAnnotation.h \
    serialisers/sql/SQLSerialiserAnnotationStructure.h \
    serialisers/sql/SQLSerialiserBase.h \
    serialisers/sql/SQLSerialiserCorpus.h \
    query/Dataset.h \
    query/Measures.h \
    serialisers/mocadb/MocaDBSerialiserAnnotationStructure.h \
    serialisers/mocadb/MocaDBSerialiserMetadata.h \
    serialisers/mocadb/MocaDBSerialiserMetadataStructure.h \
    serialisers/sql/SQLSerialiserMetadata.h \
    serialisers/sql/SQLSerialiserMetadataStructure.h

SOURCES += \
    base/RealTime.cpp \
    interfaces/praat/PraatPointTierFile.cpp \
    interfaces/anvil/AnvilMetadataTranscript.cpp \
    base/RealValueList.cpp \
    statistics/StatisticalSummary.cpp \
    statistics/StatisticalMeasureDefinition.cpp \
    statistics/HistogramCalculator.cpp \
    QSqlMigrator/Structure/Column.cpp \
    QSqlMigrator/Structure/Index.cpp \
    QSqlMigrator/Structure/LocalScheme.cpp \
    QSqlMigrator/Structure/Table.cpp \
    QSqlMigrator/Scheme/Database.cpp \
    QSqlMigrator/Scheme/DbColumn.cpp \
    QSqlMigrator/Scheme/DbTable.cpp \
    QSqlMigrator/Helper/HelperRepository.cpp \
    QSqlMigrator/Commands/AddColumn.cpp \
    QSqlMigrator/Commands/AlterColumnType.cpp \
    QSqlMigrator/Commands/BaseCommand.cpp \
    QSqlMigrator/Commands/CreateIndex.cpp \
    QSqlMigrator/Commands/CreateTable.cpp \
    QSqlMigrator/Commands/CustomCommandBase.cpp \
    QSqlMigrator/Commands/DropColumn.cpp \
    QSqlMigrator/Commands/DropIndex.cpp \
    QSqlMigrator/Commands/DropTable.cpp \
    QSqlMigrator/Commands/RenameColumn.cpp \
    QSqlMigrator/Commands/RenameTable.cpp \
    QSqlMigrator/Migrations/Migration.cpp \
    QSqlMigrator/Migrations/MigrationRepository.cpp \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlAddColumnService.cpp \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlAlterColumnTypeService.cpp \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlCreateIndexService.cpp \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlCreateTableService.cpp \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlDropColumnService.cpp \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlDropIndexService.cpp \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlDropTableService.cpp \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlRenameColumnService.cpp \
    QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlRenameTableService.cpp \
    QSqlMigrator/BaseSqlMigrator/Helper/BaseSqlColumnService.cpp \
    QSqlMigrator/BaseSqlMigrator/Helper/BaseSqlQuoteService.cpp \
    QSqlMigrator/BaseSqlMigrator/Helper/BaseSqlTypeMapperService.cpp \
    QSqlMigrator/BaseSqlMigrator/MigrationTracker/BaseMigrationTableService.cpp \
    QSqlMigrator/CommandExecution/BaseCommandExecutionService.cpp \
    QSqlMigrator/CommandExecution/CommandExecutionContext.cpp \
    QSqlMigrator/CommandExecution/CommandExecutionService.cpp \
    QSqlMigrator/CommandExecution/CommandExecutionServiceRepository.cpp \
    QSqlMigrator/CommandExecution/CustomCommandService.cpp \
    QSqlMigrator/CommandExecution/LocalSchemeBaseCommandExecutionService.cpp \
    QSqlMigrator/CommandExecution/LocalSchemeCommandExecutionContext.cpp \
    QSqlMigrator/CommandExecution/LocalSchemeCommandExecutionService.cpp \
    QSqlMigrator/CommandExecution/LocalSchemeCommandExecutionServiceRepository.cpp \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeAddColumnService.cpp \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeAlterColumnTypeService.cpp \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeCreateIndexService.cpp \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeCreateTableService.cpp \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeDropColumnService.cpp \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeDropIndexService.cpp \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeDropTableService.cpp \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeRenameColumnService.cpp \
    QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeRenameTableService.cpp \
    QSqlMigrator/LocalSchemeMigrator/LocalSchemeComparisonContext.cpp \
    QSqlMigrator/LocalSchemeMigrator/LocalSchemeComparisonService.cpp \
    QSqlMigrator/LocalSchemeMigrator/LocalSchemeMigrator.cpp \
    QSqlMigrator/MigrationExecution/LocalSchemeMigrationExecutionContext.cpp \
    QSqlMigrator/MigrationExecution/LocalSchemeMigrationExecutionService.cpp \
    QSqlMigrator/MigrationExecution/MigrationExecutionConfig.cpp \
    QSqlMigrator/MigrationExecution/MigrationExecutionContext.cpp \
    QSqlMigrator/MigrationExecution/MigrationExecutionService.cpp \
    QSqlMigrator/QSqlMigrator/QSqlMigratorConfig.cpp \
    QSqlMigrator/QSqlMigrator/QSqlMigratorService.cpp \
    QSqlMigrator/Databases/SqliteMigrator/SqliteMigrator.cpp \
    QSqlMigrator/Databases/SqliteMigrator/DatabaseLock.cpp \
    QSqlMigrator/Databases/SqliteMigrator/CommandExecution/SqliteAlterColumnService.cpp \
    QSqlMigrator/Databases/SqliteMigrator/CommandExecution/SqliteAlterColumnTypeService.cpp \
    QSqlMigrator/Databases/SqliteMigrator/CommandExecution/SqliteDropColumnService.cpp \
    QSqlMigrator/Databases/SqliteMigrator/CommandExecution/SqliteRenameColumnService.cpp \
    QSqlMigrator/Databases/SqliteMigrator/Helper/SqliteSqlStructureService.cpp \
    QSqlMigrator/Databases/SqliteMigrator/MigrationTracker/SqliteMigrationTableService.cpp \
    QSqlMigrator/Databases/MysqlMigrator/MysqlMigrator.cpp \
    QSqlMigrator/Databases/MysqlMigrator/Helper/MysqlColumnService.cpp \
    QSqlMigrator/Databases/MysqlMigrator/Helper/MysqlStructureService.cpp \
    QSqlMigrator/Databases/MysqlMigrator/Helper/MysqlTypeMapperService.cpp \
    QSqlMigrator/Databases/MysqlMigrator/CommandExecution/MysqlAlterColumnTypeService.cpp \
    QSqlMigrator/Databases/MysqlMigrator/CommandExecution/MysqlRenameColumnService.cpp \
    QSqlMigrator/Databases/FirebirdMigrator/FirebirdMigrator.cpp \
    QSqlMigrator/Databases/FirebirdMigrator/Helper/FirebirdSqlStructureService.cpp \
    QSqlMigrator/Databases/PostgresqlMigrator/PostgresqlMigrator.cpp \
    QSqlMigrator/Databases/PostgresqlMigrator/Helper/PostgresqlColumnService.cpp \
    QSqlMigrator/Databases/PostgresqlMigrator/Helper/PostgresqlQuoteService.cpp \
    QSqlMigrator/Databases/PostgresqlMigrator/Helper/PostgresqlStructureService.cpp \
    QSqlMigrator/Databases/PostgresqlMigrator/Helper/PostgresqlTypeMapperService.cpp \
    base/DataType.cpp \
    QSqlMigrator/Structure/SqlType.cpp \
    structure/StructureBase.cpp \
    structure/AnnotationStructureAttribute.cpp \
    structure/AnnotationStructureLevel.cpp \
    structure/AnnotationStructure.cpp \
    structure/MetadataStructureAttribute.cpp \
    structure/MetadataStructureSection.cpp \
    structure/MetadataStructure.cpp \
    serialisers/DatastoreFactory.cpp \
    corpus/Corpus.cpp \
    corpus/CorpusCommunication.cpp \
    corpus/CorpusParticipation.cpp \
    corpus/CorpusObject.cpp \
    corpus/CorpusAnnotation.cpp \
    corpus/CorpusRecording.cpp \
    corpus/CorpusSpeaker.cpp \
    corpus/CorpusBookmark.cpp \
    annotation/AnnotationDataTable.cpp \
    annotation/AnnotationElement.cpp \
    annotation/AnnotationTier.cpp \
    annotation/AnnotationTierGroup.cpp \
    annotation/Interval.cpp \
    annotation/IntervalTier.cpp \
    annotation/Point.cpp \
    annotation/PointTier.cpp \
    annotation/RelationTier.cpp \
    annotation/SpeakerTimeline.cpp \
    annotation/TreeTier.cpp \
    interfaces/praat/PraatTextFile.cpp \
    interfaces/praat/PraatTextGrid.cpp \
    interfaces/exmaralda/ExmaraldaBasicTranscription.cpp \
    interfaces/exmaralda/ExmaraldaTranscriptionBridge.cpp \
    interfaces/subtitles/SubtitlesFile.cpp \
    interfaces/transcriber/TranscriberAnnotationGraph.cpp \
    interfaces/InterfaceTextFile.cpp \
    serialisers/CorpusDefinition.cpp \
    serialisers/xml/XMLAnnotationDatastore.cpp \
    serialisers/xml/XMLMetadataDatastore.cpp \
    serialisers/xml/XMLSerialiserAnnotation.cpp \
    serialisers/xml/XMLSerialiserAnnotationStructure.cpp \
    serialisers/xml/XMLSerialiserBase.cpp \
    serialisers/xml/XMLSerialiserCorpus.cpp \
    serialisers/xml/XMLSerialiserCorpusBookmark.cpp \
    serialisers/xml/XMLSerialiserMetadataStructure.cpp \
    query/QueryDefinition.cpp \
    query/QueryFilterGroup.cpp \
    query/QueryFilterSequence.cpp \
    query/QueryOccurrence.cpp \
    serialisers/sql/SQLAnnotationDatastore.cpp \
    serialisers/sql/SQLMetadataDatastore.cpp \
    serialisers/sql/SQLQueryEngineAnnotation.cpp \
    serialisers/sql/SQLSchemaProxyBase.cpp \
    serialisers/sql/SQLSchemaProxyMetadata.cpp \
    serialisers/sql/SQLSerialiserAnnotation.cpp \
    serialisers/sql/SQLSerialiserAnnotationStructure.cpp \
    serialisers/sql/SQLSerialiserBase.cpp \
    serialisers/sql/SQLSerialiserCorpus.cpp \
    serialisers/sql/SQLSerialiserMetadataStructure.cpp \
    serialisers/mocadb/MocaDBSerialiserMetadataStructure.cpp \
    serialisers/mocadb/MocaDBSerialiserMetadata.cpp \
    corpus/CorpusObjectInfo.cpp \
    serialisers/sql/SQLSerialiserMetadata.cpp \
    serialisers/mocadb/MocaDBSerialiserAnnotationStructure.cpp \
    query/Dataset.cpp \
    query/Measures.cpp

DISTFILES += \
    pncore.qmodel
