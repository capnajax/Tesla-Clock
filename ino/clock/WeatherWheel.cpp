
#include "WeatherWheel.h"

char * WheelRain::status() {
	return "WheelRain";
}

color_t WheelRain::calculate(int dial) {

    if(dial < 24) {
        if(((dial+1)/2)%12 == hour) {
            return color(brightness);
        } else {
        	// colour cycle in but only with the blue
		    int huePos = (240 + (int)(dial * 10) - (int)(24 * centi / 10)) % 240;
		    if(huePos < 80) {
		        return color(0, 0, max(1,max(8,brightness)*huePos/240));
		    } else if(huePos < 160) {
		        return color(0, 0, max(1,max(8,brightness)*(160-huePos)/240));
		    } else {
		        return color(0,0,1);
		    }
        }
	} else {
		dial -= 24;
        if(dial == minute) {
            return color(max(1,brightness));
        } else if(dial == 0) {
        	// top is always blue
            return color(0,0,max((dial==second)?2:1, brightness/3));
        } else if(dial == 30) {
        	// bottom is always off
        	return color(0,0,(dial==second)?2:1);
        } else {
        	// "rain down" the sides, right slightly earlier than left
        	int cycletime = centi/5; // remember, integer division
        	if(dial < 30) {
        		return color(0,0, max((dial==second)?2:1, (1+((100+dial-cycletime)%4)) * brightness/15));
        	} else {
        		return color(0,0, max((dial==second)?2:1, (1+((dial+cycletime)%4)) * brightness/15));
        	}
        }
    }
}

char * WheelAlert::status() {
	return "WheelAlert";
}
color_t WheelAlert::calculate(int dial) {
    if(dial < 24) {
        if(((dial+1)/2)%12 == hour) {
            return color(brightness);
        } else {
            // colour cycle in but only with the red -- widen the red to 100 "degrees" instead of 80, and 
            // index its brightness to the 20 "degree" offset mark so it has a wider peak area as well.
            int huePos = (240 + (int)(dial * 10) - (int)(24 * centi / 10)) % 240;
            if(huePos <= 20 || huePos > 220) {
                return color(max(6, 2*brightness/3), 0, 0);
            } else if (huePos <= 100) {
                return color(max(brightness/3, brightness*(100-huePos)/120), 0, 0);
            } else if (huePos > 120) {
                return color(max(brightness/3, brightness*(huePos-120)/120), 0, 0);
            } else {
                return color(max(2,brightness/3), 0, 0);
            }
        }
    } else {

        dial -= 24;
        color_t result = color(0);

        if(brightness > 6) {

            if(dial == minute) {
                result = color(brightness);
            } else if(dial == 0) {
                result = hue(dial * 4, 4 * second, brightness);
            } else {
                result = hue(dial * 4, 4 * second, brightness / 3);
            }
        }
    
        if(brightness < 12) {

            if(dial == minute) {
                result = color(max(brightness, 4));
            } else if(0 == dial) {
                result.rgb.r = max(result.rgb.r, 2);
            } else {
                if(dial == second) {
                    result.rgb.r = max(result.rgb.r, 6);
                } else {
                    result.rgb.r = max(result.rgb.r, 1);
                }
            }
        }

        return result;
    }
}

char * WheelWarning::status() {
	return "WheelWarning";
}
color_t WheelWarning::calculate(int dial) {
    
    if(( dial < 24 && ((dial+1)/2)%12 == hour) || 
         dial >= 24 && dial-24 == minute) {

        return color(brightness);

    } else {

        // colour cycle both dials only in red, but the outer dial should make a full cycle every
        // second and in reverse.

        int huePos = dial < 24 ?
                (240 + (int)( dial     * 10) - (int)(24 * centi / 10)) % 240 :
                (480 - (int)((dial-24) * 4 ) - (int)(24 * centi / 10)) % 240;

        if(huePos <= 20 || huePos > 220) {
            return color(max(6, 2*brightness/3), 0, 0);
        } else if (huePos <= 100) {
            return color(max(brightness/3, brightness*(100-huePos)/120), 0, 0);
        } else if (huePos > 120) {
            return color(max(brightness/3, brightness*(huePos-120)/120), 0, 0);
        } else {
            return color(max(2,brightness/3), 0, 0);
        }
    }
}
