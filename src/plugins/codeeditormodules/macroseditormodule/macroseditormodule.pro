TEMPLATE = lib

TARGET = macroseditormodule

QT = core concurrent gui widgets
BEQT = core widgets codeeditor

include(../../plugin.pri)

HEADERS += \
    macro.h \
    macrocommand.h \
    macrocommandargument.h \
    macroexecutionstack.h \
    macroseditormodule.h \
    macroseditormoduleplugin.h \
    macrossettingstab.h

SOURCES += \
    macro.cpp \
    macrocommand.cpp \
    macrocommandargument.cpp \
    macroexecutionstack.cpp \
    macroseditormodule.cpp \
    macroseditormoduleplugin.cpp \
    macrossettingstab.cpp

##############################################################################
################################ Installing ##################################
##############################################################################

!contains(TCRT_CONFIG, no_install) {

include(../../../../prefix.pri)

##############################################################################
################################ Binaries ####################################
##############################################################################

target.path = $${PLUGINS_INSTALLS_PATH}
INSTALLS = target

} #end !contains(TCRT_CONFIG, no_install)
