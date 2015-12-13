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

#include "CSVFileWriter.h"

#include "model/Model.h"
#include "model/SparseOneDimensionalModel.h"
#include "model/SparseTimeValueModel.h"
#include "model/NoteModel.h"
#include "model/TextModel.h"

#include "base/TempWriteFile.h"
#include "base/Exceptions.h"
#include "base/Selection.h"

#include <QFile>
#include <QTextStream>

CSVFileWriter::CSVFileWriter(QString path,
                             Model *model,
                             QString delimiter,
                             DataExportOptions options) :
    m_path(path),
    m_model(model),
    m_error(""),
    m_delimiter(delimiter),
    m_options(options)
{
}

CSVFileWriter::~CSVFileWriter()
{
}

bool
CSVFileWriter::isOK() const
{
    return m_error == "";
}

QString
CSVFileWriter::getError() const
{
    return m_error;
}

void
CSVFileWriter::write()
{
    try {
        TempWriteFile temp(m_path);

        QFile file(temp.getTemporaryFilename());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            m_error = tr("Failed to open file %1 for writing")
                .arg(temp.getTemporaryFilename());
            return;
        }
    
        QTextStream out(&file);
        out << m_model->toDelimitedDataStringWithOptions
            (m_delimiter, m_options);

        file.close();
        temp.moveToTarget();

    } catch (FileOperationFailed &f) {
        m_error = f.what();
    }
}

void
CSVFileWriter::writeSelection(MultiSelection *selection)
{
    try {
        TempWriteFile temp(m_path);

        QFile file(temp.getTemporaryFilename());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            m_error = tr("Failed to open file %1 for writing")
                .arg(temp.getTemporaryFilename());
            return;
        }
    
        QTextStream out(&file);

        for (MultiSelection::SelectionList::iterator i =
                 selection->getSelections().begin();
             i != selection->getSelections().end(); ++i) {
	
            sv_frame_t f0(i->getStartFrame()), f1(i->getEndFrame());
            out << m_model->toDelimitedDataStringSubsetWithOptions
                (m_delimiter, m_options, f0, f1);
        }

        file.close();
        temp.moveToTarget();

    } catch (FileOperationFailed &f) {
        m_error = f.what();
    }
}



