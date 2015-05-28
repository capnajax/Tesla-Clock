
#include "AppWheel.h"

char * Photo::status() {
    return "Photo";
}
void Photo::setPhoto(rgb_t * pixels) {
    photo = pixels;
}

color_t Photo::calculate(int dial) {
    color_t result = color(photo[dial]);
}

char * ArtificialHorizon::status() {
    return "ArtificialHorizon";
}

void ArtificialHorizon::setLevel(int16_t newLevel) {
	level = newLevel;
}

color_t ArtificialHorizon::calculate(int dial) {

    int lowerHalfStart, lowerHalfEnd,
    	levelSet = level + LEVELER_OFFSET;

    if(dial < 24) {

        if(((dial+1)/2)%12 == hour) {
            return color(brightness);
        } else {

            lowerHalfStart = max(0, min(11, 6-levelSet/50));
            lowerHalfEnd = max(12, min(23, 17-levelSet/50));

            if(dial == lowerHalfStart || dial == lowerHalfEnd) {
                return color(brightness/3, brightness/3, brightness/3);
            } else if(dial == 6 || dial == 17) {
                return color(brightness/5, brightness/5, 0);
            } else if(dial > lowerHalfEnd || dial < lowerHalfStart) {
                return color(0, 0, brightness/3);
            } else {
                return color(0, brightness/3, 0);
            }
        }

    } else {

        dial -= 24;

        if(dial == minute) {
            return color(brightness);
        } else {

            lowerHalfStart = max(0, min(29, 15-(levelSet/240)));
            lowerHalfEnd = max(30, min(59, 45-(levelSet/240)));

            if(dial == lowerHalfStart || dial == lowerHalfEnd) {
                return color(brightness/3, brightness/3, brightness/3);
            } else if(dial == 15 || dial == 45) {
                return color(brightness/5, brightness/5, 0);
            } else if(dial > lowerHalfEnd || dial < lowerHalfStart) {
                return color(0, 0, brightness/3);
            } else {
                return color(0, brightness/3, 0);
            }

        }
    }

}


