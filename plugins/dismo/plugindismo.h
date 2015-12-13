#ifndef PLUGINDISMO_H
#define PLUGINDISMO_H

#include "plugindismo_global.h"

#include "iannotationplugin.h"
#include <QtilitiesCategory>
#include <QObject>

namespace Praaline {
    namespace Plugins {
        namespace DisMo {
            /*!
              \struct PluginDisMoPrivateData
              \brief The PluginDisMoPrivateData struct stores private data used by the PluginDisMo class.
             */
            struct PluginDisMoPrivateData;

            /*!
              \class PluginDisMo
              \brief The DisMo annotator plugin for Praaline.
              This class is the entry point into the plugin and is an implementation of the Qtilities::ExtensionSystem::Interfaces::IPlugin interface.
             */
            class PLUGIN_DISMO_SHARED_EXPORT PluginDisMo : public QObject, public IAnnotationPlugin
            {
                Q_OBJECT
                Q_INTERFACES(Qtilities::ExtensionSystem::Interfaces::IPlugin)
                Q_INTERFACES(Praaline::Plugins::IAnnotationPlugin)
                #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                Q_PLUGIN_METADATA(IID "org.praaline.PluginDisMo")
                #endif

            public:
                PluginDisMo(QObject* parent = 0);
                ~PluginDisMo();

                // IObjectBase implementation
                QObject* objectBase() { return this; }
                const QObject* objectBase() const { return this; }

                // IPlugin implementation
                bool initialize(const QStringList &arguments, QStringList *error_strings);
                bool initializeDependencies(QStringList *error_strings);
                void finalize();
                QString pluginName() const;
                QtilitiesCategory pluginCategory() const;
                VersionInformation pluginVersionInformation() const;
                QString pluginPublisher() const;
                QString pluginPublisherWebsite() const;
                QString pluginPublisherContact() const;
                QString pluginDescription() const;
                QString pluginCopyright() const;
                QString pluginLicense() const;

                // IAnnotationPlugin implementation
                QList<PluginParameter> pluginParameters() const;
                void setParameters(QHash<QString, QVariant> parameters);
                void process(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);

            signals:
                void printMessage(QString message);
                void madeProgress(int progress);

            private:
                PluginDisMoPrivateData* d;

                void addMWUindications(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);
            };
        }
    }
}

#endif // PLUGINDISMO_H
