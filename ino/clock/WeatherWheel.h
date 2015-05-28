
#ifndef _WEATHER_WHEEL_H
#define _WEATHER_WHEEL_H

#include "Wheel.h"

class WheelRain: public Wheel {

public:
	virtual char * status();
	virtual color_t calculate(int dial);
};


class WheelAlert : public Wheel {
public:
	virtual char * status();
	virtual color_t calculate(int dial);
};

class WheelWarning : public Wheel {
public:
	virtual char * status();
	virtual color_t calculate(int dial);
};

#endif
