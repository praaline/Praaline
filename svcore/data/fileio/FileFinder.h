/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2009 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _FILE_FINDER_H_
#define _FILE_FINDER_H_

#include <QString>

class FileFinder 
{
public:
    enum FileType {
        SessionFile,
        AudioFile,
        LayerFile,
        LayerFileNoMidi,
        SessionOrAudioFile,
        ImageFile,
        AnyFile,
        CSVFile,
        LayerFileNonSV,
        LayerFileNoMidiNonSV,
    };

    virtual QString getOpenFileName(FileType type, QString fallbackLocation = "") = 0;
    virtual QString getSaveFileName(FileType type, QString fallbackLocation = "") = 0;
    virtual void registerLastOpenedFilePath(FileType type, QString path) = 0;

    virtual QString find(FileType type, QString location, QString lastKnownLocation = "") = 0;

    static FileFinder *getInstance() {
        FFContainer *container = FFContainer::getInstance();
        return container->getFileFinder();
    }

protected:
    class FFContainer {
    public:
        static FFContainer *getInstance() {
            static FFContainer instance;
            return &instance;
        }
        void setFileFinder(FileFinder *ff) { m_ff = ff; }
        FileFinder *getFileFinder() const { return m_ff; }
    private:
        FileFinder *m_ff;
    };

    static void registerFileFinder(FileFinder *ff) {
        FFContainer *container = FFContainer::getInstance();
        container->setFileFinder(ff);
    }
};

#endif

    
