#include "SVLayersInitialiser.h"

#include "LayerFactory.h"

#include "Colour3DPlotLayer.h"
#include "FlexiNoteLayer.h"
#include "ImageLayer.h"
#include "NoteLayer.h"
#include "RegionLayer.h"
#include "SliceLayer.h"
#include "SpectrogramLayer.h"
#include "SpectrumLayer.h"
#include "TextLayer.h"
#include "TimeInstantLayer.h"
#include "TimeRulerLayer.h"
#include "TimeValueLayer.h"
#include "WaveformLayer.h"

void SVLayersInitialiser::initialise()
{
    LayerFactory &factory = LayerFactory::getInstance();
    factory.registerLayerType("Waveform", LayerFactory::LayerConfiguration(
                                  "waveform", "Waveform", "waveform", false,
                                  QStringList() << "RangeSummarisableTimeValueModel",
                                  [](LayerFactory::LayerType) -> Layer * { return new WaveformLayer(); },
                                  [](Model *) -> Model * { return 0; } ));
    factory.registerLayerType("TimeRuler", LayerFactory::LayerConfiguration(
                                  "timeruler", "Ruler", "timeruler", false,
                                  QStringList(),
                                  [](LayerFactory::LayerType) -> Layer * { return new TimeRulerLayer(); },
                                  [](Model *) -> Model * { return 0; } ));
    factory.registerLayerType("TimeInstants", LayerFactory::LayerConfiguration(
                                  "timeinstants", "Time Instants", "instants", true,
                                  QStringList() << "SparseOneDimensionalModel",
                                  [](LayerFactory::LayerType) -> Layer * { return new TimeInstantLayer(); },
                                  [](Model *baseModel) -> Model * { return TimeInstantLayer::createEmptyModel(baseModel); } ));
    factory.registerLayerType("TimeValues", LayerFactory::LayerConfiguration(
                                  "timevalues", "Time Values", "values", true,
                                  QStringList() << "SparseTimeValueModel",
                                  [](LayerFactory::LayerType type) -> Layer * { Q_UNUSED(type); return new TimeValueLayer(); },
                                  [](Model *baseModel) -> Model * { return TimeValueLayer::createEmptyModel(baseModel); } ));
    factory.registerLayerType("Spectrum", LayerFactory::LayerConfiguration(
                                  "spectrum", "Spectrum", "spectrum", false,
                                  QStringList() << "DenseTimeValueModel",
                                  [](LayerFactory::LayerType type) -> Layer * { Q_UNUSED(type); return new SpectrumLayer(); },
                                  [](Model *) -> Model * { return 0; } ));
    factory.registerLayerType("Spectrogram", LayerFactory::LayerConfiguration(
                                  "spectrogram", "Spectrogram", "spectrogram", false,
                                  QStringList() << "DenseTimeValueModel",
                                  [](LayerFactory::LayerType type) -> Layer * { Q_UNUSED(type); return new SpectrogramLayer(SpectrogramLayer::FullRangeDb); },
                                  [](Model *) -> Model * { return 0; } ));
    factory.registerLayerType("MelodicRangeSpectrogram", LayerFactory::LayerConfiguration(
                                  "melodicrange", "Spectrogram", "spectrogram", false,
                                  QStringList() << "DenseTimeValueModel",
                                  [](LayerFactory::LayerType type) -> Layer * { Q_UNUSED(type); return new SpectrogramLayer(SpectrogramLayer::MelodicRange); },
                                  [](Model *) -> Model * { return 0; } ));
    factory.registerLayerType("PeakFrequencySpectrogram", LayerFactory::LayerConfiguration(
                                  "peakfrequency", "Spectrogram", "spectrogram", false,
                                  QStringList() << "DenseTimeValueModel",
                                  [](LayerFactory::LayerType type) -> Layer * { Q_UNUSED(type); return new SpectrogramLayer(SpectrogramLayer::MelodicPeaks); },
                                  [](Model *) -> Model * { return 0; } ));
    factory.registerLayerType("Text", LayerFactory::LayerConfiguration(
                                  "text", "Free Text", "text", true,
                                  QStringList() << "TextModel",
                                  [](LayerFactory::LayerType type) -> Layer * { Q_UNUSED(type); return new TextLayer(); },
                                  [](Model *baseModel) -> Model * { return TextLayer::createEmptyModel(baseModel); } ));
    factory.registerLayerType("Image", LayerFactory::LayerConfiguration(
                                  "image", "Images", "images", true,
                                  QStringList() << "ImageModel",
                                  [](LayerFactory::LayerType type) -> Layer * { Q_UNUSED(type); return new ImageLayer(); },
                                  [](Model *baseModel) -> Model * { return ImageLayer::createEmptyModel(baseModel); } ));
    factory.registerLayerType("Regions", LayerFactory::LayerConfiguration(
                                  "regions", "Regions", "regions", true,
                                  QStringList() << "RegionModel",
                                  [](LayerFactory::LayerType type) -> Layer * { Q_UNUSED(type); return new RegionLayer(); },
                                  [](Model *baseModel) -> Model * { return RegionLayer::createEmptyModel(baseModel); } ));
    factory.registerLayerType("Colour3DPlot", LayerFactory::LayerConfiguration(
                                  "colour3dplot", "Colour 3D Plot", "colour3d", false,
                                  QStringList() << "DenseThreeDimensionalModel",
                                  [](LayerFactory::LayerType type) -> Layer * { Q_UNUSED(type); return new Colour3DPlotLayer(); },
                                  [](Model *) -> Model * { return 0; } ));
    factory.registerLayerType("Notes", LayerFactory::LayerConfiguration(
                                  "notes", "Notes", "notes", true,
                                  QStringList() << "NoteModel",
                                  [](LayerFactory::LayerType type) -> Layer * { Q_UNUSED(type); return new NoteLayer(); },
                                  [](Model *baseModel) -> Model * { return NoteLayer::createEmptyModel(baseModel); } ));
    factory.registerLayerType("FlexiNotes", LayerFactory::LayerConfiguration(
                                  "flexinotes", "Flexible Notes", "flexinotes", true,
                                  QStringList() << "FlexiNoteModel",
                                  [](LayerFactory::LayerType type) -> Layer * { Q_UNUSED(type); return new FlexiNoteLayer(); },
                                  [](Model *baseModel) -> Model * { return FlexiNoteLayer::createEmptyModel(baseModel); } ));
    factory.registerLayerType("Slice", LayerFactory::LayerConfiguration(
                                  "slice", "Time Slice", "spectrum", false,
                                  QStringList() << "DenseThreeDimensionalModel",
                                  [](LayerFactory::LayerType type) -> Layer * { Q_UNUSED(type); return new SliceLayer(); },
                                  [](Model *) -> Model * { return 0; } ));
}



