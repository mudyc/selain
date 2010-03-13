TEMPLATE = subdirs
SUBDIRS = pixmapload \
    selain
selain.subdir = selain
pixmapload.subdir = pixmapload
selain.depends = pixmapload
OTHER_FILES += FEATURES.txt \
    LICENSE.txt
