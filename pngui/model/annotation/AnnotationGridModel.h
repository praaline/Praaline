#ifndef ANNOTATIONGRIDMODEL_H
#define ANNOTATIONGRIDMODEL_H

#include <QPointer>
#include <QMap>
#include <QList>
#include <QPair>
#include <QStringList>

#include "pncore/base/RealTime.h"
#include "svcore/base/XmlExportable.h"
#include "svcore/data/model/Model.h"
#include "pngui/model/annotation/annotationtiermodel.h"
#include "pngui/model/annotation/AnnotationGridPointModel.h"

struct AnnotationGridModelData;

class AnnotationGridModel : public Model
{
    Q_OBJECT

public:
    struct TierTuple {
        TierTuple(const QString &speakerID, const QString &levelID, const QString &attributeID,
                  int indexLevelAttributePair) :
            speakerID(speakerID), levelID(levelID), attributeID(attributeID), indexLevelAttributePair(indexLevelAttributePair) {}
        QString speakerID;
        QString levelID;
        QString attributeID;
        int indexLevelAttributePair;
    };

    enum GridLayout {
        LayoutSpeakersThenLevelAttributes,
        LayoutLevelAttributesThenSpeakers
    };

    AnnotationGridModel(sv_samplerate_t sampleRate,
                        QMap<QString, QPointer<Praaline::Core::AnnotationTierGroup> > &tiers,
                        const QList<QPair<QString, QString> > &attributes);
    virtual ~AnnotationGridModel();

    virtual std::string getType() const { return "AnnotationGridModel"; }

    // Model base class implementation
    virtual bool isOK() const { return true; }
    virtual sv_frame_t getStartFrame() const;
    virtual sv_frame_t getEndFrame() const;
    virtual sv_samplerate_t getSampleRate() const;
    virtual QString getTypeName() const { return tr("Annotation Grid"); }

    virtual void toXml(QTextStream &out, QString indent = "", QString extraAttributes = "") const;

    int countSpeakers() const;
    int countLevels() const;
    int countLevelsAttributes() const;
    int countAttributesForLevel(const QString &levelID) const;
    QList<QString> speakers() const;
    QList<QString> levels() const;
    QList<QString> attributesForLevel(const QString &levelID) const;

    void excludeSpeakerIDs(const QList<QString> &list);
    void clearExcludedSpeakerIDs();

    QList<TierTuple> tierTuples(const GridLayout) const;

    QPointer<AnnotationGridPointModel> boundariesForLevel(const QString &levelID);
    QVariant data(const QString &speakerID, const QString &levelID, const QString &attributeID, const int index) const;
    bool setData(const QString &speakerID, const QString &levelID, const QString &attributeID, const int index, const QVariant &value);

protected:
    AnnotationGridModelData *d;
};


#endif // ANNOTATIONGRIDMODEL_H
