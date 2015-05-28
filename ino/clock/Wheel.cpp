
#include "Wheel.h"

//
//  NOTE TO DEVELOPERS
//
//  These classes and their derived classes use a few terms very specifically:
//
//  "degrees" - These degrees are different, here a degree is 1/240th of a circle, not 1/360th.
//
//  "zero-point" - red's zero-point is 0, green is at 80 "degrees", and blue is at 240 "degrees"


char * Wheel::status() {
    return "Wheel";
}

void Wheel::setBrightness(uint8_t newBrightness) {
	brightness = newBrightness;
}

uint8_t Wheel::dialPosition(uint8_t wheelPos) {

	// TODO this can be replaced by a lookup table

    if(wheelPos < 24) {
        return (120+wheelPos+HOUR_TOP) % 24;
    } else {
        return (60+wheelPos+MINUTE_TOP-24) % 60 + 24;
    }

}

color_t Wheel::color(uint8_t r, uint8_t g, uint8_t b) {
    color_t result;
    result.rgb.r = r;
    result.rgb.g = g;
    result.rgb.b = b;
    return result;
}

color_t Wheel::color(uint8_t w) {
    color_t result;
    result.rgb.r = result.rgb.g = result.rgb.b = w;
    return result;
}

color_t Wheel::color(rgb_t rgb) {
    color_t result;
    result.rgb = rgb;
    return result;
}

/**
 * For color cycling patterns.
 *  @param wheelPos the position of the LED in a circle of 240 "degrees"
 *  @param offset the offset of the color pattern in a circle of 240 "degrees"
 *  @param brightness the brightness of the color pattern
 */
color_t Wheel::hue(uint8_t position, uint8_t offset, uint8_t brightness) {

    int huePos = (240 + (int)position - (int)offset) % 240;

    if(huePos < 80) {
        return color(brightness*(80-huePos)/80, 0, brightness*huePos/80);
    } else if(huePos < 160) {
        return color(0, brightness*(huePos-80)/80, brightness*(160-huePos)/80);
    } else {
        return color(brightness*(huePos-160)/80, brightness*(240-huePos)/80, 0);
    }

}

color_t Wheel::calculate(int dial) {

    if(dial < 24) {
		return (((dial+1)/2)%12 == hour) ? color(0, brightness, 0) : color(0);
	} else {
        dial -= 24;
		return (dial == minute) ? color(0, brightness, 0) : color(0);
	}

}

void Wheel::setTime(uint32_t time) {

	rawTime = time;

	time /= 10;
	centi = time % 100;

	time /= 100;
	second = time % 60;

	time /= 60;
	minute = time % 60;

	time /= 60;
	hour = time % 12;

	time /= 12;
	ispm = time % 2;

}

char * WheelBright::status() {
    return "WheelBright";
}

color_t WheelBright::calculate(int dial) {

    color_t result = {0};

    if(brightness > 6) {

        if(dial < 24) {
            if(((dial+1)/2)%12 == hour) {
                result = color(brightness);
            } else {
            	result = hue(dial * 10, 24 * centi / 10, brightness / 3);
            }
    	} else {
            if((dial-24) == minute) {
                result = color(brightness);
            } else if((dial-24) == 0) {
                result = hue((dial-24) * 4, 4 * second, brightness);
            } else {
                result = hue((dial-24) * 4, 4 * second, brightness / 3);
            }
        }
    }

    if(brightness < 12) {

        if(dial < 24) {
            // hours
            if(((dial+1)/2)%12 == hour) {
                result = color(max(brightness, 4));
            } else {
                int innerDial = (24 + dial - 24*centi/100) % 24;
                if(innerDial < (4-brightness/3)) {
                    result.rgb.r = max((4-brightness/3)-innerDial,result.rgb.r); 
                } else if (innerDial > 21 + (4-brightness/3)) {
                    result.rgb.r = max(innerDial - (20+(4-(brightness/3))),result.rgb.r); 
                } else {
                    result.rgb.r = max(1,result.rgb.r); 
                }
            }
        } else {

            // minutes

            if((dial-24) == minute) {
                result = color(max(brightness, 4));
            } else if(0 == (dial-24)) {
                result.rgb.r = max(result.rgb.r, 2);
            } else {
                if((dial-24) == second) {
                    result.rgb.r = max(result.rgb.r, 6);
                } else {
                    result.rgb.r = max(result.rgb.r, 1);
                }
            }
        }

    }

    return result;
}

char * WheelDark::status() {
    return "WheelDark";
}

color_t WheelDark::calculate(int dial) {

    if(dial < 24) {
        // hours
        if(((dial+1)/2)%12 == hour) {
        	return color(4,4,4);;
        } else {
            int innerDial = (24 + dial - 24*centi/100) % 24;
            if(innerDial < 3) {
                return color(4 - innerDial,0,0);
            } else if (innerDial > 21) {
            	return color(innerDial - 20,0,0);
            } else {
            	return color(1,0,0);
            }
        }
    } else {

    	dial -= 24;
        // minutes

        if(dial == minute) {
        	return color(4,4,4);;
        } else if(0 == dial) {
        	return color(2,0,0);
        } else {
            if(dial == second) {
            	return color(6,0,0);
            } else {
            	return color(1,0,0);
            }
        }
    }

}

char * WheelMood::status() {
    return "WheelMood";
}

void WheelMood::setMood(color_t newMood) {
	mood = newMood;
}

/**
 *  Elevate the "bright" color pattern by the mood color
 */
color_t WheelMood::calculate(int dial) {
    
    color_t result = WheelBright::calculate(dial);

    result.rgb.r = (255 - result.rgb.r < mood.rgb.r) ? 255 : result.rgb.r + mood.rgb.r;
    result.rgb.g = (255 - result.rgb.g < mood.rgb.g) ? 255 : result.rgb.g + mood.rgb.g;
    result.rgb.b = (255 - result.rgb.b < mood.rgb.b) ? 255 : result.rgb.b + mood.rgb.b;

    return result;
}

