TEMPLATE = lib

TARGET = keyboardlayouteditormodule

VERSION = 0.1.1
VER_MAJ = 0
VER_MIN = 1
VER_PAT = 1

QT = core concurrent gui widgets
BEQT = core widgets codeeditor

include(../../plugin.pri)

HEADERS += \
    keyboardlayouteditormodule.h \
    keyboardlayouteditormoduleplugin.h \
    keyboardlayoutmap.h \
    modulecomponents.h

SOURCES += \
    keyboardlayouteditormodule.cpp \
    keyboardlayouteditormoduleplugin.cpp \
    keyboardlayoutmap.cpp \
    modulecomponents.cpp
