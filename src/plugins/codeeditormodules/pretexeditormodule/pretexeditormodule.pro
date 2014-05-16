TEMPLATE = lib

TARGET = pretexeditormodule

QT = core concurrent gui widgets
BEQT = core widgets codeeditor

include(../../plugin.pri)

HEADERS += \
    macro.h \
    macrocommand.h \
    macrocommandargument.h \
    macroexecutionstack.h \
    pretexeditormodule.h \
    pretexeditormoduleplugin.h \
    macrossettingstab.h \
    unmacrocommand.h \
    global.h \
    generalmacrocommands.h \
    binmacrocommands.h \
    execmacrocommands.h \
    varfuncmacrocommands.h \
    documentmacrocommands.h \
    loopmacrocommand.h \
    tokendata.h \
    token.h \
    parser.h \
    lexicalanalyzer.h \
    executionmodule.h \
    modulecomponents.h

SOURCES += \
    macro.cpp \
    macrocommand.cpp \
    macrocommandargument.cpp \
    macroexecutionstack.cpp \
    pretexeditormodule.cpp \
    pretexeditormoduleplugin.cpp \
    macrossettingstab.cpp \
    unmacrocommand.cpp \
    global.cpp \
    generalmacrocommands.cpp \
    binmacrocommands.cpp \
    execmacrocommands.cpp \
    varfuncmacrocommands.cpp \
    documentmacrocommands.cpp \
    loopmacrocommand.cpp \
    tokendata.cpp \
    token.cpp \
    parser.cpp \
    lexicalanalyzer.cpp \
    executionmodule.cpp \
    modulecomponents.cpp
