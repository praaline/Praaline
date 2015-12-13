#ifndef PNCORE_GLOBAL_H
#define PNCORE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PRAALINE_CORE_LIBRARY)
#  define PRAALINE_CORE_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define PRAALINE_CORE_SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PNCORE_GLOBAL_H
