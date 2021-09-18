#ifndef PLUGINPROMISE_H
#define PLUGINPROMISE_H

#include "PluginPromise_global.h"

#include "IAnnotationPlugin.h"
#include <QtilitiesCategory>
#include <QObject>
#include "PraalineCore/Base/DataType.h"

namespace Praaline {
    namespace Core {
        class AnnotationStructureLevel;
    }

    namespace Plugins {
        namespace Promise {
            /*!
              \struct PluginPromisePrivateData
              \brief The PluginPromisePrivateData struct stores private data used by the PluginPromise class.
             */
            struct PluginPromisePrivateData;

            /*!
              \class PluginPromise
              \brief The Promise annotator plugin for Praaline.
              This class is the entry point into the plugin and is an implementation of the Qtilities::ExtensionSystem::Interfaces::IPlugin interface.
             */
            class PLUGIN_PROMISE_SHARED_EXPORT PluginPromise : public QObject, public IAnnotationPlugin
            {
                Q_OBJECT
                Q_INTERFACES(Qtilities::ExtensionSystem::Interfaces::IPlugin)
                #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                Q_PLUGIN_METADATA(IID "org.praaline.PluginPromise")
                #endif

            public:
                PluginPromise(QObject *parent = nullptr);
                virtual ~PluginPromise();

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
                PluginPromisePrivateData* d;

                void createPromiseSyllableInfoStructure(CorpusRepository *repository);
                void runSpeechRateEstimator(const QList<CorpusCommunication *> &communications);
                void runSyllableProminenceAnnotator(const QList<CorpusCommunication *> &communications);
                void runProsodicBoundariesAnnotator(const QList<CorpusCommunication *> &communications);
                void runProsodicUnitsAnnotator(const QList<CorpusCommunication *> &communications);

                static void createAttribute(CorpusRepository *repository, AnnotationStructureLevel *level, const QString &prefix,
                                            const QString &ID, const QString &name = QString(), const QString &description = QString(),
                                            const DataType &datatype = DataType(DataType::VarChar, 256), int order = 0,
                                            bool indexed = false, const QString &nameValueList = QString());
            };
        }
    }
}

#endif // PLUGINPROMISE_H
