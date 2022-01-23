#ifndef PLUGIN_WEBDJANGO_GLOBAL_H
#define PLUGIN_WEBDJANGO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PLUGIN_WEBDJANGO_LIBRARY)
#  define PLUGIN_WEBDJANGO_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define PLUGIN_WEBDJANGO_SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PLUGIN_WEBDJANGO_GLOBAL_H