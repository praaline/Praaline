#ifndef PNCORE_GLOBAL_H
#define PNCORE_GLOBAL_H

/*
    Praaline - Core module
    Copyright (c) 2011-2017 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include <QtCore/qglobal.h>

#if defined(PRAALINE_CORE_LIBRARY)
#  define PRAALINE_CORE_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define PRAALINE_CORE_SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PNCORE_GLOBAL_H
