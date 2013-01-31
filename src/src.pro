TEMPLATE = app
TARGET = tex-creator

CONFIG += release

QT = core network gui widgets
BEQT = core network widgets codeeditor

include(../beqt/depend.pri)

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    consolewidget.cpp \
    applicationserver.cpp \
    symbolswidget.cpp \
    sample.cpp \
    client.cpp \
    application.cpp \
    texsamplesettingstab.cpp \
    codeeditorsettingstab.cpp \
    maindocumenteditormodule.cpp \
    generalsettingstab.cpp \
    samplesproxymodel.cpp \
    sampleswidget.cpp \
    samplesmodel.cpp \
    consolesettingstab.cpp \
    sampleinfodialog.cpp \
    requestprogressdialog.cpp \
    sendsamplesdialog.cpp \
    accountsettingstab.cpp \
    administrationdialog.cpp

HEADERS += \
    mainwindow.h \
    consolewidget.h \
    applicationserver.h \
    symbolswidget.h \
    sample.h \
    client.h \
    application.h \
    texsamplesettingstab.h \
    codeeditorsettingstab.h \
    maindocumenteditormodule.h \
    generalsettingstab.h \
    samplesproxymodel.h \
    sampleswidget.h \
    samplesmodel.h \
    consolesettingstab.h \
    sampleinfodialog.h \
    requestprogressdialog.h \
    sendsamplesdialog.h \
    accountsettingstab.h \
    administrationdialog.h

TRANSLATIONS += \
    ../translations/tex-creator_ru.ts

RC_FILE = win.rc

#Gets a file name
#Returns the given file name.
#On Windows slash characters will be replaced by backslashes
defineReplace(nativeFileName) {
    fileName=$${1}
    win32:fileName=$$replace(fileName, "/", "\\")
    return($${fileName})
}

translationsTs=$$files($${PWD}/../translations/*.ts)
for(fileName, translationsTs) {
    system(lrelease $$nativeFileName($${fileName}))
}

contains(CONFIG, builtin_resources) {
    DEFINES += BUILTIN_RESOURCES
    RESOURCES += \
        tex_creator.qrc \
        tex_creator_doc.qrc \
        tex_creator_symbols.qrc \
        ../translations/tex_creator_translations.qrc
}

#beqtInstallsTranslations.files=$$files($${PWD}/translations/*.qm)
#beqtInstallsTranslations.path=$${resourcesInstallsPath}/translations
#INSTALLS += beqtInstallsTranslations
