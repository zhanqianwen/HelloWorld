//
//  SineWaveRecorder.h
//  MidiSmoother
//
//  Created by Nathan Holmberg on 12/05/15.
//  Copyright (c) 2015 Nathan Holmberg. All rights reserved.
//

#ifndef __MidiSmoother__SineWaveRecorder__
#define __MidiSmoother__SineWaveRecorder__

#include <string>

class SineWaveRecorder
{
public:
	SineWaveRecorder(const std::string filename);
	~SineWaveRecorder();
	
	void RecordVelocity( double velocity, double for_time_ms);
private:
	static const int kSampleRate = 48000;
	static const int kBaseFrequency = 1;
	static const float kGain;
	
	FILE* mOutputFile;
	FILE* mcsvFile;
	unsigned long mBytesWritten;
	
	double mBaseSineStep;
	double mSinePhase;
	double mPreviousVelocity;
};

#endif /* defined(__MidiSmoother__SineWaveRecorder__) */
