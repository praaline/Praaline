#ifndef PRAALINE_ISTATISTICSPLUGIN_H
#define PRAALINE_ISTATISTICSPLUGIN_H

#include <QString>
#include <QList>
#include <QHash>
#include <QPointer>
#include <IPlugin>
using namespace Qtilities::ExtensionSystem::Interfaces;
#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

namespace Praaline {
    namespace Plugins {
        class IStatisticsPlugin : public IPlugin
        {
        public:
            IStatisticsPlugin() : IPlugin() {}
            virtual ~IStatisticsPlugin() {}

            virtual QStringList analyserIDs() = 0;
            virtual QString analyserName(const QString &analyserID) = 0;
            virtual QWidget *analyser(const QString &analyserID, CorpusRepository *repository) = 0;

        signals:

        public slots:
        };
    }
}

Q_DECLARE_INTERFACE(Praaline::Plugins::IStatisticsPlugin, "org.Praaline.Plugins.IStatisticsPlugin/1.0")

#endif // PRAALINE_ISTATISTICSPLUGIN_H
