TEMPLATE = lib

TARGET = pretexeditormodule

VERSION = 2.0.0
VER_MAJ = 2
VER_MIN = 0
VER_PAT = 0

QT = script
BEQT = core widgets codeeditor

include(../../plugin.pri)

RESOURCES -= $${moduleNameNoHyphen}.qrc
RESOURCES += \
    $${moduleNameNoHyphen}_1.qrc \
    $${moduleNameNoHyphen}_2.qrc \
    $${moduleNameNoHyphen}_3.qrc

HEADERS += \
    modulecomponents.h \
    pretexeditormodule.h \
    pretexeditormoduleplugin.h \
    pretexfiletype.h \
    pretexobject.h \
    pretexsettingstab.h \
    recordingmodule.h

SOURCES += \
    modulecomponents.cpp \
    pretexeditormodule.cpp \
    pretexeditormoduleplugin.cpp \
    pretexfiletype.cpp \
    pretexobject.cpp \
    pretexsettingstab.cpp \
    recordingmodule.cpp
