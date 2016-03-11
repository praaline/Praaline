#ifndef PLUGINALIGNER_H
#define PLUGINALIGNER_H

#include "pluginaligner_global.h"

#include "iannotationplugin.h"
#include <QtilitiesCategory>
#include <QObject>

namespace Praaline {
    namespace Plugins {
        namespace Aligner {
            /*!
              \struct PluginAlignerPrivateData
              \brief The PluginAlignerPrivateData struct stores private data used by the PluginAligner class.
             */
            struct PluginAlignerPrivateData;

            /*!
              \class PluginAligner
              \brief The Aligner plugin for Praaline.
              This class is the entry point into the plugin and is an implementation of the Qtilities::ExtensionSystem::Interfaces::IPlugin interface.
             */
            class PLUGIN_ALIGNER_SHARED_EXPORT PluginAligner : public QObject, public IAnnotationPlugin
            {
                Q_OBJECT
                Q_INTERFACES(Qtilities::ExtensionSystem::Interfaces::IPlugin)
                #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                Q_PLUGIN_METADATA(IID "org.praaline.PluginAligner")
                #endif

            public:
                PluginAligner(QObject* parent = 0);
                virtual ~PluginAligner();

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

            protected slots:
                void futureResultReadyAt(int index);
                void futureProgressValueChanged(int progressValue);
                void futureFinished();

            private:
                PluginAlignerPrivateData* d;
                void addPhonetisationToTokens(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications);
                void createFeatureFilesFromUtterances(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications);
                void align(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications);
                void createUtterancesFromProsogramAutosyll(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);
                void checks(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);
            };
        }
    }
}

#endif // PLUGINALIGNER_H
