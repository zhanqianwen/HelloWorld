//
//  main.cpp
//  MidiFirer
//
//  Created by Nathan Holmberg on 8/04/14.
//  Copyright (c) 2014 Nathan Holmberg. All rights reserved.
//

#include <iostream>
#include <fstream>

#include "Input/MidiFirer.h"
#include "Output/VelocityConsumer.h"

void PrintUsage( const char* binary_name )
/*
 * Prints the usage for this application
 *
 * @param binary_name
 *		The name of the current binary
 */
{
	std::cout << "Usage: " << binary_name << " <midi_file>" << std::endl;
	exit(-1);
}

int main(int argc, const char * argv[])
{
	if( argc < 2 )
	{
		PrintUsage( argv[0]);
	}

	std::string output = (argc >= 3) ? argv[2] : "output.wav";

	// The values for the smoother are from the real world. This particular device has 2048 'clicks' around it's wheel
	// and all devices have one revolution is 1.8 seconds (it's a DJ thing)
	MidiSmoother smoother( 2048, 1.8 );
    MidiFirer firer( smoother );
    VelocityConsumer consumer( smoother, output );
	
	// Load MIDI data from the supplied file argument
	{
		std::ifstream filestream = std::ifstream(argv[1]);
		
		if( !filestream )
		{
			std::cout << "Failed to open file " << argv[1] << std::endl;
			exit(-1);
		}
	
		firer.LoadMidiDataFromStream(filestream);
	}
    
	// start the firer and consumer
    firer.Start();
	consumer.Start();
    firer.WaitForCompletion();
    consumer.WaitForCompletion();
    
    return 0;
}

