//
//  VelocityConsumer.h
//  MidiSmoother
//
//  Created by Nathan Holmberg on 9/04/14.
//  Copyright (c) 2014 Nathan Holmberg. All rights reserved.
//

#ifndef __MidiSmoother__VelocityConsumer__
#define __MidiSmoother__VelocityConsumer__

#include <thread>
#include <mutex>
#include <condition_variable>

#include "MidiSmoother.h"
#include "SineWaveRecorder.h"

#define MAX_NUM 2048

class VelocityConsumer {
public:
	VelocityConsumer( MidiSmoother& smoother, const std::string& output );
	
	~VelocityConsumer();
	
    void Start();
	
    void Stop();
	
	void WaitForCompletion();
private:
    void ConsumeThreadFunction( );
	
	void RequestAndTrackVelocity( double ms_to_process );
	
    MidiSmoother& mMidiSmoother;
    
	int mNum;
	double mX[MAX_NUM];
	double mY[MAX_NUM];
    std::mutex mThreadStartMutex;
    std::condition_variable mThreadStart;
    std::thread	mConsumeThread;
    bool mbThreadRunning;
	
	SineWaveRecorder mSineWaveRecorder;
};

#endif /* defined(__MidiSmoother__VelocityConsumer__) */
