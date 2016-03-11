! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin no_plugin_name_prefix

TARGET = PraalinePy

PYTHONPATH = C:/Python34

INCLUDEPATH += $${PYTHONPATH}/include
INCLUDEPATH += $${PYTHONPATH}/Lib/site-packages/PyQt5/include

INCLUDEPATH += . .. ../pncore
DEPENDPATH += . .. ../pncore

CONFIG += qt thread warn_on stl rtti exceptions c++11

# Build folder organisation
CONFIG( debug, debug|release ) {
    # debug
    COMPONENTSPATH = build/debug
} else {
    # release
    COMPONENTSPATH = build/release
}

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

DEFINES += _hypot=hypot

# SIP Files are part of the project
SIP_FILES += std_string.sip \
             RealTime.sip \
             PraalinePy.sip

QT_SIP_PATH = c:/python34/sip.exe

defineReplace(sipSourceHandler) {
    SIP_SOURCE_ABS = $${_PRO_FILE_PWD_}/sipOutput/sip${QMAKE_TARGET}$${NO_PREFIX}.cpp
    SIP_SOURCE = $$relative_path( $$SIP_SOURCE_ABS, $${OUT_PWD} )
    return($${SIP_SOURCE})
#    qmFile = $$basename(1)
#    qmFile = $$replace(qmFile, "\\.ts", ".qm")
#    return($${buildDir}translations/$${qmFile})
}

sipSourceBuilder.name = SIP Builder
sipSourceBuilder.input = SIP_FILES
sipSourceBuilder.output_function = sipSourceHandler
sipSourceBuilder.variable_out = SOURCES
sipSourceBuilder.dependency_type = TYPE_C
sipSourceBuilder.CONFIG += target_predeps
sipSourceBuilder.commands = $$QT_SIP_PATH -c $${_PRO_FILE_PWD_}/sipOutput $${_PRO_FILE_PWD_}/${QMAKE_FILE_IN}

#sipSourceBuilder.output = $${_PRO_FILE_PWD_}/sipOutput/sip$${QMAKE_FILE_BASE}cmodule.cpp
#sipSourceBuilder.clean = $${_PRO_FILE_PWD_}/sipOutput/sip$${QMAKE_FILE_BASE}cmodule.cpp

QMAKE_CLEAN += $${_PRO_FILE_PWD_}/sipOutput/sip${QMAKE_FILE_BASE}cmodule.cpp
SOURCES += $${_PRO_FILE_PWD_}/sipOutput/sip${QMAKE_FILE_BASE}cmodule.cpp

QMAKE_EXTRA_COMPILERS += sipSourceBuilder

#LIBS +=  \
#        -L../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX} \
#        -L$${PYTHONPATH}/libs -lpython34 \
#        $$LIBS

#PRE_TARGETDEPS += \
#        ../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.a

