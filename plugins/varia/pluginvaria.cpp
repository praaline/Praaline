#include <QString>
#include <QList>
#include <QStringList>
#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>
#include <QThreadPool>

#include <QFile>
#include <QTextStream>

#include "chunkannotator.h"
#include "svbridge.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"
#include "PraalineCore/Interfaces/Anvil/AnvilMetadataTranscript.h"

#include "InterraterAgreement.h"
#include "SequencerSyntax.h"
#include "SequencerDisfluencies.h"
#include "SequencerProsodicUnits.h"
#include "SequencerCombineUnits.h"
#include "BratAnnotationExporter.h"
#include "PhonetiserExternal.h"
#include "BratSyntaxAndDisfluencies.h"
#include "ProsodicUnits.h"
#include "SilentPauseManipulator.h"
#include "PhonoSeesaw.h"
#include "IntervalTierCombinations.h"

#include "corpus-specific/CPROMDISS.h"
#include "corpus-specific/Rhapsodie.h"
#include "corpus-specific/NCCFR.h"
#include "corpus-specific/ORFEO.h"
#include "YizhiScripts.h"
#include "SophieScripts.h"

#include "pluginvaria.h"


using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Varia::PluginVariaPrivateData {
    PluginVariaPrivateData() {}

    QFuture<QString> future;
    QFutureWatcher<QString> watcher;
};

Praaline::Plugins::Varia::PluginVaria::PluginVaria(QObject* parent) :
    QObject(parent), d(new PluginVariaPrivateData)
{
    setObjectName(pluginName());
    connect(&(d->watcher), SIGNAL(resultReadyAt(int)), this, SLOT(futureResultReadyAt(int)));
    connect(&(d->watcher), SIGNAL(progressValueChanged(int)), this, SLOT(futureProgressValueChanged(int)));
    connect(&(d->watcher), SIGNAL(finished()), this, SLOT(futureFinished()));
}

Praaline::Plugins::Varia::PluginVaria::~PluginVaria()
{
    delete d;
}

bool Praaline::Plugins::Varia::PluginVaria::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::Varia::PluginVaria::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::Varia::PluginVaria::finalize() {

}

QString Praaline::Plugins::Varia::PluginVaria::pluginName() const {
    return "Varia";
}

QtilitiesCategory Praaline::Plugins::Varia::PluginVaria::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::Varia::PluginVaria::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::Varia::PluginVaria::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::Varia::PluginVaria::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::Varia::PluginVaria::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::Varia::PluginVaria::pluginDescription() const {
    return tr("Varia is a part-of-speech tagger, multi-word unit and disfluency detector for spoken language transcriptions.");
}

QString Praaline::Plugins::Varia::PluginVaria::pluginCopyright() const {
    return QString(tr("Copyright") + " 2012-2014, George Christodoulides");
}

QString Praaline::Plugins::Varia::PluginVaria::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::Varia::PluginVaria::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    return parameters;
}

void Praaline::Plugins::Varia::PluginVaria::setParameters(const QHash<QString, QVariant> &parameters)
{
    Q_UNUSED(parameters)
}

// ====================================================================================================================
// Asynchronous execution: basic event handling
// ====================================================================================================================

void Praaline::Plugins::Varia::PluginVaria::futureResultReadyAt(int index)
{
    QString result = d->watcher.resultAt(index);
    qDebug() << d->watcher.progressValue() << result;
    emit printMessage(result);
    emit madeProgress(d->watcher.progressValue() * 100 / d->watcher.progressMaximum());
}

void Praaline::Plugins::Varia::PluginVaria::futureProgressValueChanged(int progressValue)
{
    qDebug() << progressValue;
    if (d->watcher.progressMaximum() > 0)
        emit madeProgress(progressValue * 100 / d->watcher.progressMaximum());
    else
        emit madeProgress(100);
}

void Praaline::Plugins::Varia::PluginVaria::futureFinished()
{
    emit madeProgress(100);
    emit printMessage("Finished");
    qDebug() << "Finished";
}

// ====================================================================================================================
// Asynchronous execution steps: procedures for each plugin sub-commmand
// ====================================================================================================================

struct RunOrfeoAlignStep
{
    RunOrfeoAlignStep() {}
    typedef QString result_type;

    QString operator() (CorpusCommunication *com)
    {
        if (!com) return QString("%1\tis empty.").arg(com->ID());
        ORFEO orfeo;
        return orfeo.align(com);
    }
};

// ====================================================================================================================

void Praaline::Plugins::Varia::PluginVaria::process(const QList<CorpusCommunication *> &communications)
{
//    madeProgress(0);
//    printMessage("Starting");
//    QElapsedTimer timer;
//    QThreadPool::globalInstance()->setMaxThreadCount(16);
//    timer.start();
//    d->future = QtConcurrent::mapped(communications, RunOrfeoAlignStep());
//    d->watcher.setFuture(d->future);
//    while (d->watcher.isRunning()) QApplication::processEvents();
//    printMessage(QString("Time: %1 seconds").arg(static_cast<double>(timer.elapsed()) / 1000.0));
//    madeProgress(100);
//    return;

    SophieScripts sophie;

    ORFEO orfeo;
    QString m;
    int countDone = 0;
    madeProgress(0);
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;
        // printMessage(sophie.importPhonAnnotation(com));

//        // printMessage(orfeo.updateSoundFiles(com));
//        // printMessage(orfeo.readOrfeoFile(com));
//        // printMessage(orfeo.mapTokensToDisMo(com));
          // printMessage(orfeo.phonetise(com));
//        // printMessage(orfeo.createUtterances(com));
//        // printMessage(orfeo.align(com));
//        printMessage(com->ID());
         // Prosogram + Promise

        // printMessage(orfeo.createSentenceUnits(com));
        printMessage(orfeo.createMajorProsodicUnits(com));
        // printMessage(orfeo.createCombinedUnits(com));

        countDone++;
        madeProgress(countDone * 100 / communications.count());

        if (!m.isEmpty()) printMessage(m);
    }
    // orfeo.phonetiseOOV();

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Varia;
    Q_EXPORT_PLUGIN2(PluginVaria, PluginVaria)
#endif

