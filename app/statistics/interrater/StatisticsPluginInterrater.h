#ifndef STATISTICSPLUGINInterrater_H
#define STATISTICSPLUGINInterrater_H

#include "interfaces/IStatisticsPlugin.h"
#include <QtilitiesCategory>
#include <QObject>

namespace Praaline {
    namespace Plugins {
        namespace StatisticsPluginInterrater {
            /*!
              \struct StatisticsPluginInterraterData
              \brief The StatisticsPluginInterraterData struct stores private data used by the StatisticsPluginInterrater class.
             */
            struct StatisticsPluginInterraterData;

            /*!
              \class StatisticsPluginInterrater
              \brief Statistical analysis plugin: Interrater Measures
              This class is the entry point into the plugin and is an implementation of the Qtilities::ExtensionSystem::Interfaces::IPlugin interface.
             */
            class StatisticsPluginInterrater : public QObject, public IStatisticsPlugin
            {
                Q_OBJECT
                Q_INTERFACES(Qtilities::ExtensionSystem::Interfaces::IPlugin)
                #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                // Q_PLUGIN_METADATA(IID "org.praaline.StatisticsPluginInterrater")
                #endif

            public:
                StatisticsPluginInterrater(QObject* parent = 0);
                virtual ~StatisticsPluginInterrater();

                // IObjectBase implementation
                QObject* objectBase() override { return this; }
                const QObject* objectBase() const override { return this; }

                // IPlugin implementation
                bool initialize(const QStringList &arguments, QStringList *error_strings) override;
                bool initializeDependencies(QStringList *error_strings) override;
                void finalize() override;
                QString pluginName() const override;
                QtilitiesCategory pluginCategory() const override;
                VersionInformation pluginVersionInformation() const override;
                QString pluginPublisher() const override;
                QString pluginPublisherWebsite() const override;
                QString pluginPublisherContact() const override;
                QString pluginDescription() const override;
                QString pluginCopyright() const override;
                QString pluginLicense() const override;

                // IStatisticsPlugin implementation
                QStringList analyserIDs() override;
                QString analyserName(const QString &analyserID) override;
                QWidget *analyser(const QString &analyserName, CorpusRepository *repository) override;

            private:
                StatisticsPluginInterraterData* d;

            };
        }
    }
}

#endif // STATISTICSPLUGINInterrater_H
