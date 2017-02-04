#ifndef ANNOTATIONINTERFACEPRAAT_H
#define ANNOTATIONINTERFACEPRAAT_H

#include "pncore_global.h"
#include <QObject>
#include <QString>

namespace Praaline {
namespace Core {

class CorpusRepository;
class AnnotationTierGroup;

struct AnnotationInterfacePraatData;

class PRAALINE_CORE_SHARED_EXPORT AnnotationInterfacePraat : public QObject
{
    Q_OBJECT
public:
    class Correspondance {
    public:
        Correspondance(const QString &levelID, const QString &attributeID, const QString &tierName) :
            levelID(levelID), attributeID(attributeID), tierName(tierName)
        {}

        QString levelID;
        QString attributeID;
        QString tierName;
    };
    enum SpeakerPolicy {
        SpeakerPolicy_SinglePerFile,
        SpeakerPolicy_PrefixTierNames,
        SpeakerPolicy_SpeakerIntervals,
        SpeakerPolicy_PrimaryAndSecondary
    };

    explicit AnnotationInterfacePraat(QObject *parent = 0);
    ~AnnotationInterfacePraat();

    const QList<Correspondance> &correpondances();
    void addCorrepondance(const QString &levelID, const QString &attributeID, const QString &tierName);
    void removeCorrepondance(const QString &levelID, const QString &annotationID);

    SpeakerPolicy speakerPolicy() const;
    void setSpeakerPolicy(SpeakerPolicy policy);

    QString exportPath() const;
    void setExportPath(const QString &path);

    QString exportFilenameTemplate() const;
    void setExportFilenameTemplate(const QString exportFilenameTemplate);

    bool exportAnnotation(CorpusRepository *repository, const QString &annotationID);

signals:

public slots:

private:
    AnnotationInterfacePraatData *d;

    void copyToTextgrid(AnnotationTierGroup *tiers, AnnotationTierGroup *txg, const QString &prefixTierNames = QString());
};

} // namespace Core
} // namespace Praaline

#endif // ANNOTATIONINTERFACEPRAAT_H
