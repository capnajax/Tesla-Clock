
#include "BlendWheel.h"

char * BlendWheel::status() {
    return "BlendWheel";
}

void BlendWheel::setWheel1(Wheel * newWheel1) {
	wheel1 = newWheel1;
}

void BlendWheel::setWheel2(Wheel * newWheel2) {
	wheel2 = newWheel2;
}

Wheel * BlendWheel::getWheel1() {
    return wheel1;
}

Wheel * BlendWheel::getWheel2() {
    return wheel2;
}

void BlendWheel::setFade(uint32_t startTime, int16_t duration) {
	fadeStartTime = startTime;
	fadeDuration = duration;
    complete = false;
}

bool BlendWheel::fadeComplete() {
	return complete;
}

color_t BlendWheel::calculate(int dial) {

    uint8_t transition = lastTransition;

    int rmin, gmin, bmin;

    color_t color1, color2;

    if(rawTime != lastTransitionTime) {
        transition = 
                (fadeDuration == FADE_NEVER) ?
                    0 :
                    (uint8_t)(min(255, 255 * (rawTime - fadeStartTime) / fadeDuration) & 0xFF);
        lastTransition = transition;
        lastTransitionTime = rawTime;

        wheel1->setTime(rawTime);
        wheel2->setTime(rawTime);
        wheel1->setBrightness(brightness);
        wheel2->setBrightness(brightness);
    }

    // I'm assuming this uses linear colors, not logarithmic.
    // TODO test this by fading 50% white to 50% white.

    switch(transition) {
    case 0:

        //if(dial == 0) Serial.println("S b0x00");
        return wheel1->calculate(dial);
    
    case 255:
    
        //if(dial == 0) Serial.println("S b0xFF");
        complete = true;
        return wheel2->calculate(dial);
    
    default:

        //if(dial == 0) {Serial.print("S b");Serial.println(transition, HEX);}
        color1 = wheel1->calculate(dial);
        color2 = wheel2->calculate(dial);

        // don't fade to zero unless both color 1 and color2 are zero.
        rmin = color1.rgb.r || color2.rgb.r ? 1 : 0,
        gmin = color1.rgb.g || color2.rgb.g ? 1 : 0,
        bmin = color1.rgb.b || color2.rgb.b ? 1 : 0;

        return color(
            max(rmin, ((255 - transition) * color1.rgb.r + transition * color2.rgb.r) / 255),
            max(gmin, ((255 - transition) * color1.rgb.g + transition * color2.rgb.g) / 255),
            max(bmin, ((255 - transition) * color1.rgb.b + transition * color2.rgb.b) / 255)
        );

    }
}
