TEMPLATE = lib

TARGET = keyboardlayouteditormodule

VERSION = 0.1.0
VER_MAJ = 0
VER_MIN = 1
VER_PAT = 0

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
