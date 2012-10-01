QT += \
    network \
    xml

CONFIG += release

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/consolewidget.cpp \
    src/applicationserver.cpp \
    src/symbolswidget.cpp

HEADERS += \
    src/mainwindow.h \
    src/consolewidget.h \
    src/applicationserver.h \
    src/symbolswidget.h

RESOURCES += \
    tex-creator.qrc

TRANSLATIONS += \
    translations/tex-creator_ru.ts

RC_FILE = win.rc

unix {
LIBS += -lbeqtcore -lbeqtgui
INCLUDEPATH += "/usr/include/beqt"
}
win32 {
LIBS += -L"$$(systemdrive)/BeQt/lib" -lbeqtcore1 -lbeqtgui1
INCLUDEPATH += "$$(systemdrive)/BeQt/include"
}

builddir = .build

MOC_DIR = $$builddir
OBJECTS_DIR = $$builddir
RCC_DIR = $$builddir

unix {
### Target ###
target.path = /usr/lib/tex-creator
INSTALLS = target
### Docs ###
docs.files = doc/*
docs.path = /usr/share/doc/tex-creator
INSTALLS += docs
### Translations ###
trans.files = translations/*.qm
trans.path = /usr/share/tex-creator/translations
INSTALLS += trans
### KLM ###
klm.files = klm/*
klm.path = /usr/share/tex-creator/klm
INSTALLS += klm
### Symbols ###
symbols.files = symbols/*
symbols.path = /usr/share/tex-creator/symbols
INSTALLS += symbols
### BeQt translations ###
beqttrans.files = /usr/share/beqt/translations/*.qm
beqttrans.path = /usr/share/tex-creator/translations
INSTALLS += beqttrans
### Unix sh ###
unixsh.files = unix-only/tex-creator.sh
unixsh.path = /usr/bin
INSTALLS += unixsh
### Unix pixmaps ###
unixpixmaps.files = tex-creator.png
unixpixmaps.path = /usr/share/pixmaps
INSTALLS += unixpixmaps
### Unix desktop ###
unixdesktop.files = unix-only/tex-creator.desktop
unixdesktop.path = /usr/share/applications
INSTALLS += unixdesktop
}
win32 {
isEmpty(PREFIX) {
    PREFIX = $$(systemdrive)/TeX-Creator
}
### Target ###
target.path = $$PREFIX
INSTALLS = target
### Docs ###
docs.files = doc/*
docs.path = $$PREFIX/doc
INSTALLS += docs
### Translations ###
trans.files = translations/*.qm
trans.path = $$PREFIX/translations
INSTALLS += trans
### KLM ###
klm.files = klm/*
klm.path = $$PREFIX/klm
INSTALLS += klm
### Symbols ###
symbols.files = symbols/*
symbols.path = $$PREFIX/symbols
INSTALLS += symbols
### BeQt translations ###
beqttrans.files = /usr/share/beqt/translations/*.qm
beqttrans.path = $$PREFIX/translations
INSTALLS += beqttrans
}
