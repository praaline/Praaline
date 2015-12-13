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

#include "Selection.h"
#include <QTextStream>

Selection::Selection() :
    m_startFrame(0),
    m_endFrame(0)
{
}

Selection::Selection(sv_frame_t startFrame, sv_frame_t endFrame) :
    m_startFrame(startFrame),
    m_endFrame(endFrame)
{
    if (m_startFrame > m_endFrame) {
	sv_frame_t tmp = m_endFrame;
	m_endFrame = m_startFrame;
	m_startFrame = tmp;
    }
}

Selection::Selection(const Selection &s) :
    m_startFrame(s.m_startFrame),
    m_endFrame(s.m_endFrame)
{
}

Selection &
Selection::operator=(const Selection &s)
{
    if (this != &s) {
	m_startFrame = s.m_startFrame;
	m_endFrame = s.m_endFrame;
    } 
    return *this;
}

Selection::~Selection()
{
}

bool
Selection::isEmpty() const
{
    return m_startFrame == m_endFrame;
}

sv_frame_t
Selection::getStartFrame() const
{
    return m_startFrame;
}

sv_frame_t
Selection::getEndFrame() const
{
    return m_endFrame;
}

bool
Selection::contains(sv_frame_t frame) const
{
    return (frame >= m_startFrame) && (frame < m_endFrame);
}

bool
Selection::operator<(const Selection &s) const
{
    if (isEmpty()) {
	if (s.isEmpty()) return false;
	else return true;
    } else {
	if (s.isEmpty()) return false;
	else return (m_startFrame < s.m_startFrame);
    }
}

bool
Selection::operator==(const Selection &s) const
{
    if (isEmpty()) return s.isEmpty();

    return (m_startFrame == s.m_startFrame &&
	    m_endFrame == s.m_endFrame);
}


MultiSelection::MultiSelection()
{
}

MultiSelection::~MultiSelection()
{
}

const MultiSelection::SelectionList &
MultiSelection::getSelections() const
{
    return m_selections;
}

void
MultiSelection::setSelection(const Selection &selection)
{
    clearSelections();
    addSelection(selection);
}

void
MultiSelection::addSelection(const Selection &selection)
{
    m_selections.insert(selection);

    // Cope with a sitation where the new selection overlaps one or
    // more existing ones.  This is a terribly inefficient way to do
    // this, but that probably isn't significant in real life.

    // It's essential for the correct operation of
    // getContainingSelection that the selections do not overlap, so
    // this is not just a frill.

    for (SelectionList::iterator i = m_selections.begin();
	 i != m_selections.end(); ) {
	
	SelectionList::iterator j = i;
	if (++j == m_selections.end()) break;

	if (i->getEndFrame() >= j->getStartFrame()) {
	    Selection merged(i->getStartFrame(),
			     std::max(i->getEndFrame(), j->getEndFrame()));
	    m_selections.erase(i);
	    m_selections.erase(j);
	    m_selections.insert(merged);
	    i = m_selections.begin();
	} else {
	    ++i;
	}
    }
}

void
MultiSelection::removeSelection(const Selection &selection)
{
    //!!! Likewise this needs to cope correctly with the situation
    //where selection is not one of the original selection set but
    //simply overlaps one of them (cutting down the original selection
    //appropriately)

    if (m_selections.find(selection) != m_selections.end()) {
	m_selections.erase(selection);
    }
}

void
MultiSelection::clearSelections()
{
    if (!m_selections.empty()) {
	m_selections.clear();
    }
}

void
MultiSelection::getExtents(sv_frame_t &startFrame, sv_frame_t &endFrame) const
{
    startFrame = 0;
    endFrame = 0;
    
    for (SelectionList::const_iterator i = m_selections.begin();
	 i != m_selections.end(); ++i) {

        if (i == m_selections.begin() || i->getStartFrame() < startFrame) {
            startFrame = i->getStartFrame();
        }

        if (i == m_selections.begin() || i->getEndFrame() > endFrame) {
            endFrame = i->getEndFrame();
        }
    }
}

Selection
MultiSelection::getContainingSelection(sv_frame_t frame, bool defaultToFollowing) const
{
    // This scales very badly with the number of selections, but it's
    // more efficient for very small numbers of selections than a more
    // scalable method, and I think that may be what we need

    for (SelectionList::const_iterator i = m_selections.begin();
	 i != m_selections.end(); ++i) {

	if (i->contains(frame)) return *i;

	if (i->getStartFrame() > frame) {
	    if (defaultToFollowing) return *i;
	    else return Selection();
	}
    }

    return Selection();
}

void
MultiSelection::toXml(QTextStream &stream, QString indent,
                      QString extraAttributes) const
{
    stream << indent << QString("<selections %1>\n").arg(extraAttributes);
    for (SelectionList::iterator i = m_selections.begin();
	 i != m_selections.end(); ++i) {
	stream << indent
               << QString("  <selection start=\"%1\" end=\"%2\"/>\n")
	    .arg(i->getStartFrame()).arg(i->getEndFrame());
    }
    stream << indent << "</selections>\n";
}

