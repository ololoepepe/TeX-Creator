QT += \
    network \
    xml

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
LIBS += -L"$$(systemdrive)/Program files/BeQt/lib" -lbeqtcore1 -lbeqtgui1
INCLUDEPATH += "$$(systemdrive)/Program files/BeQt/include"
}

MOC_DIR = .build
OBJECTS_DIR = .build
RCC_DIR = .build

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
### BeQt libs ###
beqtlibs.files += /usr/lib/libbeqtcore.so.1
beqtlibs.files += /usr/lib/libbeqtgui.so.1
beqtlibs.path = /usr/lib/tex-creator/beqt
INSTALLS += beqtlibs
### BeQt translations ###
beqttrans.files = /usr/share/beqt/translations/*.qm
beqttrans.path = /usr/share/tex-creator/translations
INSTALLS += beqttrans
### Qt libs ###
qtlibs.files += $$QMAKE_LIBDIR_QT/libQtCore.so.4
qtlibs.files += $$QMAKE_LIBDIR_QT/libQtGui.so.4
qtlibs.files += $$QMAKE_LIBDIR_QT/libQtNetwork.so.4
qtlibs.files += $$QMAKE_LIBDIR_QT/libQtXml.so.4
qtlibs.path = /usr/lib/tex-creator/qt
INSTALLS += qtlibs
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
}
