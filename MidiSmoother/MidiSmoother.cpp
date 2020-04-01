//
//  MidiSmoother.cpp
//  MidiSmoother
//
//  Created by Nathan Holmberg on 9/04/14.
//  Copyright (c) 2014 Nathan Holmberg. All rights reserved.
//

#include "MidiSmoother.h"

#include <iostream>
#include <complex>
#include <math.h>
#include <thread>
#include <algorithm>
#include <cassert>
#include <vector>

#define PI acos(-1)




MidiSmoother::MidiSmoother(int midi_values_per_revolution, double seconds_per_revolution) :
    mMidiValuesPerRevolution(midi_values_per_revolution),
    mSecondsPerRevolution(seconds_per_revolution),
    mbMidiIsProcessing(false),
    mThreadStartMutex(),
    mThreadStart(),
    mMidiSmoothThread(),
    mbThreadRunning(false),
    mNum(0), //indicate the last input
    mPos(0), //indicate the last output
    mLastVelocity(0.0)
    /*
     * Constructor for a Midi Smoother.
     *
     * @param midi_values_per_revolution
     *		The number of midi values that would need to be recieved for an entire platter revolution to be expected
     * @param seconds_per_revolution
     *		The number of seconds an entire platter revolution represents
     */
{

}

void MidiSmoother::StartMidiProcessing()
/*
 * Indicates that the midi processing is about to begin!
 */
{
    mbMidiIsProcessing = true;

    std::memset(mX, 0, sizeof(mX));
    std::memset(mFx, 0, sizeof(mFx));
    startTime = std::chrono::steady_clock::now();
    mMidiSmoothThread = std::thread(&MidiSmoother::MidiSmootherThreadFunction, this);

    std::unique_lock<std::mutex> scoped_lock(mThreadStartMutex);
    mbThreadRunning = true;
    mThreadStart.notify_all();
    
}

void MidiSmoother::StopMidiProcessing()
/*
 * Indicates that there is no more midi coming!
 */
{
    mbMidiIsProcessing = false;

    
    {
        std::unique_lock<std::mutex> scoped_lock(mThreadStartMutex);
        mbThreadRunning = false;
        // ensure the thread has started
        mThreadStart.notify_all();
    }
    if (mMidiSmoothThread.joinable())
        mMidiSmoothThread.join();
	
}

bool MidiSmoother::MidiIsProcessing() const
{
	return mbMidiIsProcessing;
}

void MidiSmoother::AddData(double X, double Y)
/*
 * Problem: for given vectors x and y, (x is the input, y is the label). 
 * Find a optimal line that best predicts future y with future x.
 * We need to find the line: y = bx + a which best fits the pattern of the data dots.
 *
 * @param X, Y
 *		X is the input, Y is the label
 */
{
   
    //For Linear Regression
    //printf("X=: %f \n", X);
    mX[mPos] = X;
    mFx[mPos++] = Y;
    if (mPos >= MaxNum-1) mPos = 0;
    mNum = mNum < MaxNum ? mNum + 1 : mNum;
    /*
    * In order to calculate the ground truth for the slope and intercept, 
    * we need to run the following calculation for all training data.
    * b --> slope = (N * Sum(x_i * y_i) - Sum(x_i) * Sum(y_i)) / (N * Sum(x_i^2) - Sum(x_i)^2)
    * a --> intercept = (Sum(y_i) - b * Sum(x_i)) / N
    */
    if (mNum > 1)
    {
        double _x = 0, _y = 0;

        for (int i = 0; i < mNum; i++)
            _x += mX[i], _y += mFx[i];
        _x /= (double)mNum; _y /= (double)mNum;
        double up = 0, down = 0;
        for (int i = 0; i < mNum; i++)
        {
            up += (mX[i] - _x) * (mFx[i] - _y);
            down += (mX[i] - _x) * (mX[i] - _x);
        }
        b = up / down;
        a = _y - b * _x;
    }
    
}



double MidiSmoother::LinearRegression(double X)
/*
 * If the goal is prediction, forecasting, or error reduction, linear regression can be used to fit 
 * a predictive model to an observed data set of values of the response and explanatory variables. 
 * After developing such a model, if additional values of the explanatory variables are collected 
 * without an accompanying response value, the fitted model can be used to make a prediction of the
 * response
 *
 * @param midi_value
 *		X is the input.
 * @return
 *		The F (x) corresponding to the X coordinate (midi value corresponding Seconds).
 */
{
    return b * X + a;
}

void MidiSmoother::NotifyMidiValue( char midi_value )
/*
 * Notify the smoother that a new value has been received from the platter.
 *
 * @param midi_value
 *		The number of midi values that has passed. This can be negative indicating reverse direction.
 */
{
    // printf("NotifyMidiValue: %d \n", midi_value);
	// NB: This is incorrect! This assumes that we recieve a midi value every millisecond which isn't true
    //mLastVelocity = midi_value/(double)mMidiValuesPerRevolution * mSecondsPerRevolution * 1000;
    
    mbMidiIsProcessing = true;
    std::lock_guard<std::mutex> lk(mThreadStartMutex);
    // Use lock_guard to lock the signal, insert the data 
    std::chrono::steady_clock::time_point currentT = std::chrono::steady_clock::now();
    std::chrono::duration<int, std::micro> duration_m = std::chrono::duration_cast<std::chrono::duration<int, std::micro>>(currentT - startTime);
    double X = double(duration_m.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
    X = X / 1000;
    
    double Y = midi_value / (double)mMidiValuesPerRevolution * mSecondsPerRevolution * 1000;
    AddData(X, Y);

    mThreadStart.notify_one();
	
}

double MidiSmoother::RequestMSToMoveValue( double ms_to_process ) const
/*
 * Request a ms to move value from the smoother. This is so the audio engine would know how much to step forward to produce the appropriate pitch for what the platter is doing.
 *
 * @param ms_to_process
 *		The number of ms we are calculating this step for. 
 * @return
 *		The number of ms that should be moved during this process step
 */
{
	return mLastVelocity * ms_to_process;
}


void MidiSmoother::MidiSmootherThreadFunction()
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
        if (!mbThreadRunning) {
            mThreadStart.wait(scoped_lock);
        }

    }

    // determine our request frequency
    const int iterations_per_block = 7;
    const double ms_request_per_iteration = 30 / 44.1;
    const int total_interval_microseconds = (int)(ms_request_per_iteration * iterations_per_block * 1000);
    // continue caculating the F(Xi)
    while (mbThreadRunning )
    {
        // request a block of updates
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        std::chrono::duration<int, std::micro> duration_m = std::chrono::duration_cast<std::chrono::duration<int, std::micro>>(start - startTime);
        double curSeconds = double(duration_m.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
        curSeconds = curSeconds / 1000;
        //process data;
        if (mNum <= 0)
        {
            mLastVelocity = 0;
        }
        else if (mNum < 3)
        {
            mLastVelocity = mFx[mNum - 1];
        }
        else
        {
            //mLastVelocity = LagrangeInter(curSeconds);
            mLastVelocity = LinearRegression(curSeconds);
            //AddData(curSeconds, mLastVelocity);
        }
        
        
        // calculate how long it took us to get here
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::chrono::duration<int, std::micro> duration_micro = std::chrono::duration_cast<std::chrono::duration<int, std::micro>>(end - start);
        // and so calculate how long we should sleep for now
        long long microseconds = std::max((long long)0, (long long)(total_interval_microseconds - duration_micro.count()));
        std::chrono::microseconds interval = std::chrono::microseconds(microseconds);
        // and sleep
        std::this_thread::sleep_for(interval);

    }
}
