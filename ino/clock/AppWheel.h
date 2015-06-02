#ifndef _APP_WHEEL_H
#define _APP_WHEEL_H


#include "Wheel.h"

class Photo: public Wheel {

public:
	virtual char * status();
	virtual void setPhoto(rgb_t * photo);
	virtual color_t calculate(int dial);

private:
	rgb_t * photo;
	
};

class ArtificialHorizon : public Wheel {

public:
	char * status();
	void setLevel(int16_t newLevel);
	virtual color_t calculate(int dial);

private:
	int16_t level;

};

#endif
