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

#ifndef _CLIPBOARD_H_
#define _CLIPBOARD_H_

#include <QString>
#include <vector>

#include "base/BaseTypes.h"

class Clipboard
{
public:
    class Point
    {
    public:
        Point(sv_frame_t frame, QString label);
        Point(sv_frame_t frame, float value, QString label);
        Point(sv_frame_t frame, float value, sv_frame_t duration, QString label);
        Point(sv_frame_t frame, float value, sv_frame_t duration, float level, QString label);
        Point(const Point &point);
        Point &operator=(const Point &point);

        bool haveFrame() const;
        sv_frame_t getFrame() const;
        Point withFrame(sv_frame_t frame) const;

        bool haveValue() const;
        float getValue() const;
        Point withValue(float value) const;
        
        bool haveDuration() const;
        sv_frame_t getDuration() const;
        Point withDuration(sv_frame_t duration) const;
        
        bool haveLabel() const;
        QString getLabel() const;
        Point withLabel(QString label) const;

        bool haveLevel() const;
        float getLevel() const;
        Point withLevel(float level) const;

        bool haveReferenceFrame() const;
        bool referenceFrameDiffers() const; // from point frame

        sv_frame_t getReferenceFrame() const;
        void setReferenceFrame(sv_frame_t);

    private:
        bool m_haveFrame;
        sv_frame_t m_frame;
        bool m_haveValue;
        float m_value;
        bool m_haveDuration;
        sv_frame_t m_duration;
        bool m_haveLabel;
        QString m_label;
        bool m_haveLevel;
        float m_level;
        bool m_haveReferenceFrame;
        sv_frame_t m_referenceFrame;
    };

    Clipboard();
    ~Clipboard();

    typedef std::vector<Point> PointList;

    void clear();
    bool empty() const;
    const PointList &getPoints() const;
    void setPoints(const PointList &points);
    void addPoint(const Point &point);

    bool haveReferenceFrames() const;
    bool referenceFramesDiffer() const;

protected:
    PointList m_points;
};

#endif
