#ifndef STATISTICSPLUGINPCA_H
#define STATISTICSPLUGINPCA_H

#include "interfaces/IStatisticsPlugin.h"
#include <QtilitiesCategory>
#include <QObject>

namespace Praaline {
    namespace Plugins {
        namespace StatisticsPluginPCA {
            /*!
              \struct StatisticsPluginPCAData
              \brief The StatisticsPluginPCAData struct stores private data used by the StatisticsPluginPCA class.
             */
            struct StatisticsPluginPCAData;

            /*!
              \class StatisticsPluginPCA
              \brief Statistical analysis plugin: Principal Component Analysis
              This class is the entry point into the plugin and is an implementation of the Qtilities::ExtensionSystem::Interfaces::IPlugin interface.
             */
            class StatisticsPluginPCA : public QObject, public IStatisticsPlugin
            {
                Q_OBJECT
                Q_INTERFACES(Qtilities::ExtensionSystem::Interfaces::IPlugin)
                #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                // Q_PLUGIN_METADATA(IID "org.praaline.StatisticsPluginPCA")
                #endif

            public:
                StatisticsPluginPCA(QObject* parent = 0);
                virtual ~StatisticsPluginPCA();

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
                QWidget *analyser(const QString &analyserName, CorpusRepository *repository, QWidget *parent = nullptr) override;

            private:
                StatisticsPluginPCAData* d;

            };
        }
    }
}

#endif // STATISTICSPLUGINPCA_H
