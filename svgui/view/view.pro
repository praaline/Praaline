TEMPLATE = lib

SV_UNIT_PACKAGES =
load(../prf/sv.prf)

CONFIG += sv staticlib qt thread warn_on stl rtti exceptions
QT += xml

TARGET = svview

DEPENDPATH += . ..
INCLUDEPATH += . ..
OBJECTS_DIR = tmp_obj
MOC_DIR = tmp_moc

# Input
HEADERS += Overview.h \
           Pane.h \
           PaneStack.h \
           View.h \
           ViewManager.h
SOURCES += Overview.cpp \
           Pane.cpp \
           PaneStack.cpp \
           View.cpp \
           ViewManager.cpp
