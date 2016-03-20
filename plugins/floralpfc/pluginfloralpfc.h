#ifndef PLUGINFLORALPFC_H
#define PLUGINFLORALPFC_H

#include "pluginfloralpfc_global.h"

#include "IAnnotationPlugin.h"
#include <QtilitiesCategory>
#include <QObject>

namespace Praaline {
    namespace Plugins {
        namespace FloralPFC {
            /*!
              \struct PluginFloralPFCPrivateData
              \brief The PluginFloralPFCPrivateData struct stores private data used by the PluginFloralPFC class.
             */
            struct PluginFloralPFCPrivateData;

            /*!
              \class PluginFloralPFC
              \brief The FloralPFC plugin for Praaline.
              This class is the entry point into the plugin and is an implementation of the Qtilities::ExtensionSystem::Interfaces::IPlugin interface.
             */
            class PLUGIN_FLORALPFC_SHARED_EXPORT PluginFloralPFC : public QObject, public IAnnotationPlugin
            {
                Q_OBJECT
                Q_INTERFACES(Qtilities::ExtensionSystem::Interfaces::IPlugin)
                #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                Q_PLUGIN_METADATA(IID "org.praaline.PluginFloralPFC")
                #endif

            public:
                PluginFloralPFC(QObject* parent = 0);
                virtual ~PluginFloralPFC();

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
                PluginFloralPFCPrivateData* d;
            };
        }
    }
}

#endif // PLUGINFLORALPFC_H
