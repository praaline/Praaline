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
#include "pngui/model/annotation/AnnotationMultiTierTableModel.h"
#include "pngui/model/visualiser/AnnotationGridPointModel.h"

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

    // Model base class implementation
    virtual std::string getType() const { return "AnnotationGridModel"; }
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
    QVariant data(const AnnotationGridPointModel::Point &boundary, const QString &attributeID) const;
    QVariant data(const QString &speakerID, const QString &levelID, const sv_frame_t frame, const QString &attributeID) const;
    bool setData(const AnnotationGridPointModel::Point &boundary, const QString &attributeID, const QVariant &value);
    bool setData(const QString &speakerID, const QString &levelID, const sv_frame_t frame, const QString &attributeID, const QVariant &value);

    RealTime elementDuration(const AnnotationGridPointModel::Point &boundary) const;
    sv_frame_t elementMoveLimitFrameLeft(const AnnotationGridPointModel::Point &boundary) const;
    sv_frame_t elementMoveLimitFrameRight(const AnnotationGridPointModel::Point &boundary) const;

    bool addBoundary(const AnnotationGridPointModel::Point &boundary);
    bool deleteBoundary(const AnnotationGridPointModel::Point &boundary);
    bool moveBoundary(const AnnotationGridPointModel::Point &oldBoundary, const AnnotationGridPointModel::Point &newBoundary);

    // Commands
    // Command to add a boundary, with undo.
    class AddBoundaryCommand : public UndoableCommand
    {
    public:
        AddBoundaryCommand(AnnotationGridModel *model, const AnnotationGridPointModel::Point &boundary,
                           const QString &name = QString()) :
            m_model(model), m_boundary(boundary), m_name(name) { }
        virtual QString getName() const { return (m_name == "" ? tr("Add Boundary") : m_name); }
        virtual void execute();
        virtual void unexecute();
        const AnnotationGridPointModel::Point &getBoundary() const { return m_boundary; }
    private:
        AnnotationGridModel *m_model;
        AnnotationGridPointModel::Point m_boundary;
        QString m_name;
    };

    // Command to remove a boundary, with undo.
    class DeleteBoundaryCommand : public UndoableCommand
    {
    public:
        DeleteBoundaryCommand(AnnotationGridModel *model, const AnnotationGridPointModel::Point &boundary) :
            m_model(model), m_boundary(boundary) { }
        virtual QString getName() const { return tr("Delete Boundary"); }
        virtual void execute();
        virtual void unexecute();
        const AnnotationGridPointModel::Point &getBoundary() const { return m_boundary; }
    private:
        AnnotationGridModel *m_model;
        AnnotationGridPointModel::Point m_boundary;
    };

    // Command to move a boundary, with undo.
    class MoveBoundaryCommand : public UndoableCommand
    {
    public:
        MoveBoundaryCommand(AnnotationGridModel *model, const AnnotationGridPointModel::Point &boundary, sv_frame_t newFrame) :
            m_model(model), m_oldBoundary(boundary), m_newBoundary(boundary)
        {
            m_newBoundary.frame = newFrame;
        }
        virtual QString getName() const { return tr("Move Boundary"); }
        virtual void execute();
        virtual void unexecute();
    private:
        AnnotationGridModel *m_model;
        AnnotationGridPointModel::Point m_oldBoundary;
        AnnotationGridPointModel::Point m_newBoundary;
    };

    // Command to add or remove a series of boundaries, with undo. Consecutive add/remove pairs for the same point are collapsed.
    class EditBoundaryCommand : public UndoableMacroCommand
    {
    public:
        EditBoundaryCommand(AnnotationGridModel *model, QString commandName) :
            UndoableMacroCommand(commandName), m_model(model) { }
        virtual void addBoundary(const AnnotationGridPointModel::Point &boundary);
        virtual void deleteBoundary(const AnnotationGridPointModel::Point &boundary);
        virtual void moveBoundary(const AnnotationGridPointModel::Point &boundary, sv_frame_t newFrame);
        //Stack an arbitrary other command in the same sequence.
        virtual void addCommand(UndoableCommand *command) { addCommand(command, true); }
        // If any points have been added or deleted, return this command (so the caller can add it to the command history).
        // Otherwise delete the command and return NULL.
        virtual EditBoundaryCommand *finish();
    protected:
        virtual void addCommand(UndoableCommand *command, bool executeFirst);
        AnnotationGridModel *m_model;
    };

protected:
    AnnotationGridModelData *d;
};


#endif // ANNOTATIONGRIDMODEL_H
