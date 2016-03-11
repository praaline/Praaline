
exists(debug.pri) {
	include(./debug.pri)
}

TEMPLATE = lib
CONFIG += warn_on
QT -= gui

TARGET = dataquay

exists(config.pri) {
        include(./config.pri)
}

VERSION=0.9
OBJECTS_DIR = o
MOC_DIR = o

QMAKE_LFLAGS_SHLIB *= $(LDFLAGS)

INCLUDEPATH += dataquay

!debug:DEFINES += NDEBUG

HEADERS += dataquay/BasicStore.h \
           dataquay/Connection.h \
           dataquay/Node.h \
           dataquay/PropertyObject.h \
           dataquay/RDFException.h \
           dataquay/Store.h \
           dataquay/Transaction.h \
           dataquay/TransactionalStore.h \
           dataquay/Triple.h \
           dataquay/Uri.h \
           dataquay/objectmapper/ContainerBuilder.h \
           dataquay/objectmapper/ObjectBuilder.h \
           dataquay/objectmapper/ObjectLoader.h \
           dataquay/objectmapper/ObjectMapper.h \
           dataquay/objectmapper/ObjectMapperDefs.h \
           dataquay/objectmapper/ObjectMapperForwarder.h \
           dataquay/objectmapper/ObjectStorer.h \
           dataquay/objectmapper/TypeMapping.h \
           src/Debug.h \
    dataquay/objectmapper/ObjectMapperExceptions.h
           
SOURCES += src/Connection.cpp \
           src/Node.cpp \
           src/PropertyObject.cpp \
           src/RDFException.cpp \
           src/Store.cpp \
           src/Transaction.cpp \
           src/TransactionalStore.cpp \
           src/Triple.cpp \
           src/Uri.cpp \
           src/backend/BasicStoreRedland.cpp \
           src/backend/BasicStoreSord.cpp \
           src/backend/define-check.cpp \
           src/objectmapper/ContainerBuilder.cpp \
           src/objectmapper/ObjectBuilder.cpp \
           src/objectmapper/ObjectLoader.cpp \
           src/objectmapper/ObjectMapper.cpp \
           src/objectmapper/ObjectMapperForwarder.cpp \
           src/objectmapper/ObjectStorer.cpp \
           src/objectmapper/TypeMapping.cpp \
           src/acsymbols.c

linux* {
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	isEmpty(LIBDIR) {
		LIBDIR = $${PREFIX}/lib
	}
        target.path = $${LIBDIR}
        includes.path = $${PREFIX}/include
        includes.files = dataquay
        pkgconfig.path = $${PREFIX}/lib/pkgconfig
        pkgconfig.files = deploy/dataquay.pc
        pkgconfig.extra = sed -e "'"s.%PREFIX%.$${PREFIX}."'" -e "'"s.%LIBDIR%.$${LIBDIR}."'" -e "'"s.%EXTRALIBS%.$${EXTRALIBS}."'" deploy/dataquay.pc.in > deploy/dataquay.pc
        INSTALLS += target includes pkgconfig
}

exists(../platform-dataquay.pri) {
	include(../platform-dataquay.pri)
}

exists(platform.pri) {
	include(./platform.pri)
}
