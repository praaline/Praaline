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

#ifndef _NOTE_MODEL_H_
#define _NOTE_MODEL_H_

#include "IntervalModel.h"
#include "NoteData.h"
#include "base/RealTime.h"
#include "base/PlayParameterRepository.h"
#include "base/Pitch.h"

/**
 * NoteModel -- a concrete IntervalModel for notes.
 */

/**
 * Note type for use in a sparse model.  All we mean by a "note" is
 * something that has an onset time, a single value, a duration, and a
 * level.  Like other points, it can also have a label.  With this
 * point type, the model can be thought of as representing a simple
 * MIDI-type piano roll, except that the y coordinates (values) do not
 * have to be discrete integers.
 */

struct Note
{
public:
    Note(sv_frame_t _frame) : frame(_frame), value(0.0f), duration(0), level(1.f) { }
    Note(sv_frame_t _frame, float _value, sv_frame_t _duration, float _level, QString _label) :
	frame(_frame), value(_value), duration(_duration), level(_level), label(_label) { }

    int getDimensions() const { return 3; }

    sv_frame_t frame;
    float value;
    sv_frame_t duration;
    float level;
    QString label;

    QString getLabel() const { return label; }
    
    void toXml(QTextStream &stream,
               QString indent = "",
               QString extraAttributes = "") const
    {
	stream <<
            QString("%1<point frame=\"%2\" value=\"%3\" duration=\"%4\" level=\"%5\" label=\"%6\" %7/>\n")
	    .arg(indent).arg(frame).arg(value).arg(duration).arg(level)
            .arg(XmlExportable::encodeEntities(label)).arg(extraAttributes);
    }

    QString toDelimitedDataString(QString delimiter, DataExportOptions opts, sv_samplerate_t sampleRate) const {
        QStringList list;
        list << RealTime::frame2RealTime(frame, sampleRate).toString().c_str();
        list << QString("%1").arg(value);
        list << RealTime::frame2RealTime(duration, sampleRate).toString().c_str();
        if (!(opts & DataExportOmitLevels)) {
            list << QString("%1").arg(level);
        }
        if (label != "") list << label;
        return list.join(delimiter);
    }

    struct Comparator {
	bool operator()(const Note &p1,
			const Note &p2) const {
	    if (p1.frame != p2.frame) return p1.frame < p2.frame;
	    if (p1.value != p2.value) return p1.value < p2.value;
	    if (p1.duration != p2.duration) return p1.duration < p2.duration;
            if (p1.level != p2.level) return p1.level < p2.level;
	    return p1.label < p2.label;
	}
    };
    
    struct OrderComparator {
	bool operator()(const Note &p1,
			const Note &p2) const {
	    return p1.frame < p2.frame;
	}
    };
};


class NoteModel : public IntervalModel<Note>, public NoteExportable
{
    Q_OBJECT
    
public:
    NoteModel(sv_samplerate_t sampleRate, int resolution,
	      bool notifyOnAdd = true) :
	IntervalModel<Note>(sampleRate, resolution, notifyOnAdd),
	m_valueQuantization(0)
    {
	PlayParameterRepository::getInstance()->addPlayable(this);
    }

    NoteModel(sv_samplerate_t sampleRate, int resolution,
	      float valueMinimum, float valueMaximum,
	      bool notifyOnAdd = true) :
	IntervalModel<Note>(sampleRate, resolution,
                            valueMinimum, valueMaximum,
                            notifyOnAdd),
	m_valueQuantization(0)
    {
	PlayParameterRepository::getInstance()->addPlayable(this);
    }

    virtual ~NoteModel()
    {
        PlayParameterRepository::getInstance()->removePlayable(this);
    }

    float getValueQuantization() const { return m_valueQuantization; }
    void setValueQuantization(float q) { m_valueQuantization = q; }

    QString getTypeName() const { return tr("Note"); }
    std::string getType() const { return "NoteModel"; }

    virtual bool canPlay() const { return true; }

    virtual QString getDefaultPlayClipId() const
    {
        return "elecpiano";
    }

    virtual void toXml(QTextStream &out,
                       QString indent = "",
                       QString extraAttributes = "") const
    {
        std::cerr << "NoteModel::toXml: extraAttributes = \"" 
                  << extraAttributes.toStdString() << std::endl;

        IntervalModel<Note>::toXml
	    (out,
             indent,
	     QString("%1 subtype=\"note\" valueQuantization=\"%2\"")
	     .arg(extraAttributes).arg(m_valueQuantization));
    }

    /**
     * TabularModel methods.  
     */
    
    virtual int getColumnCount() const
    {
        return 6;
    }

    virtual QString getHeading(int column) const
    {
        switch (column) {
        case 0: return tr("Time");
        case 1: return tr("Frame");
        case 2: return tr("Pitch");
        case 3: return tr("Duration");
        case 4: return tr("Level");
        case 5: return tr("Label");
        default: return tr("Unknown");
        }
    }

    virtual QVariant getData(int row, int column, int role) const
    {
        if (column < 4) {
            return IntervalModel<Note>::getData(row, column, role);
        }

        PointListConstIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return QVariant();

        switch (column) {
        case 4: return i->level;
        case 5: return i->label;
        default: return QVariant();
        }
    }

    virtual SVCommand *getSetDataCommand(int row, int column, const QVariant &value, int role)
    {
        if (column < 4) {
            return IntervalModel<Note>::getSetDataCommand
                (row, column, value, role);
        }

        if (role != Qt::EditRole) return 0;
        PointListConstIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return 0;
        EditCommand *command = new EditCommand(this, tr("Edit Data"));

        Point point(*i);
        command->deletePoint(point);

        switch (column) {
        case 4: point.level = float(value.toDouble()); break;
        case 5: point.label = value.toString(); break;
        }

        command->addPoint(point);
        return command->finish();
    }

    virtual SortType getSortType(int column) const
    {
        if (column == 5) return SortAlphabetical;
        return SortNumeric;
    }

    /**
     * NoteExportable methods.
     */

    NoteList getNotes() const {
        return getNotesWithin(getStartFrame(), getEndFrame());
    }

    NoteList getNotesWithin(sv_frame_t startFrame, sv_frame_t endFrame) const {
        
	PointList points = getPoints(startFrame, endFrame);
        NoteList notes;

        for (PointList::iterator pli =
		 points.begin(); pli != points.end(); ++pli) {

	    sv_frame_t duration = pli->duration;
            if (duration == 0 || duration == 1) {
                duration = sv_frame_t(getSampleRate() / 20);
            }

            int pitch = int(lrintf(pli->value));
            
            int velocity = 100;
            if (pli->level > 0.f && pli->level <= 1.f) {
                velocity = int(lrintf(pli->level * 127));
            }

            NoteData note(pli->frame, duration, pitch, velocity);

            if (getScaleUnits() == "Hz") {
                note.frequency = pli->value;
                note.midiPitch = Pitch::getPitchForFrequency(note.frequency);
                note.isMidiPitchQuantized = false;
            }
        
            notes.push_back(note);
        }
        
        return notes;
    }

protected:
    float m_valueQuantization;
};

#endif
