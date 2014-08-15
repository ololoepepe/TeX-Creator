TEMPLATE = lib

TARGET = pretexeditormodule

QT = core concurrent gui widgets
BEQT = core widgets codeeditor

include(../../plugin.pri)

HEADERS += \
    booleanfunction.h \
    executionmodule.h \
    executionstack.h \
    generalfunction.h \
    global.h \
    iofunction.h \
    lexicalanalyzer.h \
    mathfunction.h \
    modulecomponents.h \
    parser.h \
    pretexarray.h \
    pretexbuiltinfunction.h \
    pretexeditormodule.h \
    pretexeditormoduleplugin.h \
    pretexfunction.h \
    pretexsettingstab.h \
    pretexvariant.h \
    recordingmodule.h \
    specialfunction.h \
    tokendata.h \
    token.h \
    trigonometricfunction.h

SOURCES += \
    booleanfunction.cpp \
    executionmodule.cpp \
    executionstack.cpp \
    generalfunction.cpp \
    global.cpp \
    iofunction.cpp \
    lexicalanalyzer.cpp \
    mathfunction.cpp \
    modulecomponents.cpp \
    parser.cpp \
    pretexarray.cpp \
    pretexbuiltinfunction.cpp \
    pretexeditormodule.cpp \
    pretexeditormoduleplugin.cpp \
    pretexfunction.cpp \
    pretexsettingstab.cpp \
    pretexvariant.cpp \
    recordingmodule.cpp \
    specialfunction.cpp \
    token.cpp \
    tokendata.cpp \
    trigonometricfunction.cpp
