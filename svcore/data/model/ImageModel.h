/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2007 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _IMAGE_MODEL_H_
#define _IMAGE_MODEL_H_

#include "SparseModel.h"
#include "base/XmlExportable.h"
#include "base/RealTime.h"

#include <QStringList>

/**
 * Image point type for use in a SparseModel.  This represents an
 * image, identified by filename, at a given time.  The filename can
 * be empty, in which case we instead have a space to put an image in.
 */

struct ImagePoint : public XmlExportable
{
public:
    ImagePoint(long _frame) : frame(_frame) { }
    ImagePoint(long _frame, QString _image, QString _label) :
        frame(_frame), image(_image), label(_label) { }

    int getDimensions() const { return 1; }
    
    long frame;
    QString image;
    QString label;

    QString getLabel() const { return label; }
    
    void toXml(QTextStream &stream,
               QString indent = "",
               QString extraAttributes = "") const
    {
        stream <<
                  QString("%1<point frame=\"%2\" image=\"%3\" label=\"%4\" %5/>\n")
                  .arg(indent).arg(frame)
                  .arg(encodeEntities(image))
                  .arg(encodeEntities(label))
                  .arg(extraAttributes);
    }

    QString toDelimitedDataString(QString delimiter, DataExportOptions, sv_samplerate_t sampleRate) const
    {
        QStringList list;
        list << RealTime::frame2RealTime(frame, sampleRate).toString().c_str();
        list << image;
        if (label != "") list << label;
        return list.join(delimiter);
    }

    struct Comparator {
        bool operator()(const ImagePoint &p1,
                        const ImagePoint &p2) const {
            if (p1.frame != p2.frame) return p1.frame < p2.frame;
            if (p1.label != p2.label) return p1.label < p2.label;
            return p1.image < p2.image;
        }
    };
    
    struct OrderComparator {
        bool operator()(const ImagePoint &p1,
                        const ImagePoint &p2) const {
            return p1.frame < p2.frame;
        }
    };
};


// Make this a class rather than a typedef so it can be predeclared.

class ImageModel : public SparseModel<ImagePoint>
{
    Q_OBJECT

public:
    ImageModel(sv_samplerate_t sampleRate, int resolution, bool notifyOnAdd = true) :
        SparseModel<ImagePoint>(sampleRate, resolution, notifyOnAdd)
    { }

    QString getTypeName() const { return tr("Image"); }
    std::string getType() const { return "ImageModel"; }

    virtual void toXml(QTextStream &out,
                       QString indent = "",
                       QString extraAttributes = "") const
    {
        SparseModel<ImagePoint>::toXml
                (out,
                 indent,
                 QString("%1 subtype=\"image\"")
                 .arg(extraAttributes));
    }

    /**
     * Command to change the image for a point.
     */
    class ChangeImageCommand : public SVCommand
    {
    public:
        ChangeImageCommand(ImageModel *model,
                           const ImagePoint &point,
                           QString newImage,
                           QString newLabel) :
            m_model(model), m_oldPoint(point), m_newPoint(point) {
            m_newPoint.image = newImage;
            m_newPoint.label = newLabel;
        }

        virtual QString getName() const { return tr("Edit Image"); }

        virtual void execute() {
            m_model->deletePoint(m_oldPoint);
            m_model->addPoint(m_newPoint);
            std::swap(m_oldPoint, m_newPoint);
        }

        virtual void unexecute() { execute(); }

    private:
        ImageModel *m_model;
        ImagePoint m_oldPoint;
        ImagePoint m_newPoint;
    };

    /**
     * TabularModel methods.
     */
    
    virtual int getColumnCount() const
    {
        return 4;
    }

    virtual QString getHeading(int column) const
    {
        switch (column) {
        case 0: return tr("Time");
        case 1: return tr("Frame");
        case 2: return tr("Image");
        case 3: return tr("Label");
        default: return tr("Unknown");
        }
    }

    virtual QVariant getData(int row, int column, int role) const
    {
        if (column < 2) {
            return SparseModel<ImagePoint>::getData
                    (row, column, role);
        }

        PointListConstIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return QVariant();

        switch (column) {
        case 2: return i->image;
        case 3: return i->label;
        default: return QVariant();
        }
    }

    virtual SVCommand *getSetDataCommand(int row, int column, const QVariant &value, int role)
    {
        if (column < 2) {
            return SparseModel<ImagePoint>::getSetDataCommand
                    (row, column, value, role);
        }

        if (role != Qt::EditRole) return 0;
        PointListIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return 0;
        EditCommand *command = new EditCommand(this, tr("Edit Data"));

        Point point(*i);
        command->deletePoint(point);

        switch (column) {
        case 2: point.image = value.toString(); break;
        case 3: point.label = value.toString(); break;
        }

        command->addPoint(point);
        return command->finish();
    }

    virtual bool isColumnTimeValue(int column) const
    {
        return (column < 2);
    }

    virtual SortType getSortType(int column) const
    {
        if (column > 2) return SortAlphabetical;
        return SortNumeric;
    }
};


#endif



