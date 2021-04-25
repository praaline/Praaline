# Praaline
# (c) George Christodoulides 2012-2020

TEMPLATE = subdirs
CONFIG += ordered

# Dependencies
SUBDIRS += dependencies/praaline-dependencies.pro

# Praaline
SUBDIRS += praaline-core praaline-media praaline-asr pnlib pngui
SUBDIRS += sub_dataquay svcore svgui svapp
# SUBDIRS += libpraat
SUBDIRS += plugins
# SUBDIRS += PraalinePy
SUBDIRS += app

sub_dataquay.file = dataquay/libdataquay.pro

pnlib.depends = praaline-core
pngui.depends = praaline-core
plugins.depends = praaline-core

svgui.depends = svcore
svapp.depends = svcore svgui

app.depends = praaline-core pnlib pngui sub_dataquay svcore svgui svapp

