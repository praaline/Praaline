include(../common.pri)
greaterThan(QT_MAJOR_VERSION, 4): QT *= widgets
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

qtpropertybrowser-uselib:!qtpropertybrowser-buildlib {
    LIBS += -L$$QTPROPERTYBROWSER_LIBDIR -l$$QTPROPERTYBROWSER_LIBNAME
} else {
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
    SOURCES += $$PWD/qtpropertybrowser.cpp \
            $$PWD/qtpropertymanager.cpp \
            $$PWD/qteditorfactory.cpp \
            $$PWD/qtvariantproperty.cpp \
            $$PWD/qttreepropertybrowser.cpp \
            $$PWD/qtbuttonpropertybrowser.cpp \
            $$PWD/qtgroupboxpropertybrowser.cpp \
            $$PWD/qtpropertybrowserutils.cpp
    HEADERS += $$PWD/qtpropertybrowser.h \
            $$PWD/qtpropertymanager.h \
            $$PWD/qteditorfactory.h \
            $$PWD/qtvariantproperty.h \
            $$PWD/qttreepropertybrowser.h \
            $$PWD/qtbuttonpropertybrowser.h \
            $$PWD/qtgroupboxpropertybrowser.h \
            $$PWD/qtpropertybrowserutils_p.h
    RESOURCES += $$PWD/qtpropertybrowser.qrc
}

win32 {
    contains(TEMPLATE, lib):contains(CONFIG, shared):DEFINES += QT_QTPROPERTYBROWSER_EXPORT
    else:qtpropertybrowser-uselib:DEFINES += QT_QTPROPERTYBROWSER_IMPORT
}

HEADERS += \
    $$PWD/QtAbstractEditorFactoryBase \
    $$PWD/QtAbstractPropertyBrowser \
    $$PWD/QtAbstractPropertyManager \
    $$PWD/QtBoolPropertyManager \
    $$PWD/QtBrowserItem \
    $$PWD/QtButtonPropertyBrowser \
    $$PWD/QtCharEditorFactory \
    $$PWD/QtCharPropertyManager \
    $$PWD/QtCheckBoxFactory \
    $$PWD/QtColorEditorFactory \
    $$PWD/QtColorPropertyManager \
    $$PWD/QtCursorEditorFactory \
    $$PWD/QtCursorPropertyManager \
    $$PWD/QtDateEditFactory \
    $$PWD/QtDatePropertyManager \
    $$PWD/QtDateTimeEditFactory \
    $$PWD/QtDateTimePropertyManager \
    $$PWD/QtDoublePropertyManager \
    $$PWD/QtDoubleSpinBoxFactory \
    $$PWD/QtEnumEditorFactory \
    $$PWD/QtEnumPropertyManager \
    $$PWD/QtFlagPropertyManager \
    $$PWD/QtFontEditorFactory \
    $$PWD/QtFontPropertyManager \
    $$PWD/QtGroupBoxPropertyBrowser \
    $$PWD/QtGroupPropertyManager \
    $$PWD/QtIntPropertyManager \
    $$PWD/QtKeySequenceEditorFactory \
    $$PWD/QtKeySequencePropertyManager \
    $$PWD/QtLineEditFactory \
    $$PWD/QtLocalePropertyManager \
    $$PWD/QtPointFPropertyManager \
    $$PWD/QtPointPropertyManager \
    $$PWD/QtProperty \
    $$PWD/QtRectFPropertyManager \
    $$PWD/QtRectPropertyManager \
    $$PWD/QtScrollBarFactory \
    $$PWD/QtSizeFPropertyManager \
    $$PWD/QtSizePolicyPropertyManager \
    $$PWD/QtSizePropertyManager \
    $$PWD/QtSliderFactory \
    $$PWD/QtSpinBoxFactory \
    $$PWD/QtStringPropertyManager \
    $$PWD/QtTimeEditFactory \
    $$PWD/QtTimePropertyManager \
    $$PWD/QtTreePropertyBrowser \
    $$PWD/QtVariantEditorFactory \
    $$PWD/QtVariantProperty \
    $$PWD/QtVariantPropertyManager
