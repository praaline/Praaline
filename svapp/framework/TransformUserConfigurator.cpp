/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "TransformUserConfigurator.h"

#include "transform/TransformFactory.h"

#include "widgets/PluginParameterDialog.h"

#include "plugin/FeatureExtractionPluginFactory.h"
#include "plugin/RealTimePluginFactory.h"
#include "plugin/RealTimePluginInstance.h"

#include "data/model/DenseTimeValueModel.h"

#include <vamp-hostsdk/Plugin.h>

#include <QStringList>

#include <typeinfo>

static QWidget *parentWidget = 0;

void
TransformUserConfigurator::setParentWidget(QWidget *w)
{
    parentWidget = w;
}

bool
TransformUserConfigurator::getChannelRange(TransformId identifier,
					   Vamp::PluginBase *plugin,
					   int &minChannels, int &maxChannels)
{
    if (plugin && plugin->getType() == "Feature Extraction Plugin") {
	Vamp::Plugin *vp = static_cast<Vamp::Plugin *>(plugin);
	cerr << "TransformUserConfigurator::getChannelRange: is a VP" << endl;
        minChannels = int(vp->getMinChannelCount());
        maxChannels = int(vp->getMaxChannelCount());
        return true;
    } else {
	cerr << "TransformUserConfigurator::getChannelRange: is not a VP" << endl;
        return TransformFactory::getInstance()->
            getTransformChannelRange(identifier, minChannels, maxChannels);
    }
}

bool
TransformUserConfigurator::configure(ModelTransformer::Input &input,
				     Transform &transform,
				     Vamp::PluginBase *plugin,
                                     Model *&inputModel,
				     AudioPlaySource *source,
				     sv_frame_t startFrame,
				     sv_frame_t duration,
				     const QMap<QString, Model *> &modelMap,
				     QStringList candidateModelNames,
				     QString defaultModelName)
{
    bool ok = false;
    QString id = transform.getPluginIdentifier();
    QString output = transform.getOutput();
    QString outputLabel = "";
    QString outputDescription = "";

    bool frequency = false;
    bool effect = false;
    bool generator = false;

    if (!plugin) return false;

    if (FeatureExtractionPluginFactory::instanceFor(id)) {

        Vamp::Plugin *vp = static_cast<Vamp::Plugin *>(plugin);

	frequency = (vp->getInputDomain() == Vamp::Plugin::FrequencyDomain);

	std::vector<Vamp::Plugin::OutputDescriptor> od =
	    vp->getOutputDescriptors();

	cerr << "configure: looking for output: " << output << endl;

	if (od.size() > 1) {
	    for (size_t i = 0; i < od.size(); ++i) {
		if (od[i].identifier == output.toStdString()) {
		    outputLabel = od[i].name.c_str();
		    outputDescription = od[i].description.c_str();
		    break;
		}
	    }
        }

    } else if (RealTimePluginFactory::instanceFor(id)) {

        RealTimePluginFactory *factory = RealTimePluginFactory::instanceFor(id);
        const RealTimePluginDescriptor *desc = factory->getPluginDescriptor(id);

        if (desc->audioInputPortCount > 0 && 
            desc->audioOutputPortCount > 0 &&
            !desc->isSynth) {
            effect = true;
        }

        if (desc->audioInputPortCount == 0) {
            generator = true;
        }

        if (output != "A") {
            int outputNo = output.toInt();
            if (outputNo >= 0 && outputNo < int(desc->controlOutputPortCount)) {
                outputLabel = desc->controlOutputPortNames[outputNo].c_str();
            }
        }

        RealTimePluginInstance *rtp =
            static_cast<RealTimePluginInstance *>(plugin);

        if (effect && source) {
	    cerr << "Setting auditioning effect" << endl;
            source->setAuditioningEffect(rtp);
        }
    }

    int sourceChannels = 1;
    if (dynamic_cast<DenseTimeValueModel *>(inputModel)) {
	sourceChannels = dynamic_cast<DenseTimeValueModel *>(inputModel)
	    ->getChannelCount();
    }

    int minChannels = 1, maxChannels = sourceChannels;
    getChannelRange(transform.getIdentifier(), plugin,
		    minChannels, maxChannels);

    int targetChannels = sourceChannels;
    if (!effect) {
	if (sourceChannels < minChannels) targetChannels = minChannels;
	if (sourceChannels > maxChannels) targetChannels = maxChannels;
    }

    int defaultChannel = -1; //!!! no longer saved! [was context.channel]

    PluginParameterDialog *dialog = new PluginParameterDialog
        (plugin, parentWidget);

    dialog->setMoreInfoUrl(TransformFactory::getInstance()->
			   getTransformInfoUrl(transform.getIdentifier()));

    if (candidateModelNames.size() > 1 && !generator) {
	dialog->setCandidateInputModels(candidateModelNames,
					defaultModelName);
    }

    if (startFrame != 0 || duration != 0) {
	dialog->setShowSelectionOnlyOption(true);
    }

    if (targetChannels > 0) {
	dialog->setChannelArrangement(sourceChannels, targetChannels,
				      defaultChannel);
    }
        
    dialog->setOutputLabel(outputLabel, outputDescription);
        
    dialog->setShowProcessingOptions(true, frequency);

    if (dialog->exec() == QDialog::Accepted) {
	ok = true;
    }

    QString selectedInput = dialog->getInputModel();
    if (selectedInput != "") {
	if (modelMap.contains(selectedInput)) {
	    inputModel = modelMap.value(selectedInput);
	    cerr << "Found selected input \"" << selectedInput << "\" in model map, result is " << inputModel << endl;
	} else {
	    cerr << "Failed to find selected input \"" << selectedInput << "\" in model map" << endl;
	}
    } else {
	cerr << "Selected input empty: \"" << selectedInput << "\"" << endl;
    }
        
    // Write parameters back to transform object
    TransformFactory::getInstance()->
	setParametersFromPlugin(transform, plugin);

    input.setChannel(dialog->getChannel());
        
    //!!! The dialog ought to be taking & returning transform
    //objects and input objects and stuff rather than passing
    //around all this misc stuff, but that's for tomorrow
    //(whenever that may be)

    if (startFrame != 0 || duration != 0) {
	if (dialog->getSelectionOnly()) {
	    transform.setStartTime(RealTime::frame2RealTime
				   (startFrame, inputModel->getSampleRate()));
	    transform.setDuration(RealTime::frame2RealTime
				  (duration, inputModel->getSampleRate()));
	}
    }

    int stepSize = 0, blockSize = 0;
    WindowType windowType = HanningWindow;

    dialog->getProcessingParameters(stepSize,
				    blockSize,
				    windowType);

    transform.setStepSize(stepSize);
    transform.setBlockSize(blockSize);
    transform.setWindowType(windowType);

    delete dialog;

    if (effect && source) {
	source->setAuditioningEffect(0);
    }

    return ok;
}

