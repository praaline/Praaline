#ifndef PLUGINMETRONOME_H
#define PLUGINMETRONOME_H

#include "PluginMetronome_global.h"

#include "IAnnotationPlugin.h"
#include <QtilitiesCategory>
#include <QObject>

namespace Praaline {
    namespace Plugins {
        namespace Metronome {
            /*!
              \struct PluginMetronomePrivateData
              \brief The PluginMetronomePrivateData struct stores private data used by the PluginMetronome class.
             */
            struct PluginMetronomePrivateData;

            /*!
              \class PluginMetronome
              \brief The Metronome annotator plugin for Praaline.
              This class is the entry point into the plugin and is an implementation of the Qtilities::ExtensionSystem::Interfaces::IPlugin interface.
             */
            class PLUGIN_METRONOME_SHARED_EXPORT PluginMetronome : public QObject, public IAnnotationPlugin
            {
                Q_OBJECT
                Q_INTERFACES(Qtilities::ExtensionSystem::Interfaces::IPlugin)
                #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                Q_PLUGIN_METADATA(IID "org.praaline.PluginMetronome")
                #endif

            public:
                PluginMetronome(QObject* parent = 0);
                ~PluginMetronome();

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

                // IAnnotationPlugin implementation
                QList<PluginParameter> pluginParameters() const override;
                void setParameters(const QHash<QString, QVariant> &parameters) override;
                void process(const QList<CorpusCommunication *> &communications) override;

            signals:
                void printMessage(const QString &message) override;
                void madeProgress(int progress) override;

            private:
                PluginMetronomePrivateData* d;

                void annotateRhythmicPatterns(const QList<CorpusCommunication *> &communications);
                void createMetronomeAnnotationStructure(CorpusRepository *repository);
            };
        }
    }
}

#endif // PLUGINMETRONOME_H
