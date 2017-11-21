#ifndef LIBPRAAT_GLOBAL_H
#define LIBPRAAT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PRAAT_LIBRARY)
#  define LIBPRAATSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBPRAATSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBPRAAT_GLOBAL_H
