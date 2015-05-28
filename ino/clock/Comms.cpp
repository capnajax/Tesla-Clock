/**
	This class defines the communications betweeb the bean and the service. There are two type of comms provided:
	a serial and a Bluetooth comm, with the same interface. Because the Serial comm is only meant to be used during
	debugging, the Bluetooth comm is allocated on startup, but the serial comm is allocated on demand.

	The Bluetooth comm will never be de-allocated, but is considered inactive when the serial comm is active.

	Because the serial comms are so unreliable, some commands are disabled with serial comms.
 */

#include "Comms.h"


BluetoothComms::BluetoothComms() {
	for(int i = 0; i < NUM_PHOTO_SEGMENTS; i++) {
		photoSegmentsLoaded[i] = false;
	}
}

command_t BluetoothComms::command() {

	command_t command;

	command_scratch_t scratch;
	scratch.scratch = Bean.readScratchData(SCRATCH_NUMBER_COMMAND);

	if(scratch.param.sequence == commandSequence) {

		command.command = 'N';

		if(photoCommandSequence) {

			photo_scratch_t pscratch;
			pscratch.scratch = Bean.readScratchData(SCRATCH_NUMBER_PHOTO_DATA);
			uint8_t segmentNum = pscratch.param.segment;

			// test that the sequence is within range
			if(segmentNum >= NUM_PHOTO_SEGMENTS) {
				// this is bad -- send "do not copy" message
				Bean.setScratchNumber(SCRATCH_NUMBER_PHOTO_STAT, segmentNum + 0x100);
			} else if(false == photoSegmentsLoaded[segmentNum]) {
				// this segment hasn't been loaded yet, so load it.
				photoSegmentsLoaded[segmentNum] = true;

				memcpy(&(pixelBuffer[segmentNum*PHOTO_SEGMENT_LENGTH]), 
					   &(pscratch.param.pixels), 
					   PHOTO_SEGMENT_LENGTH*sizeof(rgb_t));

				bool photoComplete = true;
				for(int i = 0; i < NUM_PHOTO_SEGMENTS; i++) {
					if(false == photoSegmentsLoaded[i]) {
						photoComplete = false;
					}
				}
				if(photoComplete) {
					command.command = 'P';
					command.pixels = pixelBuffer;
					Bean.setScratchNumber(SCRATCH_NUMBER_COPY,commandSequence);
				} else {
					Bean.setScratchNumber(SCRATCH_NUMBER_PHOTO_STAT,segmentNum);
				}

			} else {
				// duplicate segment, or reading the same scratch data. do nothing
			}

		}
	
	} else {

		if(photoCommandSequence) {
			// cancel the photo load and say "do not copy" on the photo message
			Bean.setScratchNumber(SCRATCH_NUMBER_DO_NOT_COPY, photoCommandSequence);
			photoCommandSequence = 0;

		}

		if(scratch.param.sequence - commandSequence > 1) {
			Bean.setScratchNumber(SCRATCH_NUMBER_DO_NOT_COPY, commandSequence-1);
		}
		commandSequence = scratch.param.sequence;

		command.command = scratch.param.name;
		bool copy=true;
		switch(scratch.param.name) {
		case 'A':
			// "are you there", essentially a no-op, but responds with a "copy" message
			command.command = 'N';
			break;
		case 'B':
			// normal operation
			break;
		// TODO how to handle K and F?
		case 'M':
			// mood light
			command.uint32Param = scratch.param.param.uint32Param;
			break;
		// no need for 'N', that doesn't come from the server
		case 'P':
			// begin loading photo
			command.command = 'N';	// responding with "no command" because we now have to load data
			photoCommandSequence = commandSequence = scratch.param.sequence;
			// reset the segments loaded
			for(int i = 0; i < NUM_PHOTO_SEGMENTS; i++) {
				photoSegmentsLoaded[i] = false;
			}
			// this will start the photo load on the next cycle.
			Bean.setScratchNumber(SCRATCH_NUMBER_PHOTO_STAT, 0xFF);
			break;
		case 'S':
			// get status. The status response will come via virtual serial
			break;
		case 'T':
			// set the time
			command.uint32Param = scratch.param.param.uint32Param;
			break;
		case 'W':
			// send a weather alert
			command.charParam = scratch.param.param.charParam;
			copy = false;
			for(int i = 0; i < WEATHER_TYPES_LENGTH; i++) {
				if(command.charParam == weatherTypes[i]) {
					copy = true;
				}
			}
			if(false == copy) {
				command.command = 'N';
				command.charParam = '\0';
			}
			break;
		default:
			command.command = 'N';
			copy = false;
			break;
		}

		/*
		Serial.write("S command");
		Serial.print(scratch.scratch.length);
		for(int i = 0; i < scratch.scratch.length; i++) {
			Serial.write((i==0)?": 0x":", 0x");
			Serial.print(scratch.scratch.data[i],HEX);
		}
		Serial.write(".\n");
		*/

		Bean.setScratchNumber(copy?SCRATCH_NUMBER_COPY:SCRATCH_NUMBER_DO_NOT_COPY, 
							  scratch.param.sequence);

	}

	return command;
};
