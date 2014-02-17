TEMPLATE = subdirs

SUBDIRS = src

!contains(TCRT_CONFIG, no_install) {

include(prefix.pri)

##############################################################################
################################ Headers #####################################
##############################################################################

#Gets contents of an .h header
#Returns the corresponding actual header path
defineReplace(getActualHeaderInternal) {
    headerContents=$${1}
    actualHeader=$$replace(headerContents, "$${LITERAL_HASH}include", "")
    actualHeader=$$replace(actualHeader, "\\.\\./", "")
    actualHeader=$$replace(actualHeader, "\"", "")
    return($${PWD}/$${actualHeader})
}

#Gets path to a header (either .h or with no extension)
#Returns corresponding actual header path
defineReplace(getActualHeader) {
    headerPath=$${1}
    headerContents=$$cat($${headerPath})
    isEmpty(headerContents) {
        headerPath=
    } else:!equals(headerContents, $$replace(headerContents, "\\.\\./", "")) {
        headerPath=$$getActualHeaderInternal($${headerContents})
    }
    return($${headerPath})
}

#Gets include subdirectory name
#Returns a list of actual headers' paths to which headers in the given subdir point
defineReplace(getActualHeaders) {
    headerPaths=$$files($${PWD}/include/*)
    actualHeaderPaths=
    for(headerPath, headerPaths) {
        actualHeaderPath=$$getActualHeader($${headerPath})
        !isEmpty(actualHeaderPath):!equals(actualHeaderPath, $${PWD}/):actualHeaderPaths+=$${actualHeaderPath}
    }
    return($${actualHeaderPaths})
}

contains(TCRT_CONFIG, headers) {
    #Global
    tcrtInstallsHeaders.files=$$getActualHeaders()
    tcrtInstallsHeaders.path=$${HEADERS_INSTALLS_PATH}
    INSTALLS += tcrtInstallsHeaders
} #end contains(TCRT_CONFIG, headers)

} #end !contains(TCRT_CONFIG, no_install)
