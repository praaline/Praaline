AR = '/usr/bin/ar'
ARFLAGS = 'rcs'
BINDIR = '/usr/local/bin'
BUILD_SHARED = True
BUILD_STATIC = None
BUILD_TESTS = None
BUILD_UTILS = True
CC = ['/usr/bin/gcc']
CCLNK_SRC_F = []
CCLNK_TGT_F = ['-o']
CC_NAME = 'gcc'
CC_SRC_F = []
CC_TGT_F = ['-c', '-o']
CC_VERSION = ('4', '8', '4')
CFLAGS = ['-I/home/george/Develop/Praaline/sv-dependency-builds/src/serd-0.18.2', '-DNDEBUG', '-fshow-column', '-std=c99']
CFLAGS_MACBUNDLE = ['-fPIC']
CFLAGS_cshlib = ['-fPIC']
COMPILER_CC = 'gcc'
CPPPATH_ST = '-I%s'
CXXFLAGS = ['-I/home/george/Develop/Praaline/sv-dependency-builds/src/serd-0.18.2', '-DNDEBUG', '-fshow-column']
DATADIR = '/usr/local/share'
DEBUG = False
DEFINES = ['HAVE_FMAX=1', 'HAVE_POSIX_MEMALIGN=1', 'HAVE_POSIX_FADVISE=1', 'HAVE_FILENO=1', 'SERD_VERSION="0.18.2"']
DEFINES_FILENO = ['_POSIX_C_SOURCE=201112L']
DEFINES_POSIX_FADVISE = ['_POSIX_C_SOURCE=201112L']
DEFINES_POSIX_MEMALIGN = ['_POSIX_C_SOURCE=201112L']
DEFINES_ST = '-D%s'
DEST_BINFMT = 'elf'
DEST_CPU = 'x86_64'
DEST_OS = 'linux'
DOCDIR = '/usr/local/share/doc'
DOCS = False
INCLUDEDIR = '/usr/local/include'
INCLUDES_SERD = ['${INCLUDEDIR}/serd-0']
LIBDIR = '/usr/local/lib'
LIBPATH_SERD = ['/usr/local/lib']
LIBPATH_ST = '-L%s'
LIB_FMAX = ['m']
LIB_SERD = ['serd-0']
LIB_ST = '-l%s'
LINKFLAGS_MACBUNDLE = ['-bundle', '-undefined', 'dynamic_lookup']
LINKFLAGS_cshlib = ['-shared']
LINKFLAGS_cstlib = ['-Wl,-Bstatic']
LINK_CC = ['/usr/bin/gcc']
LV2DIR = '/usr/local/lib/lv2'
MANDIR = '/usr/local/share/man'
PARDEBUG = False
PKG_serd = 'serd-0'
PREFIX = '/usr/local'
RPATH_ST = '-Wl,-rpath,%s'
SERD_VERSION = '0.18.2'
SHLIB_MARKER = '-Wl,-Bdynamic'
SONAME_ST = '-Wl,-h,%s'
STATIC_PROGS = None
STLIBPATH_ST = '-L%s'
STLIB_MARKER = '-Wl,-Bstatic'
STLIB_ST = '-l%s'
SYSCONFDIR = '/usr/local/etc'
cfg_files = ['/home/george/Develop/Praaline/sv-dependency-builds/src/serd-0.18.2/build/serd_config.h']
cprogram_PATTERN = '%s'
cshlib_PATTERN = 'lib%s.so'
cstlib_PATTERN = 'lib%s.a'
define_key = ['HAVE_FMAX', 'HAVE_POSIX_MEMALIGN', 'HAVE_POSIX_FADVISE', 'HAVE_FILENO', 'SERD_VERSION']
macbundle_PATTERN = '%s.bundle'
