MidiSmoother
------------

INTRODUCTION
------------

One of the ways that a user can interact with music using our DJ products is through scratching. On a MIDI controller, much like a mouse, we only get updates when something changes and only to the level of accuracy and timing that the MIDI controller provides. This means the data is not at all continuous, regular, or smooth. When a user moves the platter back and forth we receive MIDI updates and must interpret them to see how far the user moved.

For an example of how a user might use one of these platters, check out Matt's left hand in this video: https://www.youtube.com/watch?v=AR-T5wnk2VE#t=145

On the other side of the equation is our audio. Humans are incredibly good at picking up issues and problems with audio and any unexpected glitch, sudden change in velocity, or discontinuity at all will sound terrible and our expert users and Product Owner's will pick it up and that is no good! The obvious solution is to make it REAL smooth but this then introduces 'latency' where there is a difference between the time an action was performed by the user and when they get to hear it. This is no good either!

So there must be a tradeoff between smoothness and latency, and ideally there are smart ways of being smooth without adding as much latency as other smoothing methods.


TASK OVERVIEW
-------------

For the purposes of this task you can ignore the fact that it is 'midi' and 'audio'. Really this is just a asynchronous, messy input that needs to be smoothed and delivered in consistently timed increments.

The task here is to (re)implement the MidiSmoother class so that it performs this task and provides data that is 'as smooth as possible'. How smooth it needs to be is a soft requirement that has no specific answer but should have no obvious discontinuities, unnecessary 'wobbles' or artifacts. In terms of latency, our real system introduces ~40ms for this particular controller and is very smooth. For reference, 100ms would be considered far too much and 10ms would be amazing but (we believe) is mathematically impossible to be smooth given the input data.

Please treat the input and output as black boxes! These represent other systems and so shouldn't be changed.

ENVIRONMENT
-----------

This task requires C++11 support and therefore will need Visual Studio 2012 or greater on Windows. On Mac we recommend using Xcode 5 or greater.

While there are some files that are marked 'do not change' within the project, you are free to add new files if you believe it is appropriate.

TASK INPUT
----------

Supplied is a very simple test application that reads MIDI data in and feeds it to a smoother (that you will have to make better) at particular intervals. The audio side is represented by another class which will request velocity values that would normally be applied to audio. These two tasks happen asynchronously and from different threads.

The MIDI data is held in a csv file (which can be opened in Excel etc) in the format:

INTERVAL_SINCE_LAST_MESSAGE, MIDI_VALUE

The midi values are deltas from the previous position so a positive midi value means going forwards and a negative value means going backwards. These deltas are in 'ticks' around the outside of the platter, of which there are a fixed number. So if the MIDI_VALUE is 20 and there are 2048 ticks in total, then we can say the wheel moved 20/2048 revolutions in INTERVAL_SINCE_LAST_MESSAGE. 

FYI: This data including timings are real and were measured using one of the controllers we support.

midi_data_sz.csv: This file has data from a 'smooth' ramp down where the platter was spun and allowed to slowly come to a stop
midi_data_sz_scratch.csv: This file has data from a 'scratch' where the platter was moved backwards and forwards manually.

TASK OUTPUT
-----------

Printed to the console are a series of 'steps' where each step is a position change that could be added to the current playhead. The distribution of these is even and each represents 0.72ms of audio (i.e. we apply a step every 0.72ms). These should be as smooth as possible without introducing too much latency. 

What is too much? And how smooth? That is kind of up to you but it is highly unlikely that two consecutive velocities would be exactly the same or that there could be a very sharp change within one step.

What is smooth? 
The input is a continuous function which has been made discrete by a system we have no control over. The output is also discrete but with a different sampling frequency. Ideally the solution will appear as continuous as possible!

What is latent?
The solution we ended up using has a calculated 22ms of latency. The solution it replaced had 60ms which was considered 'too much'. These figures are guidelines only and shouldn't be seen as targets.

There is also an audible output created as 'output.wav' in the working directory when run. This shows how the algorithm would sound when applied to a 1kHz sine wave and should make wobbles in the audio apparent.

UNITS OF MEASUREMENT
--------------------
It is important to note that in this problem both time and distance are measured in seconds. Time is in the normal elapsed-time as one would expect. Distance is in seconds through the 'song' that should be moved, so the units are seconds even if they are measuring a distance.

Example:
If the velocity is 0.5, this means for every second of 'actual' time we move 0.5 seconds through the song.

Midi ticks are a in angular distance which can be converted into a seconds (distance) value as described above.

Velocity is the ratio of these two (seconds as distance and seconds as time). The solution will need to use this.

APPENDIX
--------

While this task doesn't really require one to know how MIDI and Audio work, if you are interested there are some online resources that explain how the message format works and what soundcards do.

An introduction to the MIDI message can be found here: http://en.wikipedia.org/wiki/MIDI#Technical_specificationsfor our purposes it is enough to say that a platter sends a midi CC message where the first two bytes tell us it is the platter and the third is a 7bit signed integer we interpret as the magnitude of the change.

Audio systems work by receiving a request for so many 'samples' of audio to be delivered to the soundcard. Often this is a multiple of 32 samples which doesn't divide easily into ms. 32 samples in a 44,100Hz soundcard is 0.72ms, hence the funny number above.

