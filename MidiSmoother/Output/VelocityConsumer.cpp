//
//  VelocityConsumer.cpp
//  MidiSmoother
//
//  Created by Nathan Holmberg on 9/04/14.
//  Copyright (c) 2014 Nathan Holmberg. All rights reserved.
//

#include "VelocityConsumer.h"
#include <algorithm>
#include <iostream>




VelocityConsumer::VelocityConsumer( MidiSmoother& smoother, const std::string& output ) :
mMidiSmoother( smoother ),
mThreadStartMutex(),
mThreadStart(),
mConsumeThread(),
mbThreadRunning(false),
mSineWaveRecorder( output )
/*
 * Constructor for VelocityConsumer.
 * 
 * @param smoother
 *		The provided smoother that will be providing the midi values.
 */
{
}

VelocityConsumer::~VelocityConsumer()
/*
 * Destructor for the velocity consumer
 */
{
    Stop();
}


void VelocityConsumer::Start()
/*
 * Start the firing of midi events. This will continue asyncronously
 */
{

	mConsumeThread = std::thread( &VelocityConsumer::ConsumeThreadFunction, this );
	std::unique_lock<std::mutex> scoped_lock(mThreadStartMutex);
    mbThreadRunning = true;
    mThreadStart.notify_all();
}

void VelocityConsumer::Stop()
/*
 * Stop the firing of midi events. This will wait for the thread to end
 */
{
	{
		std::unique_lock<std::mutex> scoped_lock(mThreadStartMutex);
		mbThreadRunning = false;
		// ensure the thread has started
		mThreadStart.notify_all();
	}
	if( mConsumeThread.joinable() )
		mConsumeThread.join();
}

void VelocityConsumer::WaitForCompletion()
/*
 * Waits for the thread to naturally end (due to the consumption of all velocities).
 */
{
	if( mConsumeThread.joinable() )
		mConsumeThread.join();
}

void VelocityConsumer::RequestAndTrackVelocity( double ms_to_process )
/*
 * Requests a value from the midi smoother and will track the returned velocity for evaluation. Currently this means printing the velocity to std::cout
 *
 * @param ms_to_process
 *		The number of ms that we intend to step forwards.
 */
{
	double ms_to_step = mMidiSmoother.RequestMSToMoveValue( ms_to_process );
	double velocity = ms_to_step / ms_to_process;
    std::cout << velocity << "\n";
	//mX[mNum] = ms_to_process;
	//mY[mNum++] = velocity;
	mSineWaveRecorder.RecordVelocity(velocity, ms_to_process);
	
}

void VelocityConsumer::ConsumeThreadFunction()
/*
 * Static function run by the thread. This is responsible for requesting velocity in some pattern
 *
 * @param context
 *		The provided threading context. In this case it is an instance of a VelocityConsumer
 */
{
    
    // wait for the explicit start message
	{
		std::unique_lock<std::mutex> scoped_lock(mThreadStartMutex);
		if( !mbThreadRunning ){
			mThreadStart.wait( scoped_lock );
		}
	}
    
	// determine our request frequency
	const int iterations_per_block = 7;
	const double ms_request_per_iteration = 32/44.1;
	const int total_interval_microseconds = (int)(ms_request_per_iteration*iterations_per_block*1000);
	// continue asking until we are told to stop or there is no more midi
	while( mbThreadRunning && mMidiSmoother.MidiIsProcessing())
    {
		// request a block of updates
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		for( int i=0;i<iterations_per_block;i++ )
		{
			RequestAndTrackVelocity( ms_request_per_iteration );
		}
		
		// calculate how long it took us to get here
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::chrono::duration<int,std::micro> duration_micro = std::chrono::duration_cast< std::chrono::duration<int,std::micro> >(end-start);
		// and so calculate how long we should sleep for now
		long long microseconds = std::max((long long)0,(long long)(total_interval_microseconds - duration_micro.count() ) );
        std::chrono::microseconds interval = std::chrono::microseconds( microseconds ) ;
		// and sleep
        std::this_thread::sleep_for( interval );
    }
}