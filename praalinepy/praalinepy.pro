! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

# Build folder organisation
CONFIG( debug, debug|release ) {
    # debug
    COMPONENTSPATH = build/debug
} else {
    # release
    COMPONENTSPATH = build/release
}

TEMPLATE    = lib
TARGET      = PraalinePy

SIP_COMMAND = sip
SIP_OUTPUT_DIR = sipOutput

defineReplace(sipSourceHandler) {
        sipFile = $$basename(1)
        sipFile = $$replace(sipFile, "\\.sip", ".cpp")
        return($${buildDir}$${SIP_OUTPUT_DIR}/sipPraalinePy$${sipFile})
}

defineReplace(sipModuleHandler) {
        moduleFile = $$basename(1)
        moduleFile = $$replace(moduleFile, "\\.sip", "")
        return($${buildDir}$${SIP_OUTPUT_DIR}/sip$${moduleFile}cmodule.cpp)
}

sipSourceBuilder.name = SIP Builder
sipSourceBuilder.input = SIP_SOURCES
sipSourceBuilder.output_function = sipSourceHandler
sipSourceBuilder.commands = echo ${QMAKE_FILE_IN}
sipSourceBuilder.variable_out = SOURCES
sipSourceBuilder.dependency_type = TYPE_C
sipSourceBuilder.CONFIG += target_predeps

sipModuleBuilder.name = SIP Module Builder
sipModuleBuilder.input = SIP_MODULES
sipModuleBuilder.output_function = sipModuleHandler
sipModuleBuilder.commands = $${SIP_COMMAND} -c $${SIP_OUTPUT_DIR} ${QMAKE_FILE_IN}
sipModuleBuilder.variable_out = SOURCES
sipModuleBuilder.dependency_type = TYPE_C
sipModuleBuilder.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += sipSourceBuilder sipModuleBuilder

INCLUDEPATH += /usr/include/python3.4
INCLUDEPATH += ../pncore
CONFIG += dll qt thread warn_on stl rtti exceptions c++11

LIBS +=  \
        -L../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

SIP_MODULES += PraalinePy.sip
SIP_SOURCES += stdstring.sip \
               RealTime.sip
