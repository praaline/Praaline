/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RESIZEABLE_BITMAP_H_
#define _RESIZEABLE_BITMAP_H_

#include <vector>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

class ResizeableBitset {

public:
    ResizeableBitset() : m_bits(0), m_size(0) {
    }
    ResizeableBitset(size_t size) : m_bits(new std::vector<uint8_t>), m_size(size) {
        m_bits->assign((size >> 3) + 1, 0);
    }
    ResizeableBitset(const ResizeableBitset &b) {
        m_bits = new std::vector<uint8_t>(*b.m_bits);
    }
    ResizeableBitset &operator=(const ResizeableBitset &b) {
        if (&b != this) return *this;
        delete m_bits;
        m_bits = new std::vector<uint8_t>(*b.m_bits);
        return *this;
    }
    ~ResizeableBitset() {
        delete m_bits;
    }
    
    void resize(size_t size) { // retaining existing data; not thread safe
        size_t bytes = (size >> 3) + 1;
        if (m_bits && bytes == m_bits->size()) return;
        std::vector<uint8_t> *newbits = new std::vector<uint8_t>(bytes);
        newbits->assign(bytes, 0);
        if (m_bits) {
            for (size_t i = 0; i < bytes && i < m_bits->size(); ++i) {
                (*newbits)[i] = (*m_bits)[i];
            }
            delete m_bits;
        }
        m_bits = newbits;
        m_size = size;
    }
    
    bool get(size_t column) const {
        return ((*m_bits)[column >> 3]) & (1u << (column & 0x07));
    }
    
    void set(size_t column) {
        ((*m_bits)[column >> 3]) |= uint8_t((1u << (column & 0x07)) & 0xff);
    }

    void reset(size_t column) {
        ((*m_bits)[column >> 3]) &= uint8_t((~(1u << (column & 0x07))) & 0xff);
    }

    void copy(size_t source, size_t dest) {
        get(source) ? set(dest) : reset(dest);
    }

    bool isAllOff() const {
        for (size_t i = 0; i < m_bits->size(); ++i) {
            if ((*m_bits)[i]) return false;
        }
        return true;
    }

    bool isAllOn() const {
        for (size_t i = 0; i + 1 < m_bits->size(); ++i) {
            if ((*m_bits)[i] != 0xff) return false;
        }
        for (size_t i = (m_size / 8) * 8; i < m_size; ++i) {
            if (!get(i)) return false;
        }
        return true;
    }

    size_t size() const {
        return m_size;
    }
    
private:
    std::vector<uint8_t> *m_bits;
    size_t m_size;
};


#endif

