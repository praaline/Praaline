#ifndef PLUGINPROSOGRAM_H
#define PLUGINPROSOGRAM_H

#include "pluginprosogram_global.h"

#include "iannotationplugin.h"
#include <QtilitiesCategory>
#include <QObject>

namespace Praaline {
    namespace Plugins {
        namespace Prosogram {
            /*!
              \struct PluginProsogramPrivateData
              \brief The PluginProsogramPrivateData struct stores private data used by the PluginProsogram class.
             */
            struct PluginProsogramPrivateData;

            /*!
              \class PluginProsogram
              \brief The Prosogram annotator plugin for Praaline.
              This class is the entry point into the plugin and is an implementation of the Qtilities::ExtensionSystem::Interfaces::IPlugin interface.
             */
            class PLUGIN_PROSOGRAM_SHARED_EXPORT PluginProsogram : public QObject, public IAnnotationPlugin
            {
                Q_OBJECT
                Q_INTERFACES(Qtilities::ExtensionSystem::Interfaces::IPlugin)
                #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                Q_PLUGIN_METADATA(IID "org.praaline.PluginProsogram")
                #endif

            public:
                PluginProsogram(QObject* parent = 0);
                virtual ~PluginProsogram();

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

            private slots:
                void scriptSentMessage(QString message);
                void scriptFinished(int exitcode);

            private:
                PluginProsogramPrivateData* d;

                void createProsogramSyllableInfoStructure(Corpus *corpus);
                void createSegmentsFromAutoSyllables(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications);
            };
        }
    }
}

#endif // PLUGINPROSOGRAM_H
