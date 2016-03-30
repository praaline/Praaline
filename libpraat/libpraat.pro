# Praat-as-a-library for Praaline
# (c) George Christodoulides 2015

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += external/gsl \
           external/glpk \
           external/mp3 \
           external/flac \
           external/portaudio \
           external/espeak \
           kar \
           num \
           sys \
           dwsys \
           stat \
           fon \
           dwtools \
           LPC \
           EEG \
           gram \
           FFNet \
           artsynth \
           contrib/ola

# SUBDIRS += main
SUBDIRS += qpraat
