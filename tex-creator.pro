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
    res/translations/tex-creator_ru.ts

RC_FILE = tex-creator.rc

unix:LIBS += -lbeqtcore -lbeqtgui
unix:INCLUDEPATH += "/usr/include/beqt"
win32:LIBS += -L"$$(systemdrive)/Program files/BeQt/lib" -lbeqtcore1 -lbeqtgui1
win32:INCLUDEPATH += "$$(systemdrive)/Program files/BeQt/include"
