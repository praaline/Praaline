# libpraat

libpraat is a fork of the praat (http://www.fon.hum.uva.nl/praat/) application created by Paul Boersma and David Weenink at the University of Amsterdam.

This project is being created along with jpraat (https://github.com/ghedlund/jpraat/) to provide java wrappers for parts of the Praat framework.

## Building

### Mac OS X

Create makefile symlink (source root folder):

```ln -s makefiles/makefile.defs.macos.lib makefile.defs```

Run make:

```make -f makefile-mac library```

### Linux (ubuntu 14.04)

Install dependencies:

```sudo apt-get install libgtk2.0-dev libasound2-dev```

Create makefile symlink (source root folder):

```ln -s makefiles/makefile.defs.linux.silent.lib makefile.defs```

Run make:

```make library```

### Windows (using ubuntu 14.04 + mingw-w64)

Setup mingw64 as described in [mingw-setup.md](./mingw-setup.md).

Create makefile symlink (source root folder):

```ln -s makefiles/makefile.defs.mingw64.lib makefile.defs```

Run make:

```make library```

