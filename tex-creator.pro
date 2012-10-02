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

builddir = .build

MOC_DIR = $$builddir
OBJECTS_DIR = $$builddir
RCC_DIR = $$builddir

###############################################################################
# PREFIX
###############################################################################

unix {
isEmpty(PREFIX) {
    PREFIX = /usr
}
}
win32 {
isEmpty(PREFIX) {
    contains(QMAKE_HOST.arch, x86_64) {
        PREFIX = $$quote($$(systemdrive)/Program files (x86))
    } else {
        PREFIX = $$quote($$(systemdrive)/Program files)
    }
}
}

###############################################################################
# LIBS and INCLUDEPATH
###############################################################################

unix {
LIBS += -lbeqtcore -lbeqtgui
INCLUDEPATH += $$PREFIX/include/beqt
}
win32 {
LIBS += -L$$PREFIX/BeQt/lib -lbeqtcore1 -lbeqtgui1
INCLUDEPATH += $$PREFIX/BeQt/include
}

###############################################################################
# INSTALLS.files
###############################################################################

I_DOCS.files = doc/*
I_KLM.files = klm/*
I_SYMBOLS.files = symbols/*
I_TRANSLATIONS.files = translations/*.qm
unix:I_TRANSLATIONS.files += $$PREFIX/share/beqt/translations/*.qm
win32:I_TRANSLATIONS.files += $$PREFIX/BeQt/translations/*.qm
#I_TRANSLATIONS.files = $$(QTDIR)/translations/*.qm #TODO: copy only "qt_*.qm"
### unix-only ###
unix {
I_DESKTOPS.files = unix-only/tex-creator.desktop
I_PIXMAPS.files = tex-creator.png
I_SCRIPTS.files = unix-only/tex-creator.sh
}

###############################################################################
# INSTALLS.path
###############################################################################

unix {
target.path = $$PREFIX/lib/tex-creator
I_DOCS.path = $$PREFIX/share/doc/tex-creator
I_KLM.path = $$PREFIX/share/tex-creator/klm
I_SYMBOLS.path = $$PREFIX/share/tex-creator/symbols
I_TRANSLATIONS.path = $$PREFIX/share/tex-creator/translations
I_LIBS.path = $$PREFIX/lib/tex-creator
### unix-only ###
I_DESKTOPS.path = $$PREFIX/share/applications
I_PIXMAPS.path = $$PREFIX/share/pixmaps
I_SCRIPTS.path = $$PREFIX/bin
}
win32 {
appdir = $$quote($$PREFIX/TeX Creator)
target.path = $$appdir
I_DOCS.path = $$appdir/docs
I_KLM.path = $$appdir/klm
I_SYMBOLS.path = $$appdir/symbols
I_TRANSLATIONS.path = $$appdir/translations
I_LIBS.path = $$appdir
}

###############################################################################
# INSTALLS.extra
###############################################################################

unix {
I_LIBS.extra = \
    cp -P $$PREFIX/lib/libbeqtcore.so* $$PREFIX/lib/tex-creator; \
    cp -P $$PREFIX/lib/libbeqtgui.so* $$PREFIX/lib/tex-creator; \
    cp -P $$(QTDIR)/lib/libQtCore.so* $$PREFIX/lib/tex-creator; \
    cp -P $$(QTDIR)/lib/libQtGui.so* $$PREFIX/lib/tex-creator; \
    cp -P $$(QTDIR)/lib/libQtNetwork.so* $$PREFIX/lib/tex-creator; \
    cp -P $$(QTDIR)/lib/libQtXml.so* $$PREFIX/lib/tex-creator
}
win32 {
I_LIBS.extra = \
    copy $$PREFIX/lib/beqtcore1.dll $$PREFIX/appdir; \
    copy $$PREFIX/lib/beqtgui1.dll $$PREFIX/appdir; \
    copy $$(QTDIR)/lib/QtCore4.dll $$PREFIX/appdir; \
    copy $$(QTDIR)/lib/QtGui4.dll $$PREFIX/appdir; \
    copy $$(QTDIR)/lib/QtNetwork4.dll $$PREFIX/appdir; \
    copy $$(QTDIR)/lib/QtXml4.dll $$PREFIX/appdir
}

###############################################################################
# INSTALLS
###############################################################################

INSTALLS = target
INSTALLS += I_DOCS
INSTALLS += I_KLM
INSTALLS += I_SYMBOLS
INSTALLS += I_TRANSLATIONS
INSTALLS += I_LIBS
### unix-only ###
unix {
INSTALLS += I_DESKTOPS
INSTALLS += I_PIXMAPS
INSTALLS += I_SCRIPTS
}
