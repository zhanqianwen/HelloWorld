//
//  SineWaveRecorder.cpp
//  MidiSmoother
//
//  Created by Nathan Holmberg on 12/05/15.
//  Copyright (c) 2015 Nathan Holmberg. All rights reserved.
//

#include "SineWaveRecorder.h"

#include <iostream>
#include <math.h>


const float SineWaveRecorder::kGain = 0.8f;

SineWaveRecorder::SineWaveRecorder(const std::string filename) :
mBytesWritten(0),
mSinePhase(0),
mPreviousVelocity(0)
/*
 * Constructor for VelocityConsumer.
 *
 * @param filename
 *		The filename to save output to
 */
{
	mOutputFile = fopen( filename.c_str(), "wb" );
	
	int byte_rate = kSampleRate * 4;
	unsigned char header_wav[] = { 'R', 'I', 'F', 'F',
		0, 0, 0, 0, // for the length to fill in later
		'W', 'A', 'V', 'E',
		'f', 'm', 't', ' ',
		0x10, 0, 0, 0, // fmt chunk size
		0x03, 0, // Float = 3
		(unsigned char)1, 0, // 1 channels recorded
		(unsigned char)(kSampleRate & 0xff), (unsigned char)(kSampleRate >> 8 & 0xff), (unsigned char)(kSampleRate >> 16 & 0xff), 0,
		(unsigned char)(byte_rate & 0xff), (unsigned char)(byte_rate >> 8 & 0xff), (unsigned char)(byte_rate >> 16 & 0xff), (unsigned char)(byte_rate >> 24 & 0xff),
		(unsigned char)(4), 0,
		(unsigned char)32, 0,
		'd', 'a', 't', 'a',
		0, 0, 0, 0,
	};
	fwrite(header_wav,sizeof(unsigned char),sizeof(header_wav),mOutputFile);
	
	
	mBaseSineStep = kBaseFrequency*(2.0*3.14159)/(kSampleRate*0.001);
	mcsvFile = fopen("out.csv", "wb");
}

SineWaveRecorder::~SineWaveRecorder()
/*
 * Destructor for the velocity consumer
 * This method will close and finalize the wav file.
 */
{
	int data_size = (int)mBytesWritten;
	int total_size = 36 + (int)mBytesWritten;
	if(mOutputFile)
	{
		fseek( mOutputFile, 4, SEEK_SET );
		fwrite( &total_size, sizeof( int ), 1, mOutputFile );
		fseek( mOutputFile, 40, SEEK_SET );
		fwrite( &data_size, sizeof( int ), 1, mOutputFile );
		fclose( mOutputFile );
		mOutputFile = NULL;
	}

	if (mcsvFile)
	{
		fclose(mcsvFile);
		mcsvFile = NULL;
	}

}

void SineWaveRecorder::RecordVelocity( double velocity, double for_time_ms)
/*
 * Creates new samples and records them to file for a given velocity and time step. The timestep determines how many samples 
 * to write
 * 
 * @param velocity
 *		the velocity that has been calculated for this period
 * 
 * @param for_time_ms
 *		the timestep (in milliseconds) that this velocity corresponds to
 */
{
	float samples[256];
	int num_samples = (int)(for_time_ms * kSampleRate* 0.001);
	double velocity_step = (velocity - mPreviousVelocity)/num_samples;
	double sine_step = mBaseSineStep * velocity;
	for( int i=0; i <  num_samples; i++ )
	{
		samples[i] = sinf( (float)mSinePhase )*kGain;
		samples[i] = fmod( samples[i], 2.0f * 3.14159f );
		sine_step += mBaseSineStep * velocity_step;
		mSinePhase += sine_step;
		mSinePhase = fmod( mSinePhase, 2.0f * 3.14159f );
	}
	fwrite(&samples[0],sizeof(float)*num_samples,1,mOutputFile);
	char str[200];
	sprintf(str, "%f,%f\n", for_time_ms, velocity);
	fwrite(&str, strlen(str), 1, mcsvFile);
	
	mBytesWritten += sizeof(float)*num_samples;
	
	mPreviousVelocity = velocity;
}
