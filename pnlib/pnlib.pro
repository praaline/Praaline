# Praaline
# (c) George Christodoulides 2012-2014

! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib qt thread warn_on stl rtti exceptions c++11

QT += xml sql gui
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

TARGET = pnlib

INCLUDEPATH += . .. ../pncore
DEPENDPATH += . .. ../pncore

# Praaline core has a dependency on VAMP SDK (only for RealTime conversion)
win32-g++ {
    INCLUDEPATH += ../sv-dependency-builds/win32-mingw/include
    LIBS += -L../sv-dependency-builds/win32-mingw/lib
}
win32-msvc* {
    INCLUDEPATH += ../sv-dependency-builds/win32-msvc/include
    LIBS += -L../sv-dependency-builds/win32-msvc/lib
}
macx* {
    INCLUDEPATH += ../sv-dependency-builds/osx/include
    LIBS += -L../sv-dependency-builds/osx/lib
}

HEADERS += \ 
    soundinfo.h \
    audiosegmenter.h \
    diff/diff.h \
    diff/diff3.h \
    diff/dtl.h \
    diff/functors.h \
    diff/lcs.h \
    diff/sequence.h \
    diff/ses.h \
    diff/variables.h \
    diff/diffintervals.h

SOURCES += \ 
    soundinfo.cpp \
    audiosegmenter.cpp \
    diff/diffintervals.cpp
