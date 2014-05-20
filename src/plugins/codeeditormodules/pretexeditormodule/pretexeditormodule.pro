TEMPLATE = lib

TARGET = pretexeditormodule

QT = core concurrent gui widgets
BEQT = core widgets codeeditor

include(../../plugin.pri)

HEADERS += \
    executionstack.h \
    pretexeditormodule.h \
    pretexeditormoduleplugin.h \
    macrossettingstab.h \
    global.h \
    tokendata.h \
    token.h \
    parser.h \
    lexicalanalyzer.h \
    executionmodule.h \
    modulecomponents.h \
    pretexvariant.h \
    pretexarray.h \
    pretexfunction.h \
    pretexbuiltinfunction.h \
    mathfunction.h \
    booleanfunction.h \
    iofunction.h \
    specialfunction.h \
    generalfunction.h \
    trigonometricfunction.h \
    recordingmodule.h

SOURCES += \
    executionstack.cpp \
    pretexeditormodule.cpp \
    pretexeditormoduleplugin.cpp \
    macrossettingstab.cpp \
    global.cpp \
    tokendata.cpp \
    token.cpp \
    parser.cpp \
    lexicalanalyzer.cpp \
    executionmodule.cpp \
    modulecomponents.cpp \
    pretexvariant.cpp \
    pretexarray.cpp \
    pretexfunction.cpp \
    pretexbuiltinfunction.cpp \
    mathfunction.cpp \
    booleanfunction.cpp \
    iofunction.cpp \
    specialfunction.cpp \
    generalfunction.cpp \
    trigonometricfunction.cpp \
    recordingmodule.cpp
