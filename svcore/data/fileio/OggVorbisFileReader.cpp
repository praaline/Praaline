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

#ifdef HAVE_OGGZ
#ifdef HAVE_FISHSOUND

#include "OggVorbisFileReader.h"

#include "base/ProgressReporter.h"
#include "base/Profiler.h"
#include "system/System.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include <QFileInfo>

//static int instances = 0;

OggVorbisFileReader::OggVorbisFileReader(FileSource source,
                                         DecodeMode decodeMode,
                                         CacheMode mode,
                                         sv_samplerate_t targetRate,
                                         bool normalised,
                                         ProgressReporter *reporter) :
    CodedAudioFileReader(mode, targetRate, normalised),
    m_source(source),
    m_path(source.getLocalFilename()),
    m_reporter(reporter),
    m_fileSize(0),
    m_bytesRead(0),
    m_commentsRead(false),
    m_cancelled(false),
    m_completion(0),
    m_decodeThread(0)
{
    m_channelCount = 0;
    m_fileRate = 0;

//    cerr << "OggVorbisFileReader::OggVorbisFileReader(" << m_path << "): now have " << (++instances) << " instances" << endl;

    Profiler profiler("OggVorbisFileReader::OggVorbisFileReader", true);

    QFileInfo info(m_path);
    m_fileSize = info.size();

    if (!(m_oggz = oggz_open(m_path.toLocal8Bit().data(), OGGZ_READ))) {
	m_error = QString("File %1 is not an OGG file.").arg(m_path);
	return;
    }

    FishSoundInfo fsinfo;
    m_fishSound = fish_sound_new(FISH_SOUND_DECODE, &fsinfo);

    fish_sound_set_decoded_callback(m_fishSound, acceptFrames, this);
    oggz_set_read_callback(m_oggz, -1, (OggzReadPacket)readPacket, this);

    if (decodeMode == DecodeAtOnce) {

        if (m_reporter) {
            connect(m_reporter, SIGNAL(cancelled()), this, SLOT(cancelled()));
            m_reporter->setMessage
                (tr("Decoding %1...").arg(QFileInfo(m_path).fileName()));
        }

        while (oggz_read(m_oggz, 1024) > 0);
        
        fish_sound_delete(m_fishSound);
        m_fishSound = 0;
        oggz_close(m_oggz);
        m_oggz = 0;

        if (isDecodeCacheInitialised()) finishDecodeCache();
        endSerialised();

    } else {

        if (m_reporter) m_reporter->setProgress(100);

        while (oggz_read(m_oggz, 1024) > 0 &&
               (m_channelCount == 0 || m_fileRate == 0 || m_sampleRate == 0));

        if (m_channelCount > 0) {
            m_decodeThread = new DecodeThread(this);
            m_decodeThread->start();
        }
    }
}

OggVorbisFileReader::~OggVorbisFileReader()
{
//    cerr << "OggVorbisFileReader::~OggVorbisFileReader(" << m_path << "): now have " << (--instances) << " instances" << endl;
    if (m_decodeThread) {
        m_cancelled = true;
        m_decodeThread->wait();
        delete m_decodeThread;
    }
}

void
OggVorbisFileReader::cancelled()
{
    m_cancelled = true; 
}

void
OggVorbisFileReader::DecodeThread::run()
{
    if (m_reader->m_cacheMode == CacheInTemporaryFile) {
        m_reader->m_completion = 1;
        m_reader->startSerialised("OggVorbisFileReader::Decode");
    }

    while (oggz_read(m_reader->m_oggz, 1024) > 0);
        
    fish_sound_delete(m_reader->m_fishSound);
    m_reader->m_fishSound = 0;
    oggz_close(m_reader->m_oggz);
    m_reader->m_oggz = 0;
    
    if (m_reader->isDecodeCacheInitialised()) m_reader->finishDecodeCache();
    m_reader->m_completion = 100;

    m_reader->endSerialised();
} 

int
OggVorbisFileReader::readPacket(OGGZ *, ogg_packet *packet, long, void *data)
{
    OggVorbisFileReader *reader = (OggVorbisFileReader *)data;
    FishSound *fs = reader->m_fishSound;

    fish_sound_prepare_truncation(fs, packet->granulepos, int(packet->e_o_s));
    fish_sound_decode(fs, packet->packet, packet->bytes);

    reader->m_bytesRead += packet->bytes;

    // The number of bytes read by this function is smaller than
    // the file size because of the packet headers
    int p = int(lrint(double(reader->m_bytesRead) * 114 /
                      double(reader->m_fileSize)));
    if (p > 99) p = 99;
    reader->m_completion = p;
    reader->progress(p);

    if (reader->m_fileSize > 0 && reader->m_reporter) {
        reader->m_reporter->setProgress(p);
    }

    if (reader->m_cancelled) return 1;
    return 0;
}

int
OggVorbisFileReader::acceptFrames(FishSound *fs, float **frames, long nframes,
				  void *data)
{
    OggVorbisFileReader *reader = (OggVorbisFileReader *)data;

    if (!reader->m_commentsRead) {
        const FishSoundComment *comment;
        comment = fish_sound_comment_first_byname(fs, (char *)"TITLE");
        if (comment && comment->value) {
            reader->m_title = QString::fromUtf8(comment->value);
        }
        comment = fish_sound_comment_first_byname(fs, (char *)"ARTIST");
        if (comment && comment->value) {
            reader->m_maker = QString::fromUtf8(comment->value);
        }
        comment = fish_sound_comment_first(fs);
        while (comment) {
            reader->m_tags[QString::fromUtf8(comment->name).toUpper()] =
                QString::fromUtf8(comment->value);
            comment = fish_sound_comment_next(fs, comment);
        }
        reader->m_commentsRead = true;
    }

    if (reader->m_channelCount == 0) {
	FishSoundInfo fsinfo;
	fish_sound_command(fs, FISH_SOUND_GET_INFO,
			   &fsinfo, sizeof(FishSoundInfo));
	reader->m_fileRate = fsinfo.samplerate;
	reader->m_channelCount = fsinfo.channels;
        reader->initialiseDecodeCache();
    }

    if (nframes > 0) {
        reader->addSamplesToDecodeCache(frames, nframes);
    }

    if (reader->m_cancelled) return 1;
    return 0;
}

void
OggVorbisFileReader::getSupportedExtensions(std::set<QString> &extensions)
{
    extensions.insert("ogg");
    extensions.insert("oga");
}

bool
OggVorbisFileReader::supportsExtension(QString extension)
{
    std::set<QString> extensions;
    getSupportedExtensions(extensions);
    return (extensions.find(extension.toLower()) != extensions.end());
}

bool
OggVorbisFileReader::supportsContentType(QString type)
{
    return (type == "application/ogg");
}

bool
OggVorbisFileReader::supports(FileSource &source)
{
    return (supportsExtension(source.getExtension()) ||
            supportsContentType(source.getContentType()));
}

#endif
#endif
