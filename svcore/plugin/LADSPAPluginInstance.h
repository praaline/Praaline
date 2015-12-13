/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

/*
   This is a modified version of a source file from the 
   Rosegarden MIDI and audio sequencer and notation editor.
   This file copyright 2000-2006 Chris Cannam and Richard Bown.
*/

#ifndef _LADSPAPLUGININSTANCE_H_
#define _LADSPAPLUGININSTANCE_H_

#include <vector>
#include <set>
#include <QString>

#include "api/ladspa.h"
#include "RealTimePluginInstance.h"
#include "base/BaseTypes.h"

// LADSPA plugin instance.  LADSPA is a variable block size API, but
// for one reason and another it's more convenient to use a fixed
// block size in this wrapper.
//
class LADSPAPluginInstance : public RealTimePluginInstance
{
public:
    virtual ~LADSPAPluginInstance();

    virtual bool isOK() const { return m_instanceHandles.size() != 0; }

    int getClientId() const { return m_client; }
    virtual QString getPluginIdentifier() const { return m_identifier; }
    int getPosition() const { return m_position; }

    virtual std::string getIdentifier() const;
    virtual std::string getName() const;
    virtual std::string getDescription() const;
    virtual std::string getMaker() const;
    virtual int getPluginVersion() const;
    virtual std::string getCopyright() const;

    virtual void run(const RealTime &rt, int count = 0);

    virtual int getParameterCount() const;
    virtual void setParameterValue(int parameter, float value);
    virtual float getParameterValue(int parameter) const;
    virtual float getParameterDefault(int parameter) const;
    virtual int getParameterDisplayHint(int parameter) const;
    
    virtual ParameterList getParameterDescriptors() const;
    virtual float getParameter(std::string) const;
    virtual void setParameter(std::string, float);

    virtual int getBufferSize() const { return m_blockSize; }
    virtual int getAudioInputCount() const { return int(m_instanceCount * m_audioPortsIn.size()); }
    virtual int getAudioOutputCount() const { return int(m_instanceCount * m_audioPortsOut.size()); }
    virtual sample_t **getAudioInputBuffers() { return m_inputBuffers; }
    virtual sample_t **getAudioOutputBuffers() { return m_outputBuffers; }

    virtual int getControlOutputCount() const { return int(m_controlPortsOut.size()); }
    virtual float getControlOutputValue(int n) const;

    virtual bool isBypassed() const { return m_bypassed; }
    virtual void setBypassed(bool bypassed) { m_bypassed = bypassed; }

    virtual sv_frame_t getLatency();

    virtual void silence();
    virtual void setIdealChannelCount(int channels); // may re-instantiate

    virtual std::string getType() const { return "LADSPA Real-Time Plugin"; }

protected:
    // To be constructed only by LADSPAPluginFactory
    friend class LADSPAPluginFactory;

    // Constructor that creates the buffers internally
    // 
    LADSPAPluginInstance(RealTimePluginFactory *factory,
			 int client,
			 QString identifier,
                         int position,
			 sv_samplerate_t sampleRate,
			 int blockSize,
			 int idealChannelCount,
                         const LADSPA_Descriptor* descriptor);

    void init(int idealChannelCount = 0);
    void instantiate(sv_samplerate_t sampleRate);
    void cleanup();
    void activate();
    void deactivate();

    // Connection of data (and behind the scenes control) ports
    //
    void connectPorts();
    
    int                        m_client;
    int                        m_position;
    std::vector<LADSPA_Handle> m_instanceHandles;
    int                        m_instanceCount;
    const LADSPA_Descriptor   *m_descriptor;

    std::vector<std::pair<int, LADSPA_Data*> > m_controlPortsIn;
    std::vector<std::pair<int, LADSPA_Data*> > m_controlPortsOut;

    std::vector<int>          m_audioPortsIn;
    std::vector<int>          m_audioPortsOut;

    int                       m_blockSize;
    sample_t                **m_inputBuffers;
    sample_t                **m_outputBuffers;
    bool                      m_ownBuffers;
    sv_samplerate_t           m_sampleRate;
    float                    *m_latencyPort;
    bool                      m_run;
    
    bool                      m_bypassed;
};

#endif // _LADSPAPLUGININSTANCE_H_

