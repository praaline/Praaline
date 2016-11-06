#ifndef EXMARALDATRANSCRIPTIONBRIDGE_H
#define EXMARALDATRANSCRIPTIONBRIDGE_H

#include <QObject>
#include <QList>
#include "annotation/AnnotationTier.h"
#include "ExmaraldaBasicTranscription.h"

namespace Praaline {
namespace Core {

class ExmaraldaTranscriptionBridge : public QObject
{
    Q_OBJECT
public:
    class TierToExportInfo {
    public:
        QString tierID;
        QString speakerID;
        QString type;
        QString category;
        QString displayName;
        AnnotationTier *tierToExport;
    };

    explicit ExmaraldaTranscriptionBridge(QObject *parent = 0);
    void addTier(const QString &tierID, const QString &speakerID,
                 const QString &type, const QString &category,
                 const QString &displayName,
                 AnnotationTier *tierToExport);

    void exportPraalineToPartitur(ExmaraldaBasicTranscription &partitur);
    
signals:
    
public slots:
    
private:
    QList<TierToExportInfo > m_tiersToExport;
};

} // namespace Core
} // namespace Praaline

#endif // EXMARALDATRANSCRIPTIONBRIDGE_H
