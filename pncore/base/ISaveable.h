#ifndef ISAVEABLE_H
#define ISAVEABLE_H

/*
    Praaline - Core module - Base
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

#include "pncore_global.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT ISaveable {
public:
    inline bool isNew() const   { return m_isNew;   }
    inline bool isDirty() const { return m_isDirty; }
    inline bool isClean() const { return (!m_isNew) && (!m_isDirty); }

protected:
    bool m_isNew;
    bool m_isDirty;

friend class IDatastore;
};

} // namespace Core
} // namespace Praaline

#endif // ISAVEABLE_H
