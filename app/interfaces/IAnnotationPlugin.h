#ifndef PRAALINE_IANNOTATIONPLUGIN_H
#define PRAALINE_IANNOTATIONPLUGIN_H

#include <QString>
#include <QList>
#include <QHash>
#include <QPointer>
#include <IPlugin>
using namespace Qtilities::ExtensionSystem::Interfaces;
#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
using namespace Praaline::Core;

namespace Praaline {
    namespace Plugins {
        class IAnnotationPlugin : public IPlugin
        {
        public:
            class PluginParameter {
            public:
                QString ID;
                QString name;
                int propertyType;
                QVariant defaultValue;
                QStringList listOptions;
                QString description;

                PluginParameter(const QString &ID, const QString &name, int propertyType,
                                const QVariant &defaultValue = QVariant(),
                                const QStringList &listOptions = QStringList(), const QString &description = QString()) :
                    ID(ID), name(name), propertyType(propertyType), defaultValue(defaultValue),
                    listOptions(listOptions), description(description)
                {}
            };

            IAnnotationPlugin() : IPlugin() {}
            virtual ~IAnnotationPlugin() {}

            virtual QList<PluginParameter> pluginParameters() const = 0;
            virtual void setParameters(const QHash<QString, QVariant> &parameters) = 0;
            virtual void process(const QList<QPointer<CorpusCommunication> > &communications) = 0;
        signals:
            virtual void printMessage(const QString &message) = 0 ;
            virtual void madeProgress(int progress) = 0;

        public slots:
        };
    }
}

Q_DECLARE_INTERFACE(Praaline::Plugins::IAnnotationPlugin, "org.Praaline.Plugins.IAnnotationPlugin/1.0")

#endif // PRAALINE_IANNOTATIONPLUGIN_H
