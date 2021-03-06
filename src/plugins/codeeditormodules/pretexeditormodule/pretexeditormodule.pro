TEMPLATE = lib

TARGET = pretexeditormodule

VERSION = 1.1.3
VER_MAJ = 1
VER_MIN = 1
VER_PAT = 3

QT = core concurrent gui widgets
BEQT = core widgets codeeditor

include(../../plugin.pri)

RESOURCES -= $${moduleNameNoHyphen}.qrc
RESOURCES += \
    $${moduleNameNoHyphen}_1.qrc \
    $${moduleNameNoHyphen}_2.qrc \
    $${moduleNameNoHyphen}_3.qrc

HEADERS += \
    booleanfunction.h \
    executioncontext.h \
    executionmodule.h \
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
    pretexfiletype.h  \
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
    executioncontext.cpp \
    executionmodule.cpp \
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
    pretexfiletype.cpp \
    pretexfunction.cpp \
    pretexsettingstab.cpp \
    pretexvariant.cpp \
    recordingmodule.cpp \
    specialfunction.cpp \
    token.cpp \
    tokendata.cpp \
    trigonometricfunction.cpp
