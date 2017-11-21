TEMPLATE = lib

SV_UNIT_PACKAGES = vamp-hostsdk fftw3f
load(../prf/sv.prf)

CONFIG += sv staticlib qt thread warn_on stl rtti exceptions
QT += xml

TARGET = svwidgets

DEPENDPATH += . ..
INCLUDEPATH += . ..
OBJECTS_DIR = tmp_obj
MOC_DIR = tmp_moc

# Input
HEADERS += ActivityLog.h \
           AudioDial.h \
           ClickableLabel.h \
           ColourNameDialog.h \
           CommandHistory.h \
           CSVFormatDialog.h \
           Fader.h \
           InteractiveFileFinder.h \
           IconLoader.h \
           ImageDialog.h \
           ItemEditDialog.h \
           KeyReference.h \
           LabelCounterInputDialog.h \
           LayerTree.h \
           LayerTreeDialog.h \
           LEDButton.h \
           ListInputDialog.h \
           MIDIFileImportDialog.h \
           ModelDataTableDialog.h \
           NotifyingCheckBox.h \
           NotifyingComboBox.h \
           NotifyingPushButton.h \
           NotifyingTabBar.h \
           Panner.h \
           PluginParameterBox.h \
           PluginParameterDialog.h \
           ProgressDialog.h \
           PropertyBox.h \
           PropertyStack.h \
           RangeInputDialog.h \
           SelectableLabel.h \
           SubdividingMenu.h \
           TextAbbrev.h \
           Thumbwheel.h \
           TipDialog.h \
           TransformFinder.h \
           WindowShapePreview.h \
           WindowTypeSelector.h
SOURCES += ActivityLog.cpp \
           AudioDial.cpp \
           ColourNameDialog.cpp \
           CommandHistory.cpp \
           CSVFormatDialog.cpp \
           Fader.cpp \
           InteractiveFileFinder.cpp \
           IconLoader.cpp \
           ImageDialog.cpp \
           ItemEditDialog.cpp \
           KeyReference.cpp \
           LabelCounterInputDialog.cpp \
           LayerTree.cpp \
           LayerTreeDialog.cpp \
           LEDButton.cpp \
           ListInputDialog.cpp \
           MIDIFileImportDialog.cpp \
           ModelDataTableDialog.cpp \
           NotifyingCheckBox.cpp \
           NotifyingComboBox.cpp \
           NotifyingPushButton.cpp \
           NotifyingTabBar.cpp \
           Panner.cpp \
           PluginParameterBox.cpp \
           PluginParameterDialog.cpp \
           ProgressDialog.cpp \
           PropertyBox.cpp \
           PropertyStack.cpp \
           RangeInputDialog.cpp \
           SelectableLabel.cpp \
           SubdividingMenu.cpp \
           TextAbbrev.cpp \
           Thumbwheel.cpp \
           TipDialog.cpp \
           TransformFinder.cpp \
           WindowShapePreview.cpp \
           WindowTypeSelector.cpp
