TEMPLATE = subdirs

SUBDIRS = tex-creator

!contains(TCRT_CONFIG, no_plugins) {
    SUBDIRS += plugins
    tex-creator.depends = plugins
}
