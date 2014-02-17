TEMPLATE = app
TARGET = tex-creator

CONFIG += release

QT = core concurrent network gui widgets
BEQT = core network widgets codeeditor networkwidgets
TSMP = core widgets

isEmpty(BEQT_PREFIX) {
    #TODO: Add MacOS support
    mac|unix {
        BEQT_PREFIX=/usr/share/beqt
    } else:win32 {
        BEQT_PREFIX=$$(systemdrive)/PROGRA~1/BeQt
    }
}
include($${BEQT_PREFIX}/depend.pri)

isEmpty(TSMP_PREFIX) {
    #TODO: Add MacOS support
    mac|unix {
        TSMP_PREFIX=/usr/share/texsample
    } else:win32 {
        TSMP_PREFIX=$$(systemdrive)/PROGRA~1/TeXSample
    }
}
include($${TSMP_PREFIX}/depend.pri)

tcrtHeadersPath=$${PWD}/../../include

INCLUDEPATH *= $${tcrtHeadersPath}
DEPENDPATH *= $${tcrtHeadersPath}

SOURCES += \
    application.cpp \
    applicationserver.cpp \
    cache.cpp \
    client.cpp \
    consolewidget.cpp \
    global.cpp \
    keyboardlayouteditormodule.cpp \
    main.cpp \
    maindocumenteditormodule.cpp \
    mainwindow.cpp \
    networksettingstab.cpp \
    remoteterminaldriver.cpp \
    samplesmodel.cpp \
    samplesproxymodel.cpp \
    samplewidget.cpp \
    symbolswidget.cpp \
    texsamplesettingstab.cpp \
    texsamplewidget.cpp

HEADERS += \
    application.h \
    applicationserver.h \
    cache.h \
    client.h \
    consolewidget.h \
    global.h \
    keyboardlayouteditormodule.h \
    maindocumenteditormodule.h \
    mainwindow.h \
    networksettingstab.h \
    remoteterminaldriver.h \
    samplesmodel.h \
    samplesproxymodel.h \
    samplewidget.h \
    symbolswidget.h \
    texsamplesettingstab.h \
    texsamplewidget.h


TRANSLATIONS += \
    ../../translations/tex-creator/tex-creator_ru.ts

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

translationsTs=$$files($${PWD}/../../translations/tex-creator/*.ts)
for(fileName, translationsTs) {
    system(lrelease $$nativeFileName($${fileName}))
}

contains(TCRT_CONFIG, builtin_resources) {
    DEFINES += BUILTIN_RESOURCES
    RESOURCES += \
        tex_creator.qrc \
        tex_creator_dictionaries.qrc \
        tex_creator_doc.qrc \
        tex_creator_symbols.qrc \
        ../../translations/tex-creator/tex_creator_translations.qrc
}

##############################################################################
################################ Installing ##################################
##############################################################################

!contains(TCRT_CONFIG, no_install) {

include(../../prefix.pri)

##############################################################################
################################ Binaries ####################################
##############################################################################

target.path = $${BINARY_INSTALLS_PATH}
INSTALLS = target

##############################################################################
################################ Translations ################################
##############################################################################

!contains(TCRT_CONFIG, builtin_resources) {
    installsTranslations.files=$$files($${PWD}/../../translations/tex-creator/*.qm)
    installsTranslations.path=$${RESOURCES_INSTALLS_PATH}/translations
    INSTALLS += installsTranslations
}

##############################################################################
################################ Other resources #############################
##############################################################################

!contains(TCRT_CONFIG, builtin_resources) {
    installsChangelog.files=$$files($${PWD}/changelog/*.txt)
    installsChangelog.path=$${RESOURCES_INSTALLS_PATH}/changelog
    INSTALLS += installsChangelog
    installsCopying.files=$$files($${PWD}/copying/*.txt)
    installsCopying.path=$${RESOURCES_INSTALLS_PATH}/copying
    INSTALLS += installsCopying
    installsDescription.files=$$files($${PWD}/description/*.txt)
    installsDescription.path=$${RESOURCES_INSTALLS_PATH}/description
    INSTALLS += installsDescription
    installsDictionaries.files=$$files($${PWD}/dictionaries/*)
    installsDictionaries.path=$${RESOURCES_INSTALLS_PATH}/dictionaries
    INSTALLS += installsDictionaries
    installsDocs.files=$$files($${PWD}/doc/*)
    installsDocs.path=$${RESOURCES_INSTALLS_PATH}/doc
    INSTALLS += installsDocs
    installsIcons.files=$$files($${PWD}/icons/*)
    installsIcons.path=$${RESOURCES_INSTALLS_PATH}/icons
    INSTALLS += installsIcons
    installsInfos.files=$$files($${PWD}/infos/*.beqt-info)
    installsInfos.path=$${RESOURCES_INSTALLS_PATH}/infos
    INSTALLS += installsInfos
    installsKlm.files=$$files($${PWD}/klm/*.klm)
    installsKlm.path=$${RESOURCES_INSTALLS_PATH}/klm
    INSTALLS += installsKlm
    installsPixmaps.files=$$files($${PWD}/pixmaps/*)
    installsPixmaps.path=$${RESOURCES_INSTALLS_PATH}/pixmaps
    INSTALLS += installsPixmaps
    installsSymbols.files=$$files($${PWD}/symbols/*.png)
    installsSymbols.files+=$$files({PWD}/symbols/symbols.txt)
    installsSymbols.path=$${RESOURCES_INSTALLS_PATH}/symbols
    INSTALLS += installsSymbols
}

} #end !contains(TCRT_CONFIG, no_install)
