TEMPLATE = subdirs

SUBDIRS = tex-creator

!contains(TCRT_CONFIG, no_plugins) {
    SUBDIRS += plugins
    contains(CONFIG, static):tex-creator.depends = plugins
}
