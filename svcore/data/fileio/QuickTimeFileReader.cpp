/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2007 Chris Cannam and QMUL.
    
    Based on QTAudioFile.cpp from SoundBite, copyright 2006
    Chris Sutton and Mark Levy.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifdef HAVE_QUICKTIME

#include "QuickTimeFileReader.h"
#include "base/Profiler.h"
#include "base/ProgressReporter.h"
#include "system/System.h"

#include <QFileInfo>

#ifdef _WIN32
#include <QTML.h>
#include <Movies.h>
#else
#include <QuickTime/QuickTime.h>
#endif

class QuickTimeFileReader::D
{
public:
    D() : data(0), blockSize(1024) { }

    MovieAudioExtractionRef      extractionSessionRef;
    AudioBufferList              buffer;
    float                       *data;
    OSErr                        err; 
    AudioStreamBasicDescription  asbd;
    Movie                        movie;
    int                          blockSize;
};


QuickTimeFileReader::QuickTimeFileReader(FileSource source,
                                         DecodeMode decodeMode,
                                         CacheMode mode,
                                         sv_samplerate_t targetRate,
                                         bool normalised,
                                         ProgressReporter *reporter) :
    CodedAudioFileReader(mode, targetRate, normalised),
    m_source(source),
    m_path(source.getLocalFilename()),
    m_d(new D),
    m_reporter(reporter),
    m_cancelled(false),
    m_completion(0),
    m_decodeThread(0)
{
    m_channelCount = 0;
    m_fileRate = 0;

    Profiler profiler("QuickTimeFileReader::QuickTimeFileReader", true);

cerr << "QuickTimeFileReader: path is \"" << m_path << "\"" << endl;

    long QTversion;

#ifdef WIN32
    InitializeQTML(0); // FIXME should check QT version
#else
    m_d->err = Gestalt(gestaltQuickTime,&QTversion);
    if ((m_d->err != noErr) || (QTversion < 0x07000000)) {
        m_error = QString("Failed to find compatible version of QuickTime (version 7 or above required)");
        return;
    }
#endif 

    EnterMovies();
	
    Handle dataRef; 
    OSType dataRefType;

//    CFStringRef URLString = CFStringCreateWithCString
 //       (0, m_path.toLocal8Bit().data(), 0);


    QByteArray ba = m_path.toLocal8Bit();

    CFURLRef url = CFURLCreateFromFileSystemRepresentation
        (kCFAllocatorDefault,
         (const UInt8 *)ba.data(),
         (CFIndex)ba.length(),
         false);


//    m_d->err = QTNewDataReferenceFromURLCFString
    m_d->err = QTNewDataReferenceFromCFURL
        (url, 0, &dataRef, &dataRefType);

    if (m_d->err) { 
        m_error = QString("Error creating data reference for QuickTime decoder: code %1").arg(m_d->err);
        return;
    }
    
    short fileID = movieInDataForkResID; 
    short flags = 0; 
    m_d->err = NewMovieFromDataRef
        (&m_d->movie, flags, &fileID, dataRef, dataRefType);

    DisposeHandle(dataRef);
    if (m_d->err) { 
        m_error = QString("Error creating new movie for QuickTime decoder: code %1").arg(m_d->err); 
        return;
    }

    Boolean isProtected = 0;
    Track aTrack = GetMovieIndTrackType
        (m_d->movie, 1, SoundMediaType,
         movieTrackMediaType | movieTrackEnabledOnly);

    if (aTrack) {
        Media aMedia = GetTrackMedia(aTrack);	// get the track media
        if (aMedia) {
            MediaHandler mh = GetMediaHandler(aMedia);	// get the media handler we can query
            if (mh) {
                m_d->err = QTGetComponentProperty(mh,
                                                  kQTPropertyClass_DRM,
                                                  kQTDRMPropertyID_IsProtected,
                                                  sizeof(Boolean), &isProtected,nil);
            } else {
                m_d->err = 1;
            }
        } else {
            m_d->err = 1;
        }
    } else {
        m_d->err = 1;
    }
	
    if (m_d->err && m_d->err != kQTPropertyNotSupportedErr) { 
        m_error = QString("Error checking for DRM in QuickTime decoder: code %1").arg(m_d->err);
        return;
    } else if (!m_d->err && isProtected) { 
        m_error = QString("File is protected with DRM");
        return;
    } else if (m_d->err == kQTPropertyNotSupportedErr && !isProtected) {
        cerr << "QuickTime: File is not protected with DRM" << endl;
    }

    if (m_d->movie) {
        SetMovieActive(m_d->movie, TRUE);
        m_d->err = GetMoviesError();
        if (m_d->err) {
            m_error = QString("Error in QuickTime decoder activation: code %1").arg(m_d->err);
            return;
        }
    } else {
	m_error = QString("Error in QuickTime decoder: Movie object not valid");
	return;
    }
    
    m_d->err = MovieAudioExtractionBegin
        (m_d->movie, 0, &m_d->extractionSessionRef);
    if (m_d->err) {
        m_error = QString("Error in QuickTime decoder extraction init: code %1").arg(m_d->err);
        return;
    }

    m_d->err = MovieAudioExtractionGetProperty
        (m_d->extractionSessionRef,
         kQTPropertyClass_MovieAudioExtraction_Audio, kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
         sizeof(m_d->asbd),
         &m_d->asbd,
         nil);

    if (m_d->err) {
        m_error = QString("Error in QuickTime decoder property get: code %1").arg(m_d->err);
        return;
    }
	
    m_channelCount = m_d->asbd.mChannelsPerFrame;
    m_fileRate = m_d->asbd.mSampleRate;

    cerr << "QuickTime: " << m_channelCount << " channels, " << m_fileRate << " kHz" << endl;

    m_d->asbd.mFormatFlags =
        kAudioFormatFlagIsFloat |
        kAudioFormatFlagIsPacked |
        kAudioFormatFlagsNativeEndian;
    m_d->asbd.mBitsPerChannel = sizeof(float) * 8;
    m_d->asbd.mBytesPerFrame = sizeof(float) * m_d->asbd.mChannelsPerFrame;
    m_d->asbd.mBytesPerPacket = m_d->asbd.mBytesPerFrame;
	
    m_d->err = MovieAudioExtractionSetProperty
        (m_d->extractionSessionRef,
         kQTPropertyClass_MovieAudioExtraction_Audio,
         kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
         sizeof(m_d->asbd),
         &m_d->asbd);

    if (m_d->err) {
        m_error = QString("Error in QuickTime decoder property set: code %1").arg(m_d->err);
        m_channelCount = 0;
        return;
    }
    m_d->buffer.mNumberBuffers = 1;
    m_d->buffer.mBuffers[0].mNumberChannels = m_channelCount;
    m_d->buffer.mBuffers[0].mDataByteSize =
        sizeof(float) * m_channelCount * m_d->blockSize;
    m_d->data = new float[m_channelCount * m_d->blockSize];
    m_d->buffer.mBuffers[0].mData = m_d->data;

    initialiseDecodeCache();

    if (decodeMode == DecodeAtOnce) {

        if (m_reporter) {
            connect(m_reporter, SIGNAL(cancelled()), this, SLOT(cancelled()));
            m_reporter->setMessage
                (tr("Decoding %1...").arg(QFileInfo(m_path).fileName()));
        }

        while (1) {
            
            UInt32 framesRead = m_d->blockSize;
            UInt32 extractionFlags = 0;
            m_d->err = MovieAudioExtractionFillBuffer
                (m_d->extractionSessionRef, &framesRead, &m_d->buffer,
                 &extractionFlags);
            if (m_d->err) {
                m_error = QString("Error in QuickTime decoding: code %1")
                    .arg(m_d->err);
                break;
            }

            //!!! progress?

//    cerr << "Read " << framesRead << " frames (block size " << m_d->blockSize << ")" << endl;

            // QuickTime buffers are interleaved unless specified otherwise
            addSamplesToDecodeCache(m_d->data, framesRead);

            if (framesRead < m_d->blockSize) break;
        }
        
        finishDecodeCache();
        endSerialised();

        m_d->err = MovieAudioExtractionEnd(m_d->extractionSessionRef);
        if (m_d->err) {
            m_error = QString("Error ending QuickTime extraction session: code %1").arg(m_d->err);
        }

        m_completion = 100;

    } else {
        if (m_reporter) m_reporter->setProgress(100);

        if (m_channelCount > 0) {
            m_decodeThread = new DecodeThread(this);
            m_decodeThread->start();
        }
    }

    cerr << "QuickTimeFileReader::QuickTimeFileReader: frame count is now " << getFrameCount() << ", error is \"\"" << m_error << "\"" << endl;
}

QuickTimeFileReader::~QuickTimeFileReader()
{
    cerr << "QuickTimeFileReader::~QuickTimeFileReader" << endl;

    if (m_decodeThread) {
        m_cancelled = true;
        m_decodeThread->wait();
        delete m_decodeThread;
    }

    SetMovieActive(m_d->movie, FALSE);
    DisposeMovie(m_d->movie);

    delete[] m_d->data;
    delete m_d;
}

void
QuickTimeFileReader::cancelled()
{
    m_cancelled = true;
}

void
QuickTimeFileReader::DecodeThread::run()
{
    if (m_reader->m_cacheMode == CacheInTemporaryFile) {
        m_reader->m_completion = 1;
        m_reader->startSerialised("QuickTimeFileReader::Decode");
    }

    while (1) {
            
        UInt32 framesRead = m_reader->m_d->blockSize;
        UInt32 extractionFlags = 0;
        m_reader->m_d->err = MovieAudioExtractionFillBuffer
            (m_reader->m_d->extractionSessionRef, &framesRead,
             &m_reader->m_d->buffer, &extractionFlags);
        if (m_reader->m_d->err) {
            m_reader->m_error = QString("Error in QuickTime decoding: code %1")
                .arg(m_reader->m_d->err);
            break;
        }
       
        // QuickTime buffers are interleaved unless specified otherwise
        m_reader->addSamplesToDecodeCache(m_reader->m_d->data, framesRead);
        
        if (framesRead < m_reader->m_d->blockSize) break;
    }
        
    m_reader->finishDecodeCache();
    
    m_reader->m_d->err = MovieAudioExtractionEnd(m_reader->m_d->extractionSessionRef);
    if (m_reader->m_d->err) {
        m_reader->m_error = QString("Error ending QuickTime extraction session: code %1").arg(m_reader->m_d->err);
    }
    
    m_reader->m_completion = 100;
    m_reader->endSerialised();
} 

void
QuickTimeFileReader::getSupportedExtensions(std::set<QString> &extensions)
{
    extensions.insert("aiff");
    extensions.insert("aif");
    extensions.insert("au");
    extensions.insert("avi");
    extensions.insert("m4a");
    extensions.insert("m4b");
    extensions.insert("m4p");
    extensions.insert("m4v");
    extensions.insert("mov");
    extensions.insert("mp3");
    extensions.insert("mp4");
    extensions.insert("wav");
}

bool
QuickTimeFileReader::supportsExtension(QString extension)
{
    std::set<QString> extensions;
    getSupportedExtensions(extensions);
    return (extensions.find(extension.toLower()) != extensions.end());
}

bool
QuickTimeFileReader::supportsContentType(QString type)
{
    return (type == "audio/x-aiff" ||
            type == "audio/x-wav" ||
            type == "audio/mpeg" ||
            type == "audio/basic" ||
            type == "audio/x-aac" ||
            type == "video/mp4" ||
            type == "video/quicktime");
}

bool
QuickTimeFileReader::supports(FileSource &source)
{
    return (supportsExtension(source.getExtension()) ||
            supportsContentType(source.getContentType()));
}

#endif

