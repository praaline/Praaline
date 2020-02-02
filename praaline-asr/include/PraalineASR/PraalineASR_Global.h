#ifndef PRAALINE_ASR_GLOBAL_H
#define PRAALINE_ASR_GLOBAL_H

/*
    Praaline - Libraries - ASR
    Copyright (c) 2011-2020 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include <QtGlobal>

#if defined(LIBRARY_PRAALINE_ASR)
#  define PRAALINE_ASR_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define PRAALINE_ASR_SHARED_EXPORT Q_DECL_IMPORT
#endif


#if !defined(USE_NAMESPACE_PRAALINE_ASR) /* namespace */
# define PRAALINE_ASR_NAMESPACE
# define PRAALINE_ASR_USING_NAMESPACE
# define PRAALINE_ASR_BEGIN_NAMESPACE
# define PRAALINE_ASR_END_NAMESPACE
#else /* namespace */
# define PRAALINE_ASR_NAMESPACE Praaline::ASR
# define PRAALINE_ASR_USING_NAMESPACE using namespace Praaline::ASR;
# define PRAALINE_ASR_BEGIN_NAMESPACE namespace Praaline { namespace ASR {
# define PRAALINE_ASR_END_NAMESPACE } }
#endif /* namespace */


#endif // PRAALINE_ASR_GLOBAL_H
