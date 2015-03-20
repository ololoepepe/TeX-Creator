TEMPLATE = app
TARGET = tex-creator

CONFIG += release

BEQT = core network sql widgets networkwidgets codeeditor
TSMP = core network widgets networkwidgets

isEmpty(BEQT_PREFIX) {
    mac|unix {
        BEQT_PREFIX=/usr/share/beqt
    } else:win32 {
        BEQT_PREFIX=$$(systemdrive)/PROGRA~1/BeQt
    }
}
include($${BEQT_PREFIX}/share/beqt/depend.pri)

isEmpty(TSMP_PREFIX) {
    mac|unix {
        TSMP_PREFIX=/usr/share/texsample
    } else:win32 {
        TSMP_PREFIX=$$(systemdrive)/PROGRA~1/TeXSample
    }
}
include($${TSMP_PREFIX}/share/texsample/depend.pri)

tcrtHeadersPath=$${PWD}/../../include

INCLUDEPATH *= $${tcrtHeadersPath}
DEPENDPATH *= $${tcrtHeadersPath}

SOURCES += \
    application.cpp \
    consolewidget.cpp \
    editeditormodule.cpp \
    latexfiletype.cpp \
    main.cpp \
    maindocumenteditormodule.cpp \
    mainwindow.cpp \
    symbolswidget.cpp \
    settings.cpp

HEADERS += \
    application.h \
    consolewidget.h \
    editeditormodule.h \
    latexfiletype.h \
    maindocumenteditormodule.h \
    mainwindow.h \
    symbolswidget.h \
    settings.h

include(settingstab/settingstab.pri)
include(texsample/texsample.pri)

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
        tex_creator_1.qrc \
        tex_creator_2.qrc \
        tex_creator_3.qrc \
        tex_creator_4.qrc \
        tex_creator_5.qrc \
        tex_creator_6.qrc \
        tex_creator_dictionaries_1.qrc \
        tex_creator_dictionaries_2.qrc \
        tex_creator_dictionaries_3.qrc \
        tex_creator_dictionaries_4.qrc \
        tex_creator_doc_01.qrc \
        tex_creator_doc_02.qrc \
        tex_creator_doc_03.qrc \
        tex_creator_doc_04.qrc \
        tex_creator_doc_05.qrc \
        tex_creator_doc_06.qrc \
        tex_creator_doc_07.qrc \
        tex_creator_doc_08.qrc \
        tex_creator_doc_09.qrc \
        tex_creator_doc_10.qrc \
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
