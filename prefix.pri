#mac {
    #isEmpty(PREFIX):PREFIX=/Library
    #TODO: Add ability to create bundles
#} else:unix:!mac {
#TODO: Add MacOS support
mac|unix {
    isEmpty(PREFIX):PREFIX=/usr
} else:win32 {
    isEmpty(PREFIX):PREFIX=$$(systemdrive)/PROGRA~1/TeX-Creator
}

isEmpty(BINARY_INSTALLS_PATH):BINARY_INSTALLS_PATH=$${PREFIX}/bin
isEmpty(PLUGINS_INSTALLS_PATH):PLUGINS_INSTALLS_PATH=$${PREFIX}/lib/tex-creator/plugins
isEmpty(RESOURCES_INSTALLS_PATH):RESOURCES_INSTALLS_PATH=$${PREFIX}/share/tex-creator
isEmpty(HEADERS_INSTALLS_PATH):HEADERS_INSTALLS_PATH=$${PREFIX}/include/tex-creator

!isEmpty(PREFIX):export(PREFIX)
!isEmpty(BINARY_INSTALLS_PATH):export(BINARY_INSTALLS_PATH)
!isEmpty(PLUGINS_INSTALLS_PATH):export(PLUGINS_INSTALLS_PATH)
!isEmpty(HEADERS_INSTALLS_PATH):export(HEADERS_INSTALLS_PATH)
!isEmpty(RESOURCES_INSTALLS_PATH):export(RESOURCES_INSTALLS_PATH)
