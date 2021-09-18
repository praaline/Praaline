#ifndef PLUGIN_ANONYMISER_H
#define PLUGIN_ANONYMISER_H

#include "PluginAnonymiser_global.h"

#include "IAnnotationPlugin.h"
#include <QtilitiesCategory>
#include <QObject>

namespace Praaline {
    namespace Plugins {
        namespace Anonymiser {
            /*!
              \struct PluginAnonymiserPrivateData
              \brief The PluginAnonymiserPrivateData struct stores private data used by the PluginAnonymiser class.
             */
            struct PluginAnonymiserPrivateData;

            /*!
              \class PluginAnonymiser
              \brief The Anonymiser annotator plugin for Praaline.
              This class is the entry point into the plugin and is an implementation of the Qtilities::ExtensionSystem::Interfaces::IPlugin interface.
             */
            class PLUGIN_ANONYMISER_SHARED_EXPORT PluginAnonymiser : public QObject, public IAnnotationPlugin
            {
                Q_OBJECT
                Q_INTERFACES(Qtilities::ExtensionSystem::Interfaces::IPlugin)
                #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                Q_PLUGIN_METADATA(IID "org.praaline.PluginAnonymiser")
                #endif

            public:
                PluginAnonymiser(QObject *parent = nullptr);
                ~PluginAnonymiser();

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

            private slots:
                void scriptSentMessage(const QString &message);
                void scriptFinished(int exitcode);

            private:
                PluginAnonymiserPrivateData* d;

                // void concordances(const QList<CorpusCommunication *> &communications);
            };
        }
    }
}

#endif // PLUGIN_ANONYMISER_H
