// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

#include "Adafruit_NeoPixel.h"

#include "Wheel.h"
#include "BlendWheel.h"
#include "WeatherWheel.h"
#include "AppWheel.h"
#include "Comms.h"

#define NUM_PIXELS 84
#define NEOPIXEL_PIN 4
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define LEVEL_OFFSET 105
#define LEVEL_TIME 60000

uint32_t startTime = 0; // millis last midnight
int16_t level = 0; // horizontal acceleration (x10)
uint32_t bumpTime = 0; // time the level was bumped (off by more than about 30deg)

// different types of wheels available
WheelBright wheelBright;
WheelMood wheelMood;
WheelAlert wheelAlert;
WheelWarning wheelWarning;
WheelRain wheelRain;
BlendWheel blendWheel;
ArtificialHorizon artificialHorizon;
Photo photo;

Wheel * currentWheel = &wheelBright;

BluetoothComms comms;

// these are calculated from the last few light samples. To prevent it from jittering, this is a sort of
// weighted rolling average -- each sample is added to 90% of this number to make this a 10x average.
uint32_t ambientLight = 9750L;

void setup () {

    Serial.begin(9600);

    Serial.println('B'); // B means BEGIN

    digitalWrite(3, HIGH);
    digitalWrite(2, LOW);

    strip.begin();
    strip.show();

    color_t color = { 0xaabbccddL };

    Serial.write("S Color.c == 0x");Serial.println(color.c, HEX);
    Serial.write("S Color.rgb.r == 0x");Serial.println(color.rgb.r, HEX);
    Serial.write("S Color.rgb.g == 0x");Serial.println(color.rgb.g, HEX);
    Serial.write("S Color.rgb.b == 0x");Serial.println(color.rgb.b, HEX);

}

void fadeIn(Wheel * newWheel, int16_t duration, uint32_t rawTime) {
    if(currentWheel == &blendWheel) {
        blendWheel.setWheel1(blendWheel.getWheel2());
        blendWheel.setWheel2(newWheel);
        blendWheel.setFade(rawTime, duration);
    } else if(newWheel != currentWheel) {
        blendWheel.setWheel1(currentWheel);
        blendWheel.setWheel2(newWheel);
        blendWheel.setFade(rawTime, duration);
        currentWheel = &blendWheel;
    }
}

void loop () {

    uint32_t time = startTime + millis();
    bool sendStatus = false;

    // first check if there is serial data waiting
    command_t command = comms.command();

    /*
    if('N' != command.command) {
        Serial.write("S command == ");
        Serial.write(command.command);
        Serial.write("\n");
    }
    */

    switch(command.command) {
    
    case 'T': // time since midnight
        startTime = (86400000 + command.uint32Param - (millis()%86400000)) % 86400000;
        break;

    case 'M': // adjust mood
        {   color_t mood = command.colorParam;
            wheelMood.setMood(mood);
            fadeIn(&wheelMood, FADE_SLOW_TIME, time);
            break;
        }

    case 'B': // "bright" -- aka normal operation
        fadeIn(&wheelBright, FADE_SLOW_TIME, time);
        break;

    case 'L': // "bright" -- aka normal operation
        fadeIn(&artificialHorizon, FADE_SLOW_TIME, time);
        bumpTime = time;
        break;

    case 'W': // weather - next char can be A(lert), W(arning), or R(ain)
        switch(command.charParam) {
        case 'A':
            fadeIn(&wheelAlert, FADE_QUICK_TIME, time);
            break;
        case 'W':
            // don't fade, go hard
            currentWheel = &wheelWarning;
            break;
        case 'R':
            fadeIn(&wheelRain, FADE_SLOW_TIME, time);
            break;
        default:
            // do nothing, current program continues
            break;
        }
        break;

    case 'P': // image
        {   photo.setPhoto(command.pixels);
            fadeIn(&photo, FADE_SLOW_TIME, time);
        }
        break;

    case 'S': // send status
        sendStatus = true;
        break;

    default:
        break;
    }

    // Read the sensors

    ambientLight = 9 * ambientLight / 10 + analogRead(0);

    level = 9 * level / 10 + Bean.getAccelerationX();

    if(abs(level) > 1000) {
        if(0 == bumpTime) {
            if(&artificialHorizon != currentWheel) {
                fadeIn(&artificialHorizon, FADE_QUICK_TIME, time);
            }
        }
        bumpTime = time;
    }

    if(currentWheel == &artificialHorizon && bumpTime + LEVEL_TIME < time) {
        fadeIn(&wheelBright, FADE_SLOW_TIME, time);
    }

    if(currentWheel != &artificialHorizon && currentWheel != &blendWheel) {
        // if we have exited the leveler mode, we need to reset the bump time to zero or else we won't be
        // able to re-enter it later.
        bumpTime = 0;
    } else {
        // we're still in leveler mode, so report the current level
        artificialHorizon.setLevel(level + LEVEL_OFFSET);
    }


    // send a status message if requested
    if(sendStatus) {
        Serial.write("S - time: ");
        Serial.print(time);
        Serial.write(", ambientLight: ");
        Serial.print(ambientLight);
        Serial.write(", level: ");
        Serial.print(level + LEVEL_OFFSET);
        Serial.print(", wheel: ");
        Serial.println(currentWheel->status());
    }

    // Check if there's been communication with the server recently

    // TODO 

    // now check the fade
    if(currentWheel == &blendWheel) {
        if(blendWheel.fadeComplete()) {
            currentWheel = blendWheel.getWheel2();
        }
    }

    // run the programs and combine their light
    currentWheel->setTime(time);
    currentWheel->setBrightness(brightnessCalc(ambientLight));
    for(int i = 0; i < NUM_PIXELS; i++) {
        strip.setPixelColor(i, currentWheel->calculate(currentWheel->dialPosition(i)).c);
    }
    strip.show();

    delay(5);
}

uint8_t brightnessCalc(uint32_t ambientLight) {

    // brightness at 700 is 16
    // brightness at 480 is 32

    // the min(918,ambientLight) is to prevent the brightness from going to zero or negative, otherwise we'll end
    // up with bright flashing lights in dark rooms

    uint32_t b = (53L << 6), //3392
             mx = (4 << 6) * (min(9750L, ambientLight)) / ( 750L );
    uint8_t brightness = (uint8_t)((b - mx) >> 6);

    return brightness;
}
