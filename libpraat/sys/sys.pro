! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

QT += core gui widgets

CONFIG += staticlib
TARGET = sys

INCLUDEPATH += ../num ../sys ../dwsys ../kar ../external/portaudio ../external/flac ../external/mp3

SOURCES = abcio.cpp complex.cpp \
   melder_ftoa.cpp melder_atof.cpp melder_error.cpp melder_alloc.cpp melder.cpp melder_strings.cpp \
   melder_token.cpp melder_files.cpp melder_audio.cpp melder_audiofiles.cpp \
   melder_debug.cpp melder_sysenv.cpp melder_info.cpp melder_quantity.cpp \
   melder_textencoding.cpp melder_readtext.cpp melder_writetext.cpp melder_console.cpp melder_time.cpp \
   Thing.cpp Data.cpp Simple.cpp Collection.cpp Strings.cpp \
   Graphics.cpp Graphics_linesAndAreas.cpp Graphics_text.cpp Graphics_colour.cpp \
   Graphics_image.cpp Graphics_mouse.cpp Graphics_record.cpp \
   Graphics_utils.cpp Graphics_grey.cpp Graphics_altitude.cpp \
   GraphicsPostscript.cpp Graphics_surface.cpp \
   ManPage.cpp ManPages.cpp Script.cpp machine.cpp \
   GraphicsScreen.cpp Printer.cpp \
   Preferences.cpp site.cpp \
   Picture.cpp Ui.cpp UiFile.cpp UiPause.cpp Editor.cpp DataEditor.cpp HyperPage.cpp Manual.cpp TextEditor.cpp \
   praat.cpp praat_version.cpp praat_actions.cpp praat_menuCommands.cpp praat_picture.cpp sendpraat.c sendsocket.c \
   praat_script.cpp praat_statistics.cpp praat_logo.cpp \
   praat_objectMenus.cpp InfoEditor.cpp ScriptEditor.cpp ButtonEditor.cpp Interpreter.cpp Formula.cpp \
   StringsEditor.cpp DemoEditor.cpp \
   motifEmulator.cpp GuiText.cpp GuiWindow.cpp Gui.cpp GuiObject.cpp GuiDrawingArea.cpp \
   GuiMenu.cpp GuiMenuItem.cpp GuiButton.cpp GuiLabel.cpp GuiCheckButton.cpp GuiRadioButton.cpp \
   GuiDialog.cpp GuiList.cpp GuiFileSelect.cpp GuiScale.cpp GuiScrollBar.cpp GuiScrolledWindow.cpp \
   GuiControl.cpp GuiForm.cpp GuiOptionMenu.cpp GuiProgressBar.cpp GuiShell.cpp GuiThing.cpp

HEADERS += \
    abcio.h \
    abcio_enums.h \
    ButtonEditor.h \
    Collection.h \
    complex.h \
    Data.h \
    DataEditor.h \
    DemoEditor.h \
    Editor.h \
    Editor_enums.h \
    Editor_prefs.h \
    EditorM.h \
    enums.h \
    enums_getText.h \
    enums_getValue.h \
    Formula.h \
    Graphics.h \
    Graphics_enums.h \
    GraphicsP.h \
    Gui.h \
    GuiP.h \
    HyperPage.h \
    HyperPage_prefs.h \
    InfoEditor.h \
    Interpreter.h \
    Interpreter_decl.h \
    machine.h \
    macport_off.h \
    macport_on.h \
    ManPage.h \
    ManPage_enums.h \
    ManPages.h \
    ManPagesM.h \
    Manual.h \
    melder.h \
    melder_enums.h \
    MelderThread.h \
    motifEmulator_resources.h \
    oo.h \
    oo_CAN_WRITE_AS_ENCODING.h \
    oo_COPY.h \
    oo_DESCRIPTION.h \
    oo_DESTROY.h \
    oo_EQUAL.h \
    oo_READ_BINARY.h \
    oo_READ_TEXT.h \
    oo_undef.h \
    oo_WRITE_BINARY.h \
    oo_WRITE_TEXT.h \
    Picture.h \
    praat.h \
    praat_script.h \
    praat_version.h \
    praatlib.h \
    praatP.h \
    Preferences.h \
    prefs.h \
    prefs_copyToInstance.h \
    prefs_define.h \
    prefs_install.h \
    Printer.h \
    Script.h \
    ScriptEditor.h \
    sendpraat.h \
    sendsocket.h \
    Simple.h \
    Simple_def.h \
    site.h \
    Strings_.h \
    Strings_def.h \
    StringsEditor.h \
    TextEditor.h \
    TextEditor_prefs.h \
    Thing.h \
    Ui.h \
    UiP.h \
    UiPause.h \
    winport_off.h \
    winport_on.h
