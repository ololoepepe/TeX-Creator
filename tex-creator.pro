QT += \
    network \
    xml

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/consolewidget.cpp \
    src/macrosection.cpp \
    src/applicationserver.cpp \
    src/generalsettingstab.cpp \
    src/symbolswidget.cpp

HEADERS += \
    src/mainwindow.h \
    src/consolewidget.h \
    src/macrosection.h \
    src/applicationserver.h \
    src/generalsettingstab.h \
    src/symbolswidget.h

RESOURCES += \
    tex-creator.qrc

TRANSLATIONS += \
    res/translations/tex-creator_ru.ts

ICON = "logo.icns"

RC_FILE = tex-creator.rc

unix:LIBS += -lbeqtcore -lbeqtgui
unix:INCLUDEPATH += "/usr/include/beqt"
win32:LIBS += -L"$$(systemdrive)/Program files/BeQt/lib" -lbeqtcore0 -lbeqtgui0
win32:INCLUDEPATH += "$$(systemdrive)/Program files/BeQt/include"
