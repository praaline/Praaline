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
            class PluginParameter {
            public:
                QString ID;
                QString name;
                int propertyType;
                QVariant defaultValue;
                QStringList listOptions;
                QString description;

                PluginParameter(QString ID, QString name, int propertyType, QVariant defaultValue = QVariant(),
                                QStringList listOptions = QStringList(), QString description = QString()) :
                    ID(ID), name(name), propertyType(propertyType), defaultValue(defaultValue),
                    listOptions(listOptions), description(description)
                {}
            };

            IStatisticsPlugin() : IPlugin() {}
            virtual ~IStatisticsPlugin() {}

            virtual QList<PluginParameter> pluginParameters() const = 0;
            virtual void setParameters(QHash<QString, QVariant> parameters) = 0;
            virtual void process(Corpus *corpus) = 0;
        signals:
            virtual void printMessage(QString message) = 0 ;
            virtual void madeProgress(int progress) = 0;

        public slots:
        };
    }
}

Q_DECLARE_INTERFACE(Praaline::Plugins::IStatisticsPlugin, "org.Praaline.Plugins.IStatisticsPlugin/1.0")

#endif // PRAALINE_ISTATISTICSPLUGIN_H
