#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include <QSqlQuery>
#include <QSqlRecord>

#include "plugindismo.h"
#include "dismo/dismoannotator.h"
#include "dismo/dismotrainer.h"
#include "serialisers/dismoserialisersql.h"
#include "pncore/corpus/Corpus.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Core;
using namespace Praaline::Plugins;

struct Praaline::Plugins::DisMo::PluginDisMoPrivateData {
    PluginDisMoPrivateData() :
        createDisMoAnnotationLevels(false), alreadyTokenised(false), tokenisedOnlyToMinimal(false),
        levelToAnnotate("segment"), levelTokMin("tok_min"), levelTokMWU("tok_mwu"),
        levelPhones("phone"), attributePrefix("")
    {
        attributeNames["pos_min"] = "pos_min";
        attributeNames["pos_ext_min"] = "pos_ext_min";
        attributeNames["lemma_min"] = "lemma_min";
        attributeNames["disfluency"] = "disfluency";
        attributeNames["pos_mwu"] = "pos_mwu";
        attributeNames["pos_ext_mwu"] = "pos_ext_mwu";
        attributeNames["lemma_mwu"] = "lemma_mwu";
        attributeNames["discourse"] = "discourse";
    }

    bool createDisMoAnnotationLevels;
    bool alreadyTokenised;
    bool tokenisedOnlyToMinimal;
    QString levelToAnnotate;
    QString levelTokMin;
    QString levelTokMWU;
    QString levelPhones;
    QString attributePrefix;
    QHash<QString, QString> attributeNames;
};

Praaline::Plugins::DisMo::PluginDisMo::PluginDisMo(QObject* parent) : QObject(parent)
{
    d = new PluginDisMoPrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::DisMo::PluginDisMo::~PluginDisMo()
{
}

bool Praaline::Plugins::DisMo::PluginDisMo::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::DisMo::PluginDisMo::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::DisMo::PluginDisMo::finalize() {
    delete d;
}

QString Praaline::Plugins::DisMo::PluginDisMo::pluginName() const {
    return "DisMo Annotator";
}

QtilitiesCategory Praaline::Plugins::DisMo::PluginDisMo::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::DisMo::PluginDisMo::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::DisMo::PluginDisMo::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::DisMo::PluginDisMo::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::DisMo::PluginDisMo::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::DisMo::PluginDisMo::pluginDescription() const {
    return tr("DisMo is a part-of-speech tagger, multi-word unit and disfluency detector for spoken language transcriptions.");
}

QString Praaline::Plugins::DisMo::PluginDisMo::pluginCopyright() const {
    return QString(tr("Copyright") + " 2012-2014, George Christodoulides");
}

QString Praaline::Plugins::DisMo::PluginDisMo::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::DisMo::PluginDisMo::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    parameters << PluginParameter("createDisMoAnnotationLevels", "Create DisMo annotation levels?", QVariant::Bool, d->createDisMoAnnotationLevels);
    parameters << PluginParameter("alreadyTokenised", "Already tokenised?", QVariant::Bool, d->alreadyTokenised);
    parameters << PluginParameter("tokenisedOnlyToMinimal", "Tokenised only to minimal tokens?", QVariant::Bool, d->tokenisedOnlyToMinimal);
    parameters << PluginParameter("levelToAnnotate", "Level to annotate", QVariant::String, d->levelToAnnotate);
    parameters << PluginParameter("levelTokMin", "Level for tokens (minimal)", QVariant::String, d->levelTokMin);
    parameters << PluginParameter("levelTokMWU", "Level for tokens (multi-word units)", QVariant::String, d->levelTokMWU);
    parameters << PluginParameter("levelPhones", "Level for phones (optional)", QVariant::String, d->levelPhones);
    parameters << PluginParameter("attributePrefix", "Prefix attribute names", QVariant::String, d->attributePrefix);
    return parameters;
}

void Praaline::Plugins::DisMo::PluginDisMo::setParameters(QHash<QString, QVariant> parameters)
{
    if (parameters.contains("createDisMoAnnotationLevels")) d->createDisMoAnnotationLevels = parameters.value("createDisMoAnnotationLevels").toBool();
    if (parameters.contains("alreadyTokenised")) d->alreadyTokenised = parameters.value("alreadyTokenised").toBool();
    if (parameters.contains("tokenisedOnlyToMinimal")) d->tokenisedOnlyToMinimal = parameters.value("tokenisedOnlyToMinimal").toBool();
    if (parameters.contains("levelToAnnotate")) d->levelToAnnotate = parameters.value("levelToAnnotate").toString();
    if (parameters.contains("levelTokMin")) d->levelTokMin = parameters.value("levelTokMin").toString();
    if (parameters.contains("levelTokMWU")) d->levelTokMWU = parameters.value("levelTokMWU").toString();
    if (parameters.contains("levelPhones")) d->levelPhones = parameters.value("levelPhones").toString();
    if (parameters.contains("attributePrefix")) d->attributePrefix = parameters.value("attributePrefix").toString();
}

void createAttribute(Corpus *corpus, AnnotationStructureLevel *level, const QString &prefix,
                     const QString &ID, const QString &name = QString(), const QString &description = QString(),
                     const DataType &datatype = DataType(DataType::VarChar, 255), int order = 0,
                     bool indexed = false, const QString &nameValueList = QString())
{
    if (level->hasAttribute(ID)) return;
    AnnotationStructureAttribute *attr = new AnnotationStructureAttribute(prefix + ID, name, description, datatype,
                                                                          order, indexed, nameValueList);
    if (corpus->datastoreAnnotations()->createAnnotationAttribute(level->ID(), attr))
        level->addAttribute(attr);
}

void Praaline::Plugins::DisMo::PluginDisMo::createDisMoAnnotationStructure(Corpus *corpus)
{
    if (!corpus) return;
    d->attributePrefix = d->attributePrefix.trimmed();
    // If needed, create tok_min level
    AnnotationStructureLevel *level_tok_min = corpus->annotationStructure()->level(d->levelTokMin);
    if (!level_tok_min) {
        level_tok_min = new AnnotationStructureLevel(d->levelTokMin, AnnotationStructureLevel::IndependentIntervalsLevel, "Tokens (minimal)", "");
        if (!corpus->datastoreAnnotations()->createAnnotationLevel(level_tok_min)) return;
        corpus->annotationStructure()->addLevel(level_tok_min);
    }
    // Create tok_min attributes where necessary
    createAttribute(corpus, level_tok_min, d->attributePrefix, "pos_min", "Part-of-Speech (min)", "", DataType(DataType::VarChar, 32));
    createAttribute(corpus, level_tok_min, d->attributePrefix, "disfluency", "Disfluency", "", DataType(DataType::VarChar, 32));
    createAttribute(corpus, level_tok_min, d->attributePrefix, "lemma_min", "Lemma (min)", "", DataType(DataType::VarChar, 256));
    createAttribute(corpus, level_tok_min, d->attributePrefix, "pos_ext_min", "POS extended (min)", "", DataType(DataType::VarChar, 32));
    createAttribute(corpus, level_tok_min, d->attributePrefix, "dismo_confidence", "DisMo confidence", "", DataType::Double);
    createAttribute(corpus, level_tok_min, d->attributePrefix, "dismo_method", "DisMo method", "", DataType(DataType::VarChar, 64));
    // If needed, create tok_mwu level
    AnnotationStructureLevel *level_tok_mwu = corpus->annotationStructure()->level(d->levelTokMWU);
    if (!level_tok_mwu) {
        level_tok_mwu = new AnnotationStructureLevel(d->levelTokMWU, AnnotationStructureLevel::GroupingLevel, "Tokens (MWU)", "");
        if (!corpus->datastoreAnnotations()->createAnnotationLevel(level_tok_mwu)) return;
        corpus->annotationStructure()->addLevel(level_tok_mwu);
    }
    // Create tok_mwu attributes where necessary
    createAttribute(corpus, level_tok_mwu, d->attributePrefix, "pos_mwu", "Part-of-Speech (MWU)", "", DataType(DataType::VarChar, 32));
    createAttribute(corpus, level_tok_mwu, d->attributePrefix, "discourse", "Discourse", "", DataType(DataType::VarChar, 32));
    createAttribute(corpus, level_tok_mwu, d->attributePrefix, "lemma_mwu", "Lemma (MWU)", "", DataType(DataType::VarChar, 256));
    createAttribute(corpus, level_tok_mwu, d->attributePrefix, "pos_ext_mwu", "POS extended (MWU)", "", DataType(DataType::VarChar, 32));
    createAttribute(corpus, level_tok_mwu, d->attributePrefix, "dismo_confidence", "DisMo confidence", "", DataType::Double);
    createAttribute(corpus, level_tok_mwu, d->attributePrefix, "dismo_method", "DisMo method", "", DataType(DataType::VarChar, 64));
    // Save corpus
    corpus->save();
}

void trainExperiments()
{

//    DisMoAnnotator::DismoTrainer *T = new DisMoAnnotator::DismoTrainer();
//    QString path = "D:/DROPBOX/DISMO_ARTICLE/crf/French_vPFC/";
//    T->loadTableFiles(QStringList() << path + "fullcorpus_cprompfc.txt");
//    T->train(path + "test_", DisMoAnnotator::DismoTrainer::POSMin);
//    delete T;

    QStringList cuts;
    // cuts << "50" << "60" << "70";
    // cuts << "90" << "100";
    cuts << "120" << "140";
    foreach (QString cut, cuts) {
        DisMoAnnotator::DismoTrainer *T = new DisMoAnnotator::DismoTrainer();
        QString path = "D:/DROPBOX/DISMO_ARTICLE/crf/French_vPFC/";
        T->loadTableFiles(QStringList() << path + "fullcorpus_" + cut + ".txt");
        T->train(path + "coarse_train_" + cut + "_", DisMoAnnotator::DismoTrainer::POSMin);
        delete T;
    }
}

QString evaluateCRFfile(const QString &filename, double &precision, double &precisionWithNonAmbig)
{
    int countCorrect = 0, countIncorrect = 0, countIncorrectButNonAmbig = 0;
    precision = 0.0;
    precisionWithNonAmbig = 0.0;
    QString line;
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        return QString("Error opening %1").arg(filename);
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    do {
        line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;
        QString dictionary = line.section('\t', 1, 1);
        QString correct = line.section('\t', 3, 3);
        QString response = line.section('\t', 4, 4);
        if (correct == response)
            countCorrect++;
        else {
            countIncorrect++;
            if (!dictionary.contains("+"))
                countIncorrectButNonAmbig++;
        }
    } while (!stream.atEnd());
    file.close();
    int total = countCorrect + countIncorrect;
    precision = ((double)countCorrect) / ((double)total);
    precisionWithNonAmbig = ((double)(countCorrect + countIncorrectButNonAmbig)) / ((double)total);

    // get data from the training log
    QString info;
    QString filenameLog = QString(filename).replace("eval_", "").replace(".txt", ".log");
    QFile fileLog(filenameLog);
    if ( !fileLog.open( QIODevice::ReadOnly | QIODevice::Text ) )
        return QString("Error opening %1").arg(filenameLog);
    QTextStream streamLog(&fileLog);
    streamLog.setCodec("UTF-8");
    QString numSentences, numFeatures, time;
    do {
        line = streamLog.readLine().trimmed();
        if (line.startsWith("Number of sentences:")) {
            numSentences = line.section(":", 1, 1).trimmed();
        }
        else if (line.startsWith("Number of features:")) {
            numFeatures = line.section(":", 1, 1).trimmed();
        }
        else if (line.startsWith("Done!")) {
            time = line.section("!", 1, 1).remove("s").trimmed();
        }
    } while (!streamLog.atEnd());
    fileLog.close();
    info = QString("%1\t%2\t%3\t%4\t%5\t%6").arg(filename).arg(numSentences).arg(numFeatures).arg(time).arg(precision).arg(precisionWithNonAmbig);
    return info;
}


void Praaline::Plugins::DisMo::PluginDisMo::process(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
//    addMWUindications(corpus, communications);
//    return;

    //trainExperiments();
//    double precision = 0.0, precisionWithNonAmbig = 0.0;
//    QString path = "D:/DROPBOX/DISMO_ARTICLE/crf/French_vPFC/";
//    QStringList cuts; cuts << "50" << "60" << "70" << "90" << "100" << "120" << "140";
//    QStringList cfactors; cfactors << "1" << "2" << "3";
//    foreach (QString cut, cuts) {
//        foreach (QString cfactor, cfactors) {
//            QString file = "eval_cposmin_" + cut + "_" + cfactor + ".txt";
//            printMessage(evaluateCRFfile(path + file, precision, precisionWithNonAmbig));
//        }
//    }
//    return;

    if (!corpus) {
        emit printMessage("DisMo: no corpus selected!");
        return;
    }

    DisMoAnnotator::DismoAnnotator *DISMO = new DisMoAnnotator::DismoAnnotator("fr");
    QPointer<IntervalTier> tier_tok_min;
    QPointer<IntervalTier> tier_tok_mwu;

    if (d->createDisMoAnnotationLevels) {
        createDisMoAnnotationStructure(corpus);
        emit printMessage("Created 2 annotation levels to store DisMo annotations");
    }
    int countDone = 0;
    madeProgress(0);
    printMessage("DisMo Annotator ver. 1.0 running");
    foreach(QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        printMessage(QString("Annotating %1").arg(com->ID()));
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID());
            foreach (QString speakerID, tiersAll.keys()) {
                printMessage(QString("   speaker %1").arg(speakerID));
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                if (!d->alreadyTokenised) {
                    QPointer<IntervalTier> tier_input = tiers->getIntervalTierByName(d->levelToAnnotate);
                    QPointer<IntervalTier> tier_phone = tiers->getIntervalTierByName(d->levelPhones);
                    tier_tok_min = tiers->getIntervalTierByName(d->levelTokMin);
                    tier_tok_mwu = tiers->getIntervalTierByName(d->levelTokMWU);
                    if (!tier_tok_min) tier_tok_min = new IntervalTier(d->levelTokMin, tiers->tMin(), tiers->tMax());
                    if (!tier_tok_mwu) tier_tok_mwu = new IntervalTier(d->levelTokMWU, tiers->tMin(), tiers->tMax());
                    DISMO->annotate(tier_input, tier_tok_min, tier_tok_mwu, tier_phone);
                }
                else {
                    if (d->tokenisedOnlyToMinimal) {
                        tier_tok_min = tiers->getIntervalTierByName(d->levelTokMin);
                        if (!tier_tok_min) continue;
                        tier_tok_min->fillEmptyTextLabelsWith("_");
                        tier_tok_mwu = new IntervalTier(d->levelTokMWU, tiers->tMin(), tiers->tMax());
                        DISMO->annotateTokenizedToMinimal(tier_tok_min, tier_tok_mwu);
                    }
                    else {
                        tier_tok_min = tiers->getIntervalTierByName(d->levelTokMin);
                        tier_tok_mwu = tiers->getIntervalTierByName(d->levelTokMWU);
                        if (!tier_tok_min || !tier_tok_mwu) continue;
                        tier_tok_min->fillEmptyTextLabelsWith("_");
                        tier_tok_mwu->fillEmptyTextLabelsWith("_");
                        QHash<QString, QString> attr;
                        foreach (QString attrName, d->attributeNames.keys()) {
                            attr.insert(d->attributePrefix + attrName, d->attributeNames.value(attrName));
                        }
                        DISMO->annotateTokenized(tier_tok_min, tier_tok_mwu, attr);
                    }
                }
                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_tok_mwu);
                qDebug() << QString("Annotated %1, speaker %2").arg(annot->ID()).arg(speakerID);
                delete tier_tok_min;
                delete tier_tok_mwu;
            }
            qDeleteAll(tiersAll);
            QApplication::processEvents();
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
    delete DISMO;
    madeProgress(100);
    printMessage("DisMo finished.");
}

void Praaline::Plugins::DisMo::PluginDisMo::addMWUindications(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    if (!corpus) {
        emit printMessage("DisMo: no corpus selected!");
        return;
    }

    QPointer<IntervalTier> tier_tok_min;
    QPointer<IntervalTier> tier_tok_mwu;

    int countDone = 0;
    madeProgress(0);
    printMessage("DisMo Annotator ver. 1.0 running");
    foreach(QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        printMessage(QString("Annotating %1").arg(com->ID()));
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID());
            foreach (QString speakerID, tiersAll.keys()) {
                printMessage(QString("   speaker %1").arg(speakerID));
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                tier_tok_min = tiers->getIntervalTierByName(d->levelTokMin);
                if (!tier_tok_min) { qDebug() << "tier not found tok_min" << annot->ID(); continue; }
                tier_tok_mwu = tiers->getIntervalTierByName(d->levelTokMWU);
                if (!tier_tok_mwu) { qDebug() << "tier not found tok_mwu" << annot->ID(); continue; }
                foreach (Interval *tok_min, tier_tok_min->intervals()) {
                    Interval *tok_mwu = tier_tok_mwu->intervalAtTime(tok_min->tCenter());
                    if (!tok_mwu) continue;
                    if ((tok_mwu->tMin() == tok_min->tMin()) && (tok_mwu->tMax() == tok_min->tMax()))
                        tok_min->setAttribute("part_of_mwu", false);
                    else
                        tok_min->setAttribute("part_of_mwu", true);
                }
                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_tok_mwu);
                qDebug() << QString("Annotated %1, speaker %2").arg(annot->ID()).arg(speakerID);

            }
            qDeleteAll(tiersAll);
            QApplication::processEvents();
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
    madeProgress(100);
    printMessage("DisMo finished.");
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::DisMo;
    Q_EXPORT_PLUGIN2(PluginDisMo, PluginDisMo)
#endif
