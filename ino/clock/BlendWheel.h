
#ifndef _BLEND_WHEEL_H
#define _BLEND_WHEEL_H

#include "Wheel.h"

#define LEVELER_OFFSET (12) // how far off level the leveler gets

#define FADE_QUICK_TIME 250
#define FADE_SLOW_TIME 1000
#define FADE_NEVER -1

class BlendWheel: public Wheel {

public:

	char * status();
	void setWheel1(Wheel * wheel1);
	void setWheel2(Wheel * wheel2);
	Wheel * getWheel1();
	Wheel * getWheel2();
	void setFade(uint32_t startTime, int16_t duration);
	virtual color_t calculate(int dial);
	bool fadeComplete();

private:

	Wheel * wheel1, 
		  * wheel2;

	uint32_t fadeStartTime;
	uint16_t fadeDuration;
	bool complete;

	uint8_t lastTransition;
	uint32_t lastTransitionTime = 0;
};

#endif