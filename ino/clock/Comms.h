/**
	This class defines the communications betweeb the bean and the service. There are two type of comms provided:
	a serial and a Bluetooth comm, with the same interface. Because the Serial comm is only meant to be used during
	debugging, the Bluetooth comm is allocated on startup, but the serial comm is allocated on demand.

	The Bluetooth comm will never be de-allocated, but is considered inactive when the serial comm is active.

	Because the serial comms are so unreliable, some commands are disabled with serial comms.
 */

/*
	The following commands are supported

	A - acknowledge an "Are you there?" message
	B - for normal color cycling
	F - means cannot connect (eg a recent AYT message failed)
	K - send an "Are you there?" message
	M(color_t) - set the mood light -- elevates the color cycling by a certain colour value
	N - null, means there is no command available.
	P(rgb_t[84]) - photo -- not supported by the serial comms
	S - request a status
	T(uint32_t) - set the time
	W(char) - weather alert. The addition parameter can be 'R'ain, 'A'lert, 'W'arning
 
	This also sends "copy" messages via scratch 3 and "do not copy" messages via scratch 4. The messages for this
	are simply the sequence number of the command.

	Scratch 5 is used to acknowledge photo segments 0 to 13 for success, 256+(0 to 13) for failure

 */

#ifndef _COMMS_H
#define _COMMS_H

#define NUM_PHOTO_SEGMENTS 14
#define PHOTO_SEGMENT_LENGTH 6

#define SCRATCH_NUMBER_COMMAND 1
#define SCRATCH_NUMBER_PHOTO_DATA 2
#define SCRATCH_NUMBER_COPY 3
#define SCRATCH_NUMBER_DO_NOT_COPY 4
#define SCRATCH_NUMBER_PHOTO_STAT 5

#include <Arduino.h>
#include "Wheel.h"

typedef struct {
	uint8_t command;
	union {
		uint32_t uint32Param; // store for a command that takes a uint32_t param
		char charParam; // store for a command that takes a char param
		color_t colorParam; // store for a command that takes a color as a param
		rgb_t * pixels;
	};
} command_t;

typedef union {
	ScratchData scratch;
	struct {
		uint8_t length;
		uint16_t sequence;
		uint8_t check;
		char name;
		union {
			uint32_t uint32Param; // store for a command that takes a uint32_t param
			char charParam; // store for a command that takes a char param
			color_t colorParam; // store for a command that takes a color as a param
		} param;
	} param;
} command_scratch_t;

typedef union {
	ScratchData scratch;
	struct {
		uint8_t length;
		uint8_t segment;
		uint8_t check;
		rgb_t pixels[PHOTO_SEGMENT_LENGTH];
	} param;
} photo_scratch_t;

#define WEATHER_TYPES_LENGTH 3
const char weatherTypes[WEATHER_TYPES_LENGTH] = {'R','A','W'};

class Comms  {

public:

	virtual command_t command() = 0;

protected:

	union {
		uint32_t uint32Param; // store for a command that takes a uint32_t param
		char charParam; // store for a command that takes a char param
		color_t colorParam; // store for a command that takes a color as a param
		rgb_t photoBuffer[NUM_PHOTO_SEGMENTS*PHOTO_SEGMENT_LENGTH];
	};

private:

	Comms * bluetoothComms = NULL;
	Comms * serialComms = NULL;

};

/**
	The bluetooth comms use the scratch charateristics to receive commands. Scratch 1 is for the command names and for 
	single-integer commands, and scratch 2 is for sending data such as photos.

	The command scratch (1) is in this format:
	0-1 -- a command number. Each command has a number. If I receive a command out of sequence I can re-request it.
	2 -- a check, this is a simple XOR of all the bytes that follow.
	3 -- command name
	4-7 -- command parameter

	The photo data scratch (2) is in this format
	0 -- segment number. This is sequence of the photo segment, numbered 1-14. Each segment has 6 pixels.
	1 -- check, this is a simple XOR of all the bytes the follow.
	2-19 -- the pixel data

	Logging is still send via serial
 */
class BluetoothComms : public Comms {

public:
	BluetoothComms();
	command_t command();

private:

	// used to detect if a command was missed.
	uint16_t commandSequence = 0;
	uint16_t photoCommandSequence = 0; // the sequence number of a command that started a photo load.
	bool photoSegmentsLoaded[NUM_PHOTO_SEGMENTS];

	// to load the photo wheel
	rgb_t pixelBuffer[NUM_PHOTO_SEGMENTS*PHOTO_SEGMENT_LENGTH];
};

#endif