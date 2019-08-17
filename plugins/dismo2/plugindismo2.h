#ifndef PLUGINDISMO2_H
#define PLUGINDISMO2_H

#include "plugindismo2_global.h"

#include "IAnnotationPlugin.h"
#include <QtilitiesCategory>
#include <QObject>

namespace Praaline {
    namespace Plugins {
        namespace DisMo2 {
            /*!
              \struct PluginDisMoPrivateData
              \brief The PluginDisMoPrivateData struct stores private data used by the PluginDisMo class.
             */
            struct PluginDisMo2PrivateData;

            /*!
              \class PluginDisMo
              \brief The DisMo annotator plugin for Praaline.
              This class is the entry point into the plugin and is an implementation of the Qtilities::ExtensionSystem::Interfaces::IPlugin interface.
             */
            class PLUGIN_DISMO2_SHARED_EXPORT PluginDisMo2 : public QObject, public IAnnotationPlugin
            {
                Q_OBJECT
                Q_INTERFACES(Qtilities::ExtensionSystem::Interfaces::IPlugin)
                Q_INTERFACES(Praaline::Plugins::IAnnotationPlugin)
                #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                Q_PLUGIN_METADATA(IID "org.praaline.PluginDisMo2")
                #endif

            public:
                PluginDisMo2(QObject* parent = 0);
                ~PluginDisMo2();

                // IObjectBase implementation
                QObject* objectBase() override { return this; }
                const QObject* objectBase() const override { return this; }

                // IPlugin implementation
                bool initialize(const QStringList &arguments, QStringList *error_strings) override ;
                bool initializeDependencies(QStringList *error_strings) override ;
                void finalize() override ;
                QString pluginName() const override ;
                QtilitiesCategory pluginCategory() const override ;
                VersionInformation pluginVersionInformation() const override ;
                QString pluginPublisher() const override ;
                QString pluginPublisherWebsite() const override ;
                QString pluginPublisherContact() const override ;
                QString pluginDescription() const override ;
                QString pluginCopyright() const override ;
                QString pluginLicense() const override ;

                // IAnnotationPlugin implementation
                QList<PluginParameter> pluginParameters() const override ;
                void setParameters(const QHash<QString, QVariant> &parameters) override ;
                void process(const QList<CorpusCommunication *> &communications) override ;

            signals:
                void printMessage(const QString &message) override;
                void madeProgress(int progress) override;

            private:
                PluginDisMo2PrivateData* d;
            };
        }
    }
}

#endif // PLUGINDISMO_H
