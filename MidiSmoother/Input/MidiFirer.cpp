//
//  MidiFirer.cpp
//  MidiFirer
//
//  Created by Nathan Holmberg on 8/04/14.
//  Copyright (c) 2014 Nathan Holmberg. All rights reserved.
//

#include "MidiFirer.h"

#include <sstream>
#include <chrono>

MidiFirer::MidiEvent::MidiEvent( char midi_value, double interval ) :
midi_value(midi_value),
interval(interval)
/*
 * Constructor for a Midi Event.
 * @param midi_value
 *		The midi value to send with this event
 * @param interval
 *		Time since the previous midi event
 */
{}

MidiFirer::MidiEvent::MidiEvent( const MidiEvent& in_event ) :
midi_value(in_event.midi_value),
interval(in_event.interval)
/*
 * Copy Constructor for a Midi Event.
 * @param in_event
 *		The event to copy values from
 */
{}

MidiFirer::MidiFirer( MidiSmoother& smoother ) :
mMidiSmoother( smoother ),
mThreadStartMutex(),
mThreadStart(),
mFireThread(),
mbThreadRunning(false)
/*
 * Constructor for a Midi Firer class that will produce Midi and provide it to the smoother
 */
{
}

MidiFirer::~MidiFirer()
/* 
 * Midi Firer destructor
 */
{
    Stop();
}

void MidiFirer::LoadMidiDataFromStream( std::istream& stream )
/*
 * Loads a set of Midi events from a well constructed input stream. 
 * The expected format is a two column csv (with no column names). 
 * The first column is the interval since the previous event (as a double).
 * The second column is the value associated with the event
 *
 * @param stream
 *		The stream to read
 */
{
	// ensure the thread doesn't start before data has been read (and that it hasn't already started
	std::unique_lock<std::mutex> lock (mThreadStartMutex);
	if( mbThreadRunning )
		return;
	
	double interval;
	int midi_value;
	char comma;
	std::string tab_delim_line;
	while( !stream.eof() )
	{
		// read and parse each line
		std::getline( stream, tab_delim_line );
		std::istringstream string_stream( tab_delim_line );
		string_stream >> interval;
		string_stream >> comma;
		string_stream >> midi_value;
		// push the event into the back of the list of midi events
		mMidiEvents.push_back( MidiEvent( static_cast<char>(midi_value), interval ) );
	}
}

void MidiFirer::Start()
/* 
 * Start the firing of midi events. This will continue asyncronously
 */
{
	mFireThread = std::thread( &MidiFirer::FireThreadFunction, this );
	std::unique_lock<std::mutex> lock (mThreadStartMutex);
    mbThreadRunning = true;
    mThreadStart.notify_all();
    
}

void MidiFirer::Stop()
/*
 * Stop the firing of midi events. This will wait for the thread to end
 */
{
	{
		std::unique_lock<std::mutex> lock (mThreadStartMutex);
		mbThreadRunning = false;
		// ensure the thread has started
		mThreadStart.notify_all();
	}
	if( mFireThread.joinable())
		mFireThread.join();
    
}

void MidiFirer::WaitForCompletion()
/*
 * Waits for the thread to naturally end (due to the consumption of all midi).
 */
{
	if( mFireThread.joinable())
		mFireThread.join();
}


void MidiFirer::FireThreadFunction()
/*
 * Static function run by the thread. This is responsible for the consumption of midi events
 *
 * @param context
 *		The provided threading context. In this case it is an instance of a MidiFirer
 */
{   
    // wait for the explicit start message
	{
		std::unique_lock<std::mutex> lock (mThreadStartMutex);
		if( !mbThreadRunning ){
			mThreadStart.wait( lock );
		}
	}
    
	// iterate over all events
    std::vector<MidiEvent>::const_iterator event_it = mMidiEvents.begin();
    std::vector<MidiEvent>::const_iterator event_it_end = mMidiEvents.end();
	
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	mMidiSmoother.StartMidiProcessing();
    while( mbThreadRunning && event_it != event_it_end )
    {
		// determine how long we took to send the last event
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::chrono::duration<int,std::micro> duration_micro = std::chrono::duration_cast< std::chrono::duration<int,std::micro> >(end-start);
		
        // first sleep for the required interval
        std::chrono::microseconds interval = std::chrono::microseconds(static_cast<long long>((*event_it).interval * 1000000) - duration_micro.count() ) ;
        std::this_thread::sleep_for( interval );
		start = std::chrono::steady_clock::now();
		
		// then send the event
        mMidiSmoother.NotifyMidiValue( (*event_it).midi_value );
        event_it++;
    }
	mMidiSmoother.StopMidiProcessing();
}