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
    administrationdialog.cpp \
    keyboardlayouteditormodule.cpp \
    macroseditormodule.cpp \
    cache.cpp \
    editsampledialog.cpp \
    registerdialog.cpp \
    remoteterminaldriver.cpp

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
    administrationdialog.h \
    keyboardlayouteditormodule.h \
    macroseditormodule.h \
    cache.h \
    editsampledialog.h \
    registerdialog.h \
    remoteterminaldriver.h

TRANSLATIONS += \
    ../translations/tex-creator_ru.ts

RC_FILE = win.rc

##############################################################################
################################ Generating translations #####################
##############################################################################

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

##############################################################################
################################ Installing ##################################
##############################################################################

!contains(CONFIG, no_install) {

#mac {
    #isEmpty(PREFIX):PREFIX=/Library
    #TODO: Add ability to create bundles
#} else:unix:!mac {
#TODO: Add MacOS support
mac|unix {
    isEmpty(PREFIX):PREFIX=/usr
    equals(PREFIX, "/")|equals(PREFIX, "/usr")|equals(PREFIX, "/usr/local") {
        isEmpty(BINARY_INSTALLS_PATH):BINARY_INSTALLS_PATH=$${PREFIX}/lib/tex-creator
        isEmpty(RESOURCES_INSTALLS_PATH):RESOURCES_INSTALLS_PATH=$${PREFIX}/share/tex-creator
    } else {
        isEmpty(BINARY_INSTALLS_PATH):BINARY_INSTALLS_PATH=$${PREFIX}/lib
        isEmpty(RESOURCES_INSTALLS_PATH):RESOURCES_INSTALLS_PATH=$${PREFIX}
    }
} else:win32 {
    isEmpty(PREFIX):PREFIX=$$(systemdrive)/PROGRA~1/TeX-Creator
    isEmpty(BINARY_INSTALLS_PATH):BINARY_INSTALLS_PATH=$${PREFIX}
    isEmpty(RESOURCES_INSTALLS_PATH):RESOURCES_INSTALLS_PATH=$${PREFIX}
}

##############################################################################
################################ Binaries ####################################
##############################################################################

target.path = $${BINARY_INSTALLS_PATH}
INSTALLS = target

##############################################################################
################################ Translations ################################
##############################################################################

!contains(CONFIG, builtin_resources) {
    installsTranslations.files=$$files($${PWD}/../translations/*.qm)
    installsTranslations.path=$${RESOURCES_INSTALLS_PATH}/translations
    INSTALLS += installsTranslations
}

##############################################################################
################################ Other resources #############################
##############################################################################

#TODO
mac|unix {
    installsSh.files=$$files($${PWD}/../unix-only/tex-creator.sh)
    installsSh.path=$${BINARY_INSTALLS_PATH}
    INSTALLS+=installsSh
    installsDesktop.files=$$files($${PWD}/../unix-only/tex-creator.desktop)
    installsDesktop.path=$${RESOURCES_INSTALLS_PATH}/../applications
    INSTALLS+=installsDesktop
}

!contains(CONFIG, builtin_resources) {
    installsChangelog.files=$$files($${PWD}/changelog/*.txt)
    installsChangelog.path=$${RESOURCES_INSTALLS_PATH}/changelog
    INSTALLS += installsChangelog
    installsCopying.files=$$files($${PWD}/copying/*.txt)
    installsCopying.path=$${RESOURCES_INSTALLS_PATH}/copying
    INSTALLS += installsCopying
    installsDescription.files=$$files($${PWD}/description/*.txt)
    installsDescription.path=$${RESOURCES_INSTALLS_PATH}/description
    INSTALLS += installsDescription
    #doc
    #
    #
    installsInfos.files=$$files($${PWD}/infos/*.beqt-info)
    installsInfos.path=$${RESOURCES_INSTALLS_PATH}/infos
    INSTALLS += installsInfos
    installsKlm.files=$$files($${PWD}/klm/*.klm)
    installsKlm.path=$${RESOURCES_INSTALLS_PATH}/klm
    INSTALLS += installsKlm
    installsSymbols.files=$$files($${PWD}/symbols/*.png)
    installsSymbols.files+=$$files({PWD}/symbols/symbols.txt)
    installsSymbols.path=$${RESOURCES_INSTALLS_PATH}/symbols
    INSTALLS += installsSymbols
}

} #end !contains(CONFIG, no_install)
