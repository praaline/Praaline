#ifndef PLUGIN_LONGALIGN_GLOBAL_H
#define PLUGIN_LONGALIGN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PLUGIN_LONGALIGN_LIBRARY)
#  define PLUGIN_LONGALIGN_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define PLUGIN_LONGALIGN_SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PLUGIN_LONGALIGN_GLOBAL_H
