#ifndef EXMARALDATRANSCRIPTIONBRIDGE_H
#define EXMARALDATRANSCRIPTIONBRIDGE_H

#include <QObject>
#include <QList>
#include "annotation/annotationtier.h"
#include "exmaraldabasictranscription.h"

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

#endif // EXMARALDATRANSCRIPTIONBRIDGE_H
