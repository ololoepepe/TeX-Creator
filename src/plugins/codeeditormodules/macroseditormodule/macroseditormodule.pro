TEMPLATE = lib

TARGET = macroseditormodule

QT = core concurrent gui widgets
BEQT = core widgets codeeditor

include(../../plugin.pri)

HEADERS += \
    macro.h \
    macrocommand.h \
    macrocommandargument.h \
    macroexecutionstack.h \
    macroseditormodule.h \
    macroseditormoduleplugin.h \
    macrossettingstab.h \
    unmacrocommand.h \
    global.h

SOURCES += \
    macro.cpp \
    macrocommand.cpp \
    macrocommandargument.cpp \
    macroexecutionstack.cpp \
    macroseditormodule.cpp \
    macroseditormoduleplugin.cpp \
    macrossettingstab.cpp \
    unmacrocommand.cpp \
    global.cpp
