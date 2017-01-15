/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Praaline - Core module - Base
    Copyright (c) 2011-2017 George Christodoulides

    This is a modified version of a source file from:
    Sonic Visualiser - An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include <cstdint>

/** Frame index, the unit of our time axis. This is signed because the
    axis conceptually extends below zero: zero represents the start of
    the main loaded audio model, not the start of time; a windowed
    transform could legitimately produce results before then. We also
    use this for frame counts, simply to avoid error-prone arithmetic
    between signed and unsigned types.
*/
typedef int64_t sv_frame_t;

/** Check whether an integer index is in range for a container,
    avoiding overflows and signed/unsigned comparison warnings.
*/
template<typename T, typename C>
bool in_range_for(const C &container, T i)
{
    if (i < 0) return false;
    if (sizeof(T) > sizeof(typename C::size_type)) {
	return i < static_cast<T>(container.size());
    } else {
	return static_cast<typename C::size_type>(i) < container.size();
    }
}

/** Sample rate. We have to deal with sample rates provided as float
    or (unsigned) int types, so we might as well have a type that can
    represent both. Storage size isn't an issue anyway.
*/
typedef double sv_samplerate_t;

#endif

