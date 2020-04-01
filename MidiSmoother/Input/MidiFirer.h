//
//  MidiFirer.h
//  MidiFirer
//
//  Created by Nathan Holmberg on 8/04/14.
//  Copyright (c) 2014 Nathan Holmberg. All rights reserved.
//

#ifndef __MidiFirer__MidiFirer__
#define __MidiFirer__MidiFirer__

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "MidiSmoother.h"

class MidiFirer
{
public:
    MidiFirer( MidiSmoother& smoother );
    ~MidiFirer();
	void LoadMidiDataFromStream( std::istream& stream );
    void Start();
    void Stop();
    void WaitForCompletion();
private:
    struct MidiEvent
    {
        MidiEvent( char midi_value, double interval );
        MidiEvent( const MidiEvent& in_event );
        char midi_value;
        double interval;
    };
private:
    void FireThreadFunction();
    MidiSmoother& mMidiSmoother;
    
    std::mutex mThreadStartMutex;
    std::condition_variable mThreadStart;
    std::thread	mFireThread;
    bool mbThreadRunning;
    
    std::vector<MidiEvent> mMidiEvents;
};

#endif /* defined(__MidiFirer__MidiFirer__) */
