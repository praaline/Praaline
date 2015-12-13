/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- */

/* dssi.h

   DSSI version 0.10
   Copyright (c) 2004,2005 Chris Cannam, Steve Harris and Sean Bolton
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.
*/

#ifndef DSSI_INCLUDED
#define DSSI_INCLUDED

#include "ladspa.h"
#include "alsa/seq_event.h"

#define DSSI_VERSION "0.10"
#define DSSI_VERSION_MAJOR 0
#define DSSI_VERSION_MINOR 10

#ifdef __cplusplus
extern "C" {
#endif

/* 
   There is a need for an API that supports hosted MIDI soft synths
   with GUIs in Linux audio applications.  In time the GMPI initiative
   should comprehensively address this need, but the requirement for
   Linux applications to be able to support simple hosted synths is
   here now, and GMPI is not.  This proposal (the "DSSI Soft Synth
   Interface" or DSSI, pronounced "dizzy") aims to provide a simple
   solution in a way that we hope will prove complete and compelling
   enough to support now, yet not so compelling as to supplant GMPI or
   any other comprehensive future proposal.

   For simplicity and familiarity, this API is based as far as
   possible on existing work -- the LADSPA plugin API for control
   values and audio processing, and the ALSA sequencer event types for
   MIDI event communication.  The GUI part of the proposal is quite
   new, but may also be applicable retroactively to LADSPA plugins
   that do not otherwise support this synth interface.
*/

/*
   A program wishing to use the DSSI v2 API should set the following
   symbol to 2 before including this header.
*/
#if (!defined DSSI_API_LEVEL)
#define DSSI_API_LEVEL 1
#endif

typedef struct _DSSI_Program_Descriptor {

    /** Bank number for this program.  Note that DSSI does not support
        MIDI-style separation of bank LSB and MSB values.  There is no
        restriction on the set of available banks: the numbers do not
        need to be contiguous, there does not need to be a bank 0, etc. */
    unsigned long Bank;

    /** Program number (unique within its bank) for this program.
	There is no restriction on the set of available programs: the
	numbers do not need to be contiguous, there does not need to
	be a program 0, etc. */
    unsigned long Program;

    /** Name of the program. */
    const char * Name;

} DSSI_Program_Descriptor;


#define DSSI_TRANSPORT_VALID_STATE  0x01
#define DSSI_TRANSPORT_VALID_BPM    0x02
#define DSSI_TRANSPORT_VALID_BBT    0x10
#define DSSI_TRANSPORT_VALID_TIME   0x20

#define DSSI_TRANSPORT_STATE_STOPPED       0
#define DSSI_TRANSPORT_STATE_RUNNING       1
#define DSSI_TRANSPORT_STATE_FREEWHEELING  2
#define DSSI_TRANSPORT_STATE_OTHER         3  /* waiting for sync, ? */

typedef struct _DSSI_Transport_Info {

    /** The value of this field indicates which of the following
     *  transport information fields contain valid values. It is
     *  the logical OR of the DSSI_TRANSPORT_VALID_* bits defined
     *  above, and may be zero. */
    int  Valid;


    /** This field is valid when (Valid & DSSI_TRANSPORT_VALID_STATE)
     *  is true:
     *
     *  ---- The current transport state, one of the DSSI_TRANSPORT_STATE_*
     *       values defined above. */
    int  State;


    /** This field is valid when (Valid & DSSI_TRANSPORT_VALID_BPM)
     *  is true:
     *
     *  ---- The current tempo, in beats per minute.  */
    double Beats_Per_Minute;


    /** These six fields are valid when (Valid & DSSI_TRANSPORT_VALID_BBT)
     *  is true:
     *
     *  ---- The bar number at the beginning of the current process cycle. */
    unsigned long Bar;

    /** ---- The beat within that Bar. */
    unsigned long Beat;
    
    /** ---- The tick within that Beat. */
    unsigned long Tick;

    /** ---- The (possibly fractional) tick count since transport 'start'
     *       and the beginning of the current Bar. */
    double Bar_Start_Tick;

    /** ---- The number of beats per bar. */
    float  Beats_Per_Bar;

    /** ---- The number of ticks for each beat. */
    double Ticks_Per_Beat;

    /* [Sean says: I left out the 'beat_type' (time signature "denominator")
     * field of the jack_position_t structure, because I think it's useless
     * except to a notation program. Does anybody else feel like we need it?]
     */

    /** These two fields are valid when (Valid & DSSI_TRANSPORT_VALID_TIME)
     *  is true:
     *
     *  ---- The transport time at the beginning of the current process
     *       cycle, in seconds. */
    double  Current_Time;

    /** ---- The transport time at the beginning of the next process
             cycle, unless repositioning occurs. */
    double  Next_Time;

} DSSI_Transport_Info;

typedef struct _DSSI_Host_Descriptor DSSI_Host_Descriptor; /* below */

typedef struct _DSSI_Descriptor {

    /**
     * DSSI_API_Version
     *
     * This member indicates the DSSI API level used by this plugin.
     * All plugins must set this to 1 or 2.  The version 1 API contains
     * all DSSI_Descriptor fields through run_multiple_synths_adding(),
     * while the version 2 API adds the receive_host_descriptor().
     */
    int DSSI_API_Version;

    /**
     * LADSPA_Plugin
     *
     * A DSSI synth plugin consists of a LADSPA plugin plus an
     * additional framework for controlling program settings and
     * transmitting MIDI events.  A plugin must fully implement the
     * LADSPA descriptor fields as well as the required LADSPA
     * functions including instantiate() and (de)activate().  It
     * should also implement run(), with the same behaviour as if
     * run_synth() (below) were called with no synth events.
     *
     * In order to instantiate a synth the host calls the LADSPA
     * instantiate function, passing in this LADSPA_Descriptor
     * pointer.  The returned LADSPA_Handle is used as the argument
     * for the DSSI functions below as well as for the LADSPA ones.
     */
    const LADSPA_Descriptor *LADSPA_Plugin;

    /**
     * configure()
     *
     * This member is a function pointer that sends a piece of
     * configuration data to the plugin.  The key argument specifies
     * some aspect of the synth's configuration that is to be changed,
     * and the value argument specifies a new value for it.  A plugin
     * that does not require this facility at all may set this member
     * to NULL.
     *
     * This call is intended to set some session-scoped aspect of a
     * plugin's behaviour, for example to tell the plugin to load
     * sample data from a particular file.  The plugin should act
     * immediately on the request.  The call should return NULL on
     * success, or an error string that may be shown to the user.  The
     * host will free the returned value after use if it is non-NULL.
     *
     * Calls to configure() are not automated as timed events.
     * Instead, a host should remember the last value associated with
     * each key passed to configure() during a given session for a
     * given plugin instance, and should call configure() with the
     * correct value for each key the next time it instantiates the
     * "same" plugin instance, for example on reloading a project in
     * which the plugin was used before.  Plugins should note that a
     * host may typically instantiate a plugin multiple times with the
     * same configuration values, and should share data between
     * instances where practical.
     *
     * Calling configure() completely invalidates the program and bank
     * information last obtained from the plugin.
     *
     * Reserved and special key prefixes
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * The DSSI: prefix
     * ----------------
     * Configure keys starting with DSSI: are reserved for particular
     * purposes documented in the DSSI specification.  At the moment,
     * there is one such key: DSSI:PROJECT_DIRECTORY.  A host may call
     * configure() passing this key and a directory path value.  This
     * indicates to the plugin and its UI that a directory at that
     * path exists and may be used for project-local data.  Plugins
     * may wish to use the project directory as a fallback location
     * when looking for other file data, or as a base for relative
     * paths in other configuration values.
     *
     * The GLOBAL: prefix
     * ------------------
     * Configure keys starting with GLOBAL: may be used by the plugin
     * and its UI for any purpose, but are treated specially by the
     * host.  When one of these keys is used in a configure OSC call
     * from the plugin UI, the host makes the corresponding configure
     * call (preserving the GLOBAL: prefix) not only to the target
     * plugin but also to all other plugins in the same instance
     * group, as well as their UIs.  Note that if any instance
     * returns non-NULL from configure to indicate error, the host
     * may stop there (and the set of plugins on which configure has
     * been called will thus depend on the host implementation).
     * See also the configure OSC call documentation in RFC.txt.
     */
    char *(*configure)(LADSPA_Handle Instance,
		       const char *Key,
		       const char *Value);

    #define DSSI_RESERVED_CONFIGURE_PREFIX "DSSI:"
    #define DSSI_GLOBAL_CONFIGURE_PREFIX "GLOBAL:"
    #define DSSI_PROJECT_DIRECTORY_KEY \
	DSSI_RESERVED_CONFIGURE_PREFIX "PROJECT_DIRECTORY"

    /**
     * get_program()
     *
     * This member is a function pointer that provides a description
     * of a program (named preset sound) available on this synth.  A
     * plugin that does not support programs at all should set this
     * member to NULL.
     *
     * The Index argument is an index into the plugin's list of
     * programs, not a program number as represented by the Program
     * field of the DSSI_Program_Descriptor.  (This distinction is
     * needed to support synths that use non-contiguous program or
     * bank numbers.)
     *
     * This function returns a DSSI_Program_Descriptor pointer that is
     * guaranteed to be valid only until the next call to get_program,
     * deactivate, or configure, on the same plugin instance.  This
     * function must return NULL if passed an Index argument out of
     * range, so that the host can use it to query the number of
     * programs as well as their properties.
     */
    const DSSI_Program_Descriptor *(*get_program)(LADSPA_Handle Instance,
						  unsigned long Index);
    
    /**
     * select_program()
     *
     * This member is a function pointer that selects a new program
     * for this synth.  The program change should take effect
     * immediately at the start of the next run_synth() call.  (This
     * means that a host providing the capability of changing programs
     * between any two notes on a track must vary the block size so as
     * to place the program change at the right place.  A host that
     * wanted to avoid this would probably just instantiate a plugin
     * for each program.)
     * 
     * A plugin that does not support programs at all should set this
     * member NULL.  Plugins should ignore a select_program() call
     * with an invalid bank or program.
     *
     * A plugin is not required to select any particular default
     * program on activate(): it's the host's duty to set a program
     * explicitly.  The current program is invalidated by any call to
     * configure().
     *
     * A plugin is permitted to re-write the values of its input
     * control ports when select_program is called.  The host should
     * re-read the input control port values and update its own
     * records appropriately.  (This is the only circumstance in
     * which a DSSI plugin is allowed to modify its own input ports.)
     */
    void (*select_program)(LADSPA_Handle Instance,
			   unsigned long Bank,
			   unsigned long Program);

    /**
     * get_midi_controller_for_port()
     *
     * This member is a function pointer that returns the MIDI
     * controller number or NRPN that should be mapped to the given
     * input control port.  If the given port should not have any MIDI
     * controller mapped to it, the function should return DSSI_NONE.
     * The behaviour of this function is undefined if the given port
     * number does not correspond to an input control port.  A plugin
     * that does not want MIDI controllers mapped to ports at all may
     * set this member NULL.
     *
     * Correct values can be got using the macros DSSI_CC(num) and
     * DSSI_NRPN(num) as appropriate, and values can be combined using
     * bitwise OR: e.g. DSSI_CC(23) | DSSI_NRPN(1069) means the port
     * should respond to CC #23 and NRPN #1069.
     *
     * The host is responsible for doing proper scaling from MIDI
     * controller and NRPN value ranges to port ranges according to
     * the plugin's LADSPA port hints.  Hosts should not deliver
     * through run_synth any MIDI controller events that have already
     * been mapped to control port values.
     *
     * A plugin should not attempt to request mappings from
     * controllers 0 or 32 (MIDI Bank Select MSB and LSB).
     */
    int (*get_midi_controller_for_port)(LADSPA_Handle Instance,
					unsigned long Port);

    /**
     * run_synth()
     *
     * This member is a function pointer that runs a synth for a
     * block.  This is identical in function to the LADSPA run()
     * function, except that it also supplies events to the synth.
     *
     * A plugin may provide this function, run_multiple_synths() (see
     * below), both, or neither (if it is not in fact a synth).  A
     * plugin that does not provide this function must set this member
     * to NULL.  Authors of synth plugins are encouraged to provide
     * this function if at all possible.
     *
     * The Events pointer points to a block of EventCount ALSA
     * sequencer events, which is used to communicate MIDI and related
     * events to the synth.  Each event is timestamped relative to the
     * start of the block, (mis)using the ALSA "tick time" field as a
     * frame count. The host is responsible for ensuring that events
     * with differing timestamps are already ordered by time.
     *
     * See also the notes on activation, port connection etc in
     * ladpsa.h, in the context of the LADSPA run() function.
     *
     * Note Events
     * ~~~~~~~~~~~
     * There are two minor requirements aimed at making the plugin
     * writer's life as simple as possible:
     * 
     * 1. A host must never send events of type SND_SEQ_EVENT_NOTE.
     * Notes should always be sent as separate SND_SEQ_EVENT_NOTE_ON
     * and NOTE_OFF events.  A plugin should discard any one-point
     * NOTE events it sees.
     * 
     * 2. A host must not attempt to switch notes off by sending
     * zero-velocity NOTE_ON events.  It should always send true
     * NOTE_OFFs.  It is the host's responsibility to remap events in
     * cases where an external MIDI source has sent it zero-velocity
     * NOTE_ONs.
     *
     * Bank and Program Events
     * ~~~~~~~~~~~~~~~~~~~~~~~
     * Hosts must map MIDI Bank Select MSB and LSB (0 and 32)
     * controllers and MIDI Program Change events onto the banks and
     * programs specified by the plugin, using the DSSI select_program
     * call.  No host should ever deliver a program change or bank
     * select controller to a plugin via run_synth.
     */
    void (*run_synth)(LADSPA_Handle    Instance,
		      unsigned long    SampleCount,
		      snd_seq_event_t *Events,
		      unsigned long    EventCount);

    /**
     * run_synth_adding()
     *
     * This member is a function pointer that runs an instance of a
     * synth for a block, adding its outputs to the values already
     * present at the output ports.  This is provided for symmetry
     * with LADSPA run_adding(), and is equally optional.  A plugin
     * that does not provide it must set this member to NULL.
     */
    void (*run_synth_adding)(LADSPA_Handle    Instance,
			     unsigned long    SampleCount,
			     snd_seq_event_t *Events,
			     unsigned long    EventCount);

    /**
     * run_multiple_synths()
     *
     * This member is a function pointer that runs multiple synth
     * instances for a block.  This is very similar to run_synth(),
     * except that Instances, Events, and EventCounts each point to
     * arrays that hold the LADSPA handles, event buffers, and
     * event counts for each of InstanceCount instances.  That is,
     * Instances points to an array of InstanceCount pointers to
     * DSSI plugin instantiations, Events points to an array of
     * pointers to each instantiation's respective event list, and
     * EventCounts points to an array containing each instantiation's
     * respective event count.
     *
     * A host using this function must guarantee that ALL active
     * instances of the plugin are represented in each call to the
     * function -- that is, a host may not call run_multiple_synths()
     * for some instances of a given plugin and then call run_synth()
     * as well for others.  'All .. instances of the plugin' means
     * every instance sharing the same LADSPA label and shared object
     * (*.so) file (rather than every instance sharing the same *.so).
     * 'Active' means any instance for which activate() has been called
     * but deactivate() has not.
     *
     * A plugin may provide this function, run_synths() (see above),
     * both, or neither (if it not in fact a synth).  A plugin that
     * does not provide this function must set this member to NULL.
     * Plugin authors implementing run_multiple_synths are strongly
     * encouraged to implement run_synth as well if at all possible,
     * to aid simplistic hosts, even where it would be less efficient
     * to use it.
     */
    void (*run_multiple_synths)(unsigned long     InstanceCount,
                                LADSPA_Handle    *Instances,
                                unsigned long     SampleCount,
                                snd_seq_event_t **Events,
                                unsigned long    *EventCounts);

    /**
     * run_multiple_synths_adding()
     *
     * This member is a function pointer that runs multiple synth
     * instances for a block, adding each synth's outputs to the
     * values already present at the output ports.  This is provided
     * for symmetry with both the DSSI run_multiple_synths() and LADSPA
     * run_adding() functions, and is equally optional.  A plugin
     * that does not provide it must set this member to NULL.
     */
    void (*run_multiple_synths_adding)(unsigned long     InstanceCount,
                                       LADSPA_Handle    *Instances,
                                       unsigned long     SampleCount,
                                       snd_seq_event_t **Events,
                                       unsigned long    *EventCounts);

#if (DSSI_API_LEVEL > 1)

    /**
     * receive_host_descriptor()
     *
     * This member is a function pointer by which a host may provide
     * a plugin with a pointer to its DSSI_Host_Descriptor. Hosts
     * which provide host descriptor support must call this function
     * once per plugin shared object file, before any calls to
     * instantiate().
     *
     * NOTE: This field was added in version 2 of the DSSI API. Hosts
     * supporting version 2 must not access this field in a plugin
     * whose DSSI_API_Version is 1, and plugins supporting version 2
     * should behave reasonably under hosts (of any version) which do
     * not implement this function. A version 2 plugin that does not
     * provide this function must set this member to NULL.
     */
    void (*receive_host_descriptor)(const DSSI_Host_Descriptor *Descriptor);

#endif

} DSSI_Descriptor;

struct _DSSI_Host_Descriptor {

    /**
     * DSSI_API_Version
     *
     * This member indicates the DSSI API level used by this host.
     * All hosts must set this to 2.  Hopefully, we'll get this right
     * the first time, and this will never be needed.
     */
    int DSSI_API_Version;

    /**
     * request_transport_information()
     *
     * This member is a function pointer by which a plugin instance may
     * request that a host begin providing transport information (if
     * Request is non-zero), or notify the host that it no longer needs
     * transport information (if Request is zero).  Upon receiving a
     * non-zero request, the host should return a pointer to a
     * DSSI_Transport_Info structure if it is able to provide transport
     * information, or NULL otherwise.
     *
     * Once a plugin instance has received a non-null transport
     * information pointer, it may read from the structure at any time
     * within the execution of an audio class function (see doc/RFC.txt).
     * It should not consider the structure contents to be meaningful
     * while within a instantiation or control class function.  Also,
     * since the validity of fields within the structure may change
     * between each new invocation of an audio class function, a plugin
     * instance must check the Valid field of the structure accordingly
     * before using the structure's other contents.
     *
     * A host which does not support this function must set this member
     * to NULL.
     */
    DSSI_Transport_Info *
        (*request_transport_information)(LADSPA_Handle Instance,
                                         int           Request);

    /**
     * request_midi_send()
     *
     * This member is a function pointer that allows a plugin to
     * request the ability to send MIDI events to the host.
     *
     * While the interpretation of plugin-generated MIDI events is
     * host implementation specific, a mechanism exists by which a
     * plugin may declare to the host the number of destination
     * 'ports' and MIDI channels it can expect will be used in the
     * plugin-generated events.  Plugins which generate unchannelized
     * MIDI should supply zero for both Ports and Channels, otherwise
     * they should supply the maximum numbers for Ports and Channels
     * they expect to use.
     *
     * A plugin instance must call this function during instantiate().
     * [Sean says: this restriction seems reasonable to me, since
     * the host may need to create output ports, etc., and instantiate()
     * seems like a good place to do such things.  I'm sure I haven't
     * fully thought through all the details, though....]
     *
     * The host should return a non-zero value if it is able to
     * provide MIDI send for the plugin instance, otherwise it should
     * return zero, and the plugin instance may not subsequently call
     * midi_send().
     *
     * A host which does not support the MIDI send function must set
     * both this member and (*midi_send)() below to NULL.
     */
    int (*request_midi_send)(LADSPA_Handle Instance,
                             unsigned char Ports,
                             unsigned char Channels);

    /**
     * midi_send()
     *
     * This member is a function pointer by which a plugin actually
     * sends MIDI events to the host (provided it has received a non-
     * zero return from request_midi_send()). As in the run_synth()
     * functions, the Event pointer points to a block of EventCount
     * ALSA sequencer events.  The dest.port and data.*.channel fields
     * of each event are used to specify destination port and channel,
     * respectively, when the plugin is supplying channelized events.
     *
     * A plugin may only call this function from within the execution
     * of the audio class run_*() or select_program() functions. When
     * called from a run_*() functions, the events are timestamped
     * relative to the start of the block, (mis)using the ALSA "tick
     * time" field as a frame count. The plugin is responsible for
     * ensuring that events with differing timestamps are already
     * ordered by time, and that timestamps across multiple calls to
     * midi_send() from within the same run_*() invocation are
     * monotonic.  When midi_send() is called from within
     * select_program(), the timestamps are ignored, and the events
     * are considered to originate at the same frame time as the
     * select_program() call, if such a timing can be considered
     * meaningful.
     *
     * The memory pointed to by Event belongs to the plugin, and it is
     * the host's responsibility to copy the events as needed before
     * returning from the midi_send() call.
     *
     * A host which does not support the MIDI send function must set
     * both this member and (*request_midi_send)() above to NULL.
     */
    void (*midi_send)(LADSPA_Handle    Instance,
                      snd_seq_event_t *Event,
                      unsigned long    EventCount);

   /**
    * . . . additional fields could follow here, possibly supporting:
    *
    *   - a facility by which a plugin instance may request from a
    *       host a non-realtime thread in which to do off-line
    *       rendering, I/O, etc., thus (hopefully) avoiding the
    *       crashes that seem to occur when plugins create their own
    *       threads.  I got this idea after noticing that ZynAddSubFX
    *       achieves its gorgeous textures while remaining very
    *       responsive by doing a lot of non-real-time rendering.
    *       Several other uses for it have been mentioned on the DSSI
    *       list; I forget what.
    *
    *   - per-voice audio output
    */

    int (*request_non_rt_thread)(LADSPA_Handle Instance,
				 void (*RunFunction)(LADSPA_Handle Instance));
};

/**
 * DSSI supports a plugin discovery method similar to that of LADSPA:
 *
 * - DSSI hosts may wish to locate DSSI plugin shared object files by
 *    searching the paths contained in the DSSI_PATH and LADSPA_PATH
 *    environment variables, if they are present.  Both are expected
 *    to be colon-separated lists of directories to be searched (in
 *    order), and DSSI_PATH should be searched first if both variables
 *    are set.
 *
 * - Each shared object file containing DSSI plugins must include a
 *   function dssi_descriptor(), with the following function prototype
 *   and C-style linkage.  Hosts may enumerate the plugin types
 *   available in the shared object file by repeatedly calling
 *   this function with successive Index values (beginning from 0),
 *   until a return value of NULL indicates no more plugin types are
 *   available.  Each non-NULL return is the DSSI_Descriptor
 *   of a distinct plugin type.
 */

const DSSI_Descriptor *dssi_descriptor(unsigned long Index);
  
typedef const DSSI_Descriptor *(*DSSI_Descriptor_Function)(unsigned long Index);

/*
 * Macros to specify particular MIDI controllers in return values from
 * get_midi_controller_for_port()
 */

#define DSSI_CC_BITS			0x20000000
#define DSSI_NRPN_BITS			0x40000000

#define DSSI_NONE			-1
#define DSSI_CONTROLLER_IS_SET(n)	(DSSI_NONE != (n))

#define DSSI_CC(n)			(DSSI_CC_BITS | (n))
#define DSSI_IS_CC(n)			(DSSI_CC_BITS & (n))
#define DSSI_CC_NUMBER(n)		((n) & 0x7f)

#define DSSI_NRPN(n)			(DSSI_NRPN_BITS | ((n) << 7))
#define DSSI_IS_NRPN(n)			(DSSI_NRPN_BITS & (n))
#define DSSI_NRPN_NUMBER(n)		(((n) >> 7) & 0x3fff)

#ifdef __cplusplus
}
#endif

#endif /* DSSI_INCLUDED */
