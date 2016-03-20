#ifndef PLUGINDISFLUENCYANALYSER_H
#define PLUGINDISFLUENCYANALYSER_H

#include "plugindisfluencyanalyser_global.h"

#include "IAnnotationPlugin.h"
#include <QtilitiesCategory>
#include <QObject>

namespace Praaline {
    namespace Plugins {
        namespace DisfluencyAnalyser {
            /*!
              \struct PluginDisfluencyAnalyserPrivateData
              \brief The PluginDisfluencyAnalyserPrivateData struct stores private data used by the PluginDisfluencyAnalyser class.
             */
            struct PluginDisfluencyAnalyserPrivateData;

            /*!
              \class PluginDisfluencyAnalyser
              \brief The DisfluencyAnalyser annotator plugin for Praaline.
              This class is the entry point into the plugin and is an implementation of the Qtilities::ExtensionSystem::Interfaces::IPlugin interface.
             */
            class PLUGIN_DISFLUENCYANALYSER_SHARED_EXPORT PluginDisfluencyAnalyser : public QObject, public IAnnotationPlugin
            {
                Q_OBJECT
                Q_INTERFACES(Qtilities::ExtensionSystem::Interfaces::IPlugin)
                #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                Q_PLUGIN_METADATA(IID "org.praaline.PluginDisfluencyAnalyser")
                #endif

            public:
                PluginDisfluencyAnalyser(QObject* parent = 0);
                ~PluginDisfluencyAnalyser();

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
                PluginDisfluencyAnalyserPrivateData* d;

                void concordances(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);
                void patterns(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications, const QStringList &codes);
                void exportMultiTierTextgrids(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);
            };
        }
    }
}

#endif // PLUGINDISFLUENCYANALYSER_H
