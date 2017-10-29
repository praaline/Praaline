#include "PraalineLayersInitialiser.h"

#include "svgui/layer/LayerFactory.h"
#include "AnnotationGridLayer.h"
#include "ProsogramLayer.h"
#include "MovingAverageLayer.h"
#include "RegionAverageLayer.h"

void PraalineLayersInitialiser::initialise()
{
    LayerFactory &factory = LayerFactory::getInstance();
    factory.registerLayerType("AnnotationGrid", LayerFactory::LayerConfiguration(
                                  "annotationgrid", "Annotation Grid", "annotationgrid", false,
                                  QStringList() << "AnnotationGridModel",
                                  [](LayerFactory::LayerType) -> Layer * { return new AnnotationGridLayer(); },
                                  [](Model *) -> Model * { return 0; } ));
    factory.registerLayerType("Prosogram", LayerFactory::LayerConfiguration(
                                  "prosogram", "Prosogram", "prosogram", false,
                                  QStringList() << "ProsogramModel",
                                  [](LayerFactory::LayerType) -> Layer * { return new ProsogramLayer(); },
                                  [](Model *) -> Model * { return 0; } ));
    factory.registerLayerType("MovingAverage", LayerFactory::LayerConfiguration(
                                  "movingaverage", "Moving Average", "movingaverage", false,
                                  QStringList() << "MovingAverageModel",
                                  [](LayerFactory::LayerType) -> Layer * { return new MovingAverageLayer(); },
                                  [](Model *) -> Model * { return 0; } ));
    factory.registerLayerType("RegionAverage", LayerFactory::LayerConfiguration(
                                  "regionaverage", "Region Average", "regionaverage", false,
                                  QStringList() << "RegionAverageModel",
                                  [](LayerFactory::LayerType) -> Layer * { return new RegionAverageLayer(); },
                                  [](Model *) -> Model * { return 0; } ));
}

