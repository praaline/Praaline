//
//  praatlib.h
//  praat64
//

#ifndef _praatlib_h
#define _praatlib_h

#include <QtCore/qglobal.h>

#if defined(PRAAT_LIB)
#  if defined(PRAAT_LIB_SHARED_LIBRARY)
#    define PRAAT_LIB_EXPORT Q_DECL_EXPORT
#  else
#    define PRAAT_LIB_EXPORT
#  endif
#else
#  define PRAAT_LIB_EXPORT Q_DECL_IMPORT
#endif

PRAAT_LIB_EXPORT void praat_lib_init();

#endif
