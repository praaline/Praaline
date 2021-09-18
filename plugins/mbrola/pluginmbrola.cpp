#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include <QFile>
#include <QTextStream>

#include "pluginmbrola.h"


#include "PraalineCore/Base/RealValueList.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "mbrolafilemanager.h"
#include "pseudolanguage.h"
#include "MBROLAResynthesiser.h"


using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::MBROLA::PluginMBROLAPrivateData {
    PluginMBROLAPrivateData() {}
};

Praaline::Plugins::MBROLA::PluginMBROLA::PluginMBROLA(QObject* parent) : QObject(parent)
{
    d = new PluginMBROLAPrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::MBROLA::PluginMBROLA::~PluginMBROLA()
{
    delete d;
}

bool Praaline::Plugins::MBROLA::PluginMBROLA::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::MBROLA::PluginMBROLA::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::MBROLA::PluginMBROLA::finalize() {

}

QString Praaline::Plugins::MBROLA::PluginMBROLA::pluginName() const {
    return "MBROLA";
}

QtilitiesCategory Praaline::Plugins::MBROLA::PluginMBROLA::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::MBROLA::PluginMBROLA::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::MBROLA::PluginMBROLA::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::MBROLA::PluginMBROLA::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::MBROLA::PluginMBROLA::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::MBROLA::PluginMBROLA::pluginDescription() const {
    return tr("A bridge to the MBROLA diphone-based speech synthesis system.");
}

QString Praaline::Plugins::MBROLA::PluginMBROLA::pluginCopyright() const {
    return QString(tr("Copyright") + " 2015, George Christodoulides");
}

QString Praaline::Plugins::MBROLA::PluginMBROLA::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::MBROLA::PluginMBROLA::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    return parameters;
}

void Praaline::Plugins::MBROLA::PluginMBROLA::setParameters(const QHash<QString, QVariant> &parameters)
{
    Q_UNUSED(parameters)
}

// ====================================================================================================================

void Praaline::Plugins::MBROLA::PluginMBROLA::scriptSentMessage(const QString &message)
{
    emit printMessage(message);
}

void Praaline::Plugins::MBROLA::PluginMBROLA::scriptFinished(int exitcode)
{
    if (exitcode == 0)
        emit printMessage("Finished succesfully.");
    else
        emit printMessage(QString("Finished with errors. The error code was %1").arg(exitcode));
}

// ====================================================================================================================

void Praaline::Plugins::MBROLA::PluginMBROLA::process(const QList<CorpusCommunication *> &communications)
{
    QString m;
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;
        QString dir = "/Users/george/Documents/resynth";
        // dir = "/home/george/resynth";
        m = MBROLAResynthesiser::resynthesise(dir, com);
        if (!m.isEmpty()) emit printMessage(m);
    }

//    QFile file("D:/DROPBOX/2015-10_SP8_ProsodicBoundariesExpe/test.txt");
//    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;
//    QTextStream out(&file);
//    out.setCodec("UTF-8");
//    out << "communicationID\tspeakerID\tsyll_intervalNo\tsyll_tMin\tboundary\tcontour\tcategory\ttok_mwu\tpos\tpos_cat\t";
//    out << "isLexical\tnpauseDur\tnpauseDurLog\tnpauseDurLogZ\tsyllDurRel20\tsyllDurRel30\tsyllDurRel40\tsyllDurRel50\tsyllDurLogRel20\tsyllDurLogRel30\tsyllDurLogRel40\tsyllDurLogRel50\tsyllF0meanRel20\tsyllF0meanRel30\tsyllF0meanRel40\tsyllF0meanRel50\tintrasyllab_up\tintrasyllab_down\ttrajectory\tcorrect\n";

//    MBROLAFileManager *mgr = new MBROLAFileManager();
//    connect(mgr, SIGNAL(logOutput(QString)), this, SLOT(scriptSentMessage(QString)));
//    connect(mgr, SIGNAL(finished(int)), this, SLOT(scriptFinished(int)));

//    PseudoLanguage *pseudo = new PseudoLanguage();

//    foreach (CorpusCommunication *com, communications) {
//        if (!com) continue;
//        foreach (CorpusRecording *rec, com->recordings()) {
//            if (!rec) continue;
//            foreach (CorpusAnnotation *annot, com->annotations()) {
//                if (!annot) continue;

//                SpeakerAnnotationTierGroupMap tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID());
//                foreach (QString speakerID, tiersAll.keys()) {
//                    AnnotationTierGroup *tiers = tiersAll.value(speakerID);

//                    IntervalTier *tier_phones = tiers->getIntervalTierByName("phone");
//                    if (!tier_phones) continue;

//                    QString filename = QString("D:/DROPBOX/2015-10_SP8_ProsodicBoundariesExpe/synth/%1_%2_pseudo.pho")
//                            .arg(annot->ID()).arg(speakerID);

//                    if (mgr->updatePhoneTierFromPhoFile(filename, tier_phones, "pseudo_phone", true, false))
//                        printMessage(filename);

////                    pseudo->substitutePhonemes(tier_phones, "pseudolang");
////                    mgr->createPhoFile(QString("D:/DROPBOX/2015-10_SP8_ProsodicBoundariesExpe/%1_%2_pseudo.pho")
////                                       .arg(annot->ID()).arg(speakerID), tier_phones, "pseudolang");

//                    com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_phones);
//                }
//                qDeleteAll(tiersAll);

//            }
//        }
//    }

//    delete pseudo;
//    disconnect(mgr, SIGNAL(logOutput(QString)), this, SLOT(scriptSentMessage(QString)));
//    disconnect(mgr, SIGNAL(finished(int)), this, SLOT(scriptFinished(int)));
//    delete mgr;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::MBROLA;
    Q_EXPORT_PLUGIN2(PluginMBROLA, PluginMBROLA)
#endif
