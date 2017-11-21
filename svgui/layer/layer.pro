TEMPLATE = lib

SV_UNIT_PACKAGES = fftw3f
load(../prf/sv.prf)

CONFIG += sv staticlib qt thread warn_on stl rtti exceptions
QT += xml

TARGET = svlayer

DEPENDPATH += . ..
INCLUDEPATH += . ..
OBJECTS_DIR = tmp_obj
MOC_DIR = tmp_moc

# Input
HEADERS += Colour3DPlotLayer.h \
	   ColourDatabase.h \
	   ColourMapper.h \
           ImageLayer.h \
           ImageRegionFinder.h \
           Layer.h \
           LayerFactory.h \
           NoteLayer.h \
           PaintAssistant.h \
           RegionLayer.h \
           SingleColourLayer.h \
           SliceableLayer.h \
           SliceLayer.h \
           SpectrogramLayer.h \
           SpectrumLayer.h \
           TextLayer.h \
           TimeInstantLayer.h \
           TimeRulerLayer.h \
           TimeValueLayer.h \
           WaveformLayer.h
SOURCES += Colour3DPlotLayer.cpp \
	   ColourDatabase.cpp \
	   ColourMapper.cpp \
           ImageLayer.cpp \
           ImageRegionFinder.cpp \
           Layer.cpp \
           LayerFactory.cpp \
           NoteLayer.cpp \
           PaintAssistant.cpp \
           RegionLayer.cpp \
           SingleColourLayer.cpp \
           SliceLayer.cpp \
           SpectrogramLayer.cpp \
           SpectrumLayer.cpp \
           TextLayer.cpp \
           TimeInstantLayer.cpp \
           TimeRulerLayer.cpp \
           TimeValueLayer.cpp \
           WaveformLayer.cpp
