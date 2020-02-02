/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _MODEL_H_
#define _MODEL_H_

#include <vector>
#include <string>
#include <QObject>

#include "PraalineCore/Base/BaseTypes.h"
#include "base/XmlExportable.h"
#include "base/Playable.h"
#include "base/DataExportOptions.h"

typedef std::vector<float> SampleBlock;

class ZoomConstraint;
class AlignmentModel;

/** 
 * Model is the base class for all data models that represent any sort
 * of data on a time scale based on an audio frame rate.
 */

class Model : public QObject,
        public XmlExportable,
        public Playable
{
    Q_OBJECT

public:
    virtual ~Model();

    /**
     * Return true if the model was constructed successfully.  Classes
     * that refer to the model should always test this before use.
     */
    virtual bool isOK() const = 0;

    /**
     * Return the first audio frame spanned by the model.
     */
    virtual sv_frame_t getStartFrame() const = 0;

    /**
     * Return the last audio frame spanned by the model.
     */
    virtual sv_frame_t getEndFrame() const = 0;

    /**
     * Return the frame rate in frames per second.
     */
    virtual sv_samplerate_t getSampleRate() const = 0;

    /**
     * Return the frame rate of the underlying material, if the model
     * itself has already been resampled.
     */
    virtual sv_samplerate_t getNativeRate() const { return getSampleRate(); }

    /**
     * Return the "work title" of the model, if known.
     */
    virtual QString getTitle() const;

    /**
     * Return the "artist" or "maker" of the model, if known.
     */
    virtual QString getMaker() const;

    /**
     * Return the location of the data in this model (e.g. source
     * URL).  This should not normally be returned for editable models
     * that have been edited.
     */
    virtual QString getLocation() const;

    /**
     * Return the type of the model.  For display purposes only.
     */
    virtual QString getTypeName() const = 0;
    /**
     * Return the type of the model.
     * Added for the dynamic Layer/Model factory system.
     */
    virtual std::string getType() const = 0;

    /**
     * Mark the model as abandoning. This means that the application
     * no longer needs it, so it can stop doing any background
     * calculations it may be involved in. Note that as far as the
     * model API is concerned, this does nothing more than tell the
     * model to return true from isAbandoning().  The actual response
     * to this will depend on the model's context -- it's possible
     * nothing at all will change.
     */
    virtual void abandon() {
        m_abandoning = true;
    }

    /**
     * Query whether the model has been marked as abandoning.
     */
    virtual bool isAbandoning() const {
        return m_abandoning;
    }

    /**
     * Return true if the model has finished loading or calculating
     * all its data, for a model that is capable of calculating in a
     * background thread.  The default implementation is appropriate
     * for a thread that does not background any work but carries out
     * all its calculation from the constructor or accessors.
     *
     * If "completion" is non-NULL, this function should return
     * through it an estimated percentage value showing how far
     * through the background operation it thinks it is (for progress
     * reporting).  If it has no way to calculate progress, it may
     * return the special value COMPLETION_UNKNOWN.
     */
    virtual bool isReady(int *completion = 0) const {
        bool ok = isOK();
        if (completion) *completion = (ok ? 100 : 0);
        return ok;
    }
    static const int COMPLETION_UNKNOWN;

    /**
     * If this model imposes a zoom constraint, i.e. some limit to the
     * set of resolutions at which its data can meaningfully be
     * displayed, then return it.
     */
    virtual const ZoomConstraint *getZoomConstraint() const {
        return 0;
    }

    /**
     * If this model was derived from another, return the model it was
     * derived from.  The assumption is that the source model's
     * alignment will also apply to this model, unless some other
     * property (such as a specific alignment model set on this model)
     * indicates otherwise.
     */
    virtual Model *getSourceModel() const {
        return m_sourceModel;
    }

    /**
     * Set the source model for this model.
     */
    virtual void setSourceModel(Model *model);

    /**
     * Specify an aligment between this model's timeline and that of a
     * reference model.  The alignment model records both the
     * reference and the alignment.  This model takes ownership of the
     * alignment model.
     */
    virtual void setAlignment(AlignmentModel *alignment);

    /**
     * Retrieve the alignment model for this model.  This is not a
     * generally useful function, as the alignment you really want may
     * be performed by the source model instead.  You should normally
     * use getAlignmentReference, alignToReference and
     * alignFromReference instead of this.  The main intended
     * application for this function is in streaming out alignments to
     * the session file.
     */
    virtual const AlignmentModel *getAlignment() const;

    /**
     * Return the reference model for the current alignment timeline,
     * if any.
     */
    virtual const Model *getAlignmentReference() const;

    /**
     * Return the frame number of the reference model that corresponds
     * to the given frame number in this model.
     */
    virtual sv_frame_t alignToReference(sv_frame_t frame) const;

    /**
     * Return the frame number in this model that corresponds to the
     * given frame number of the reference model.
     */
    virtual sv_frame_t alignFromReference(sv_frame_t referenceFrame) const;

    /**
     * Return the completion percentage for the alignment model: 100
     * if there is no alignment model or it has been entirely
     * calculated, or less than 100 if it is still being calculated.
     */
    virtual int getAlignmentCompletion() const;

    /**
     * Set the event, feature, or signal type URI for the features
     * contained in this model, according to the Audio Features RDF
     * ontology.
     */
    void setRDFTypeURI(QString uri) { m_typeUri = uri; }

    /**
     * Retrieve the event, feature, or signal type URI for the
     * features contained in this model, if previously set with
     * setRDFTypeURI.
     */
    QString getRDFTypeURI() const { return m_typeUri; }

    virtual void toXml(QTextStream &stream,
                       QString indent = "",
                       QString extraAttributes = "") const;

    virtual QString toDelimitedDataString(QString delimiter) const {
        return toDelimitedDataStringSubset(delimiter, getStartFrame(), getEndFrame());
    }
    virtual QString toDelimitedDataStringWithOptions(QString delimiter, DataExportOptions opts) const {
        return toDelimitedDataStringSubsetWithOptions(delimiter, opts, getStartFrame(), getEndFrame());
    }
    virtual QString toDelimitedDataStringSubset(QString, sv_frame_t /* f0 */, sv_frame_t /* f1 */) const {
        return "";
    }
    virtual QString toDelimitedDataStringSubsetWithOptions(QString delimiter, DataExportOptions, sv_frame_t f0, sv_frame_t f1) const {
        // Default implementation supports no options
        return toDelimitedDataStringSubset(delimiter, f0, f1);
    }

public slots:
    void aboutToDelete();
    void sourceModelAboutToBeDeleted();

signals:
    /**
     * Emitted when a model has been edited (or more data retrieved
     * from cache, in the case of a cached model that generates slowly)
     */
    void modelChanged();

    /**
     * Emitted when a model has been edited (or more data retrieved
     * from cache, in the case of a cached model that generates slowly)
     */
    void modelChangedWithin(sv_frame_t startFrame, sv_frame_t endFrame);

    /**
     * Emitted when some internal processing has advanced a stage, but
     * the model has not changed externally.  Views should respond by
     * updating any progress meters or other monitoring, but not
     * refreshing the actual view.
     */
    void completionChanged();

    /**
     * Emitted when internal processing is complete (i.e. when
     * isReady() would return true, with completion at 100).
     */
    void ready();

    /**
     * Emitted when the completion percentage changes for the
     * calculation of this model's alignment model.
     */
    void alignmentCompletionChanged();

    /**
     * Emitted when something notifies this model (through calling
     * aboutToDelete() that it is about to delete it.  Note that this
     * depends on an external agent such as a Document object or
     * owning model telling the model that it is about to delete it;
     * there is nothing in the model to guarantee that this signal
     * will be emitted before the actual deletion.
     */
    void aboutToBeDeleted();

protected:
    Model() :
        m_sourceModel(0),
        m_alignment(0),
        m_abandoning(false),
        m_aboutToDelete(false) { }

    // Not provided.
    Model(const Model &);
    Model &operator=(const Model &);

    Model *m_sourceModel;
    AlignmentModel *m_alignment;
    QString m_typeUri;
    bool m_abandoning;
    bool m_aboutToDelete;
};

#endif
