/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
   
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _MODEL_TRANSFORMER_FACTORY_H_
#define _MODEL_TRANSFORMER_FACTORY_H_

#include "Transform.h"
#include "TransformDescription.h"
#include "FeatureExtractionModelTransformer.h"

#include "ModelTransformer.h"

#include <vamp-hostsdk/PluginBase.h>

#include <QMap>
#include <map>
#include <set>
#include <vector>

class AudioPlaySource;

class ModelTransformerFactory : public QObject
{
    Q_OBJECT

public:
    virtual ~ModelTransformerFactory();

    static ModelTransformerFactory *getInstance();

    class UserConfigurator {
    public:
        virtual bool configure(ModelTransformer::Input &input,
                               Transform &transform,
                               Vamp::PluginBase *plugin,
                               Model *&inputModel,
                               AudioPlaySource *source,
                               sv_frame_t startFrame,
                               sv_frame_t duration,
                               const QMap<QString, Model *> &modelMap,
                               QStringList candidateModelNames,
                               QString defaultModelName) = 0;
    };

    /**
     * Fill out the configuration for the given transform (may include
     * asking the user by calling back on the UserConfigurator).
     * Returns the selected input model and channel if the transform
     * is acceptable, or an input with a null model if the operation
     * should be cancelled.  Audio play source may be used to audition
     * effects plugins, if provided.
     */
    ModelTransformer::Input
    getConfigurationForTransform(Transform &transform,
                                 const std::vector<Model *> &candidateInputModels,
                                 Model *defaultInputModel,
                                 AudioPlaySource *source = 0,
                                 sv_frame_t startFrame = 0,
                                 sv_frame_t duration = 0,
                                 UserConfigurator *configurator = 0);

    class AdditionalModelHandler {
    public:
        virtual ~AdditionalModelHandler() { }

        // Exactly one of these functions will be called
        virtual void moreModelsAvailable(std::vector<Model *> models) = 0;
        virtual void noMoreModelsAvailable() = 0;
    };
    
    /**
     * Return the output model resulting from applying the named
     * transform to the given input model.  The transform may still be
     * working in the background when the model is returned; check the
     * output model's isReady completion status for more details. To
     * cancel a background transform, call abandon() on its model.
     *
     * If the transform is unknown or the input model is not an
     * appropriate type for the given transform, or if some other
     * problem occurs, return 0.  Set message if there is any error or
     * warning to report.
     * 
     * Some transforms may return additional models at the end of
     * processing. (For example, a transform that splits an output
     * into multiple one-per-bin models.) If an additionalModelHandler
     * is provided here, its moreModelsAvailable method will be called
     * when those models become available, and ownership of those
     * models will be transferred to the handler. Otherwise (if the
     * handler is null) any such models will be discarded.
     *
     * The returned model is owned by the caller and must be deleted
     * when no longer needed.
     */
    Model *transform(const Transform &transform,
                     const ModelTransformer::Input &input,
                     QString &message,
                     AdditionalModelHandler *handler = 0);

    /**
     * Return the multiple output models resulting from applying the
     * named transforms to the given input model.  The transforms may
     * differ only in output identifier for the plugin: they must all
     * use the same plugin, parameters, and programs. The plugin will
     * be run once only, but more than one output will be harvested
     * (as appropriate). Models will be returned in the same order as
     * the transforms were given. The plugin may still be working in
     * the background when the model is returned; check the output
     * models' isReady completion statuses for more details. To cancel
     * a background transform, call abandon() on its model.
     *
     * If a transform is unknown or the transforms are insufficiently
     * closely related or the input model is not an appropriate type
     * for the given transform, or if some other problem occurs,
     * return 0.  Set message if there is any error or warning to
     * report.
     *
     * Some transforms may return additional models at the end of
     * processing. (For example, a transform that splits an output
     * into multiple one-per-bin models.) If an additionalModelHandler
     * is provided here, its moreModelsAvailable method will be called
     * when those models become available, and ownership of those
     * models will be transferred to the handler. Otherwise (if the
     * handler is null) any such models will be discarded. Note that
     * calling abandon() on any one of the models returned by
     * transformMultiple is sufficient to cancel all background
     * transform activity associated with these output models.
     *
     * The returned models are owned by the caller and must be deleted
     * when no longer needed.
     */
    std::vector<Model *> transformMultiple(const Transforms &transform,
                                           const ModelTransformer::Input &input,
                                           QString &message,
                                           AdditionalModelHandler *handler = 0);

protected slots:
    void transformerFinished();

    void modelAboutToBeDeleted(Model *);

protected:
    ModelTransformer *createTransformer(const Transforms &transforms,
                                        const ModelTransformer::Input &input);

    typedef std::map<TransformId, QString> TransformerConfigurationMap;
    TransformerConfigurationMap m_lastConfigurations;

    typedef std::set<ModelTransformer *> TransformerSet;
    TransformerSet m_runningTransformers;

    typedef std::map<ModelTransformer *, AdditionalModelHandler *> HandlerMap;
    HandlerMap m_handlers;

    static ModelTransformerFactory *m_instance;
};


#endif
