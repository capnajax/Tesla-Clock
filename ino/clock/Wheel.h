
#ifndef _WHEEL_H
#define _WHEEL_H

#include <Arduino.h>

#define NUM_PIXELS 84

#define HOUR_TOP (6)
#define MINUTE_TOP (-7)

typedef struct {
	// these are reversed in order against c
	uint8_t b;
	uint8_t g;
	uint8_t r;
} rgb_t;

typedef union {
	uint32_t c;
	rgb_t rgb;
} color_t;

class Wheel {

public:

	virtual void setTime(uint32_t newTime);
	virtual void setBrightness(uint8_t newBrightness);
	virtual color_t calculate(int dial);
	virtual char * status();
	uint8_t dialPosition(uint8_t wheelPos);

protected:

	virtual color_t hue(uint8_t position, uint8_t offset, uint8_t brightness);
	color_t color(uint8_t r, uint8_t g, uint8_t b);
	color_t color(uint8_t w);
	color_t color(rgb_t rgb);

	uint32_t rawTime; 
	uint8_t brightness,
			ispm, 		// is afternoon
			hour,
			minute,
			second,
			centi; // 1/100th of a second

};


class WheelBright: public Wheel {

public:

	virtual char * status();
	virtual color_t calculate(int dial);

};

class WheelDark: public Wheel {

public:

	virtual char * status();
	virtual color_t calculate(int dial);

};

class WheelMood: public WheelBright {

public:

	virtual char * status();
	virtual void setMood(color_t newMood);

private:

	color_t mood;

	virtual color_t calculate(int dial);

};

#endif