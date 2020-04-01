//
//  MidiSmoother.h
//  MidiSmoother
//
//  Created by Nathan Holmberg on 9/04/14.
//  Copyright (c) 2014 Nathan Holmberg. All rights reserved.
//
//#include "Lagrange.h"
//#include "Line.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>


#ifndef MidiSmoother_MidiSmoother_h
#define MidiSmoother_MidiSmoother_h

#define MaxNum 20
class MidiSmoother
{
public:
	MidiSmoother( int midi_values_per_revolution, double seconds_per_revolution );
	
	void NotifyMidiValue( char midi_value );
	
	double RequestMSToMoveValue( double ms_to_process ) const;
	
	void StartMidiProcessing();
	
	void StopMidiProcessing();
	
	bool MidiIsProcessing() const;
private:
	void MidiSmootherThreadFunction();

	void AddData(double X, double Y);
	double LinearRegression(double X);

	// These variables should not be modified to ensure things continue as necessary
	const int mMidiValuesPerRevolution; // the number of midi values that would need to be recieved for an entire platter revolution to be expected
	const double mSecondsPerRevolution; // the number of seconds an entire platter revolution represents
	bool mbMidiIsProcessing;

	//Add thread for midi processing. Don't block other thread
	std::mutex mThreadStartMutex;
	std::condition_variable mThreadStart;
	std::thread	mMidiSmoothThread;
	bool mbThreadRunning;
	std::chrono::steady_clock::time_point startTime;
	
	
private:
	// This is an example of the absolute simplest way of providing velocity.
	double mLastVelocity;
	double mX[MaxNum];
	double mFx[MaxNum];
	double a, b;
	int mNum;
	int mPos;
};

#endif
