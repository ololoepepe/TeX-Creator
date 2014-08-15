TEMPLATE = lib

TARGET = keyboardlayouteditormodule

QT = core concurrent gui widgets
BEQT = core widgets codeeditor

include(../../plugin.pri)

HEADERS += \
    keyboardlayouteditormodule.h \
    keyboardlayouteditormoduleplugin.h \
    modulecomponents.h

SOURCES += \
    keyboardlayouteditormodule.cpp \
    keyboardlayouteditormoduleplugin.cpp \
    modulecomponents.cpp
