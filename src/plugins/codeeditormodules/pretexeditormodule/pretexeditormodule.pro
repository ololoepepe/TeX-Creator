TEMPLATE = lib

TARGET = pretexeditormodule

QT = core concurrent gui widgets
BEQT = core widgets codeeditor

include(../../plugin.pri)

HEADERS += \
    macro.h \
    macrocommand.h \
    macrocommandargument.h \
    executionstack.h \
    pretexeditormodule.h \
    pretexeditormoduleplugin.h \
    macrossettingstab.h \
    unmacrocommand.h \
    global.h \
    generalmacrocommands.h \
    execmacrocommands.h \
    varfuncmacrocommands.h \
    documentmacrocommands.h \
    loopmacrocommand.h \
    tokendata.h \
    token.h \
    parser.h \
    lexicalanalyzer.h \
    executionmodule.h \
    modulecomponents.h \
    pretexvariant.h \
    pretexarray.h \
    pretexfunction.h \
    pretexstatement.h \
    pretexbuiltinfunction.h \
    mathfunction.h \
    booleanfunction.h \
    iofunction.h

SOURCES += \
    macro.cpp \
    macrocommand.cpp \
    macrocommandargument.cpp \
    executionstack.cpp \
    pretexeditormodule.cpp \
    pretexeditormoduleplugin.cpp \
    macrossettingstab.cpp \
    unmacrocommand.cpp \
    global.cpp \
    generalmacrocommands.cpp \
    execmacrocommands.cpp \
    varfuncmacrocommands.cpp \
    documentmacrocommands.cpp \
    loopmacrocommand.cpp \
    tokendata.cpp \
    token.cpp \
    parser.cpp \
    lexicalanalyzer.cpp \
    executionmodule.cpp \
    modulecomponents.cpp \
    pretexvariant.cpp \
    pretexarray.cpp \
    pretexfunction.cpp \
    pretexstatement.cpp \
    pretexbuiltinfunction.cpp \
    mathfunction.cpp \
    booleanfunction.cpp \
    iofunction.cpp
