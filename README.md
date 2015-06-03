# Tesla-Clock

https://vimeo.com/129073089

Here's a STEAM* project I've been working on recently: my Tesla Clock.
It's easy to make a clock out of LEDs and an Arduino, but I wanted to go beyond the obvious and se what I could do to make the clock smarter and more beautiful, so I introduced apps:
- Clock, that dims with the lights
- Artificial Horizon, to help get the clock straight on the wall
- Weather Alerts to let you know it's raining or that there's a tornado warning,
- Mood Lighting, really let those LEDs shine!
- Photos, programming the individual LEDs from the server.

*STEAM == Science, Technology, Engineering, Art, and Mathematics

## Sources

The source code is organized into two parts, the Arduino code to run on the LightBlue Bean, and the service that runs on a Node.JS server. This is only tested on a Mac, but should work on any Node.JS-supported platform.

The TO-DO list for the web service is a mile long, mainly to complete the integrations.

## Hardware design

The schematic is include in the `hardware` folder. It can be displayed using [Fritzing](http://fritzing.org/home/). If you're comparing the schematic to the video, you'll notice a few differences because the component pinouts are slightly different from the schematic shown.

Most of the design comes from the [Smartphone-controlled mood light](https://punchthrough.com/bean/examples/mood-light/), with a few minor enhancements from me.

This clock is tested with a 2A power supply, and it runs 1/3 of the LEDs at full intensity (ie full-on red or full-on green, or full-on blue). To run full-on white it should require a 6A power supply.

###"What the hell I was thinking when I did that"

The diode on the input was added for its voltage drop. I found that the 5V power supply I used was actually producing 5.2V and it was causing unusual behaviours in the Neopixels; I needed to reduce the voltage just a little. The difference was too much for a linear regular, they require at least a 0.8V dropout voltage. I couldn't just use resistors to split the voltage because the current was too high and too variable, meaning I couldn't get an accurate voltage drop without drawing too much current and producing too much heat. A diode has a fixed forward voltage so I could still get a stable current while only reducing the voltage a little bit.

Why no real-time clock? Real-time clocks require an I^(2)C and the ambient light sensor required an analog pin. Unfortunately, the the only two analog pins on the LightBlue Bean would be consumed by any I^(2)C device so I had to make a choice. I can use the computer for semi-accurate timekeeping, so the ambient light sensor won. Later I learned that I^(2)C ambient light sensors exist, so in future revisions I will consider that. The downside is that sensor will be on a chip, which means it'll be slightly more complicated to expose.

The additional 470Ω resistor on the data pin, plus the large cap on the power supply, are recommended by Adafruit to protect the Neopixels.

###Potential enhacements

In future prototypes I would do several things:
* I used layers of foam-core to build the structure of the clock, and mounting tape to attach the electronics. I would replace that with a 3D printed or vacuum-formed shell with support structures for the electronics. The Bean is not straight in the clock's shell, and the difference is accounted for in software.
* I would add a button-cell to back up the power on the LightBlue Bean (the Bean has a holder but it's not accessible), allowing it to reset itself if the LEDs draw too much power for the power supply.
* I would use a smaller TO-92 packaged 3.3V regulator to power the LightBlue Bean, and replace the photo-resistor with an I^(2)C chip-based solution so I can add a real-time clock to the same I^(2)C bus.
* I am considering adding a microphone and band-pass filters, then I can make the clock dance with the music. This may require an additional Arduino to operate.

## Summary of the software components

This clock has several apps, each of which is a subclass of `Wheel`. Each app remains in memory at all times, and there 

### Clock

This obviously displays the time. The web service sets the clock by sending a message with the milliseconds passed since midnight. There is nothing on the clock that displays the date, so the date doesn't need to pass.

The clock does not have a Real Time Clock chip on board, so the time needs to be updated frequently.

The inner dial cycles colours every second, and the outer one every minute. Normally things moving in your peripheral vision is annoying, but I found that if the motion is both regular and smooth, it doesn't distract. This discovery drove the design of the whole clock.

When the lights dim the clock dims, and when it's dark, the colours change to red at the lowest intensity. The colour cycling continues as a red light that moves in circles in the inner dial plus a red "second hand" on the outer dial. The colour cycling on this looks like a single pixel moving around, but it's really a brightening followed by a dimming. This was to prevent the appearance of gaps or skips in the fast-moving animation should a pixel be missed between frames of the animation.

This is the third iteration of the dimming-to-red pattern I tried. The first two failed for these reasons:
* First I tried dimming at a threshold. Unfortunately, the ambient light reading is jittery and drifty; if the light level was just at the threshold the clock would flash between the bright and dark patterns.
* Second I tried using two different thresholds, a higher threshold to brighten than to dim. It didn't work well, and it made it complicated to track the brightness when other light patterns were showing on the clock.
* Ultimately using a single pattern that responds to both brightness levels and has a smooth transition in between eliminated the complexity of dual thresholds and the flickering of a single one.

I also reduced the flickering with an averaging algorithm:
```
L(n+1) = 0.9*L(n) + L(sample)
```

### Artificial Horizon

This part was actually much more simple. I used the LightBlue Bean's accelerometer to find level. Unfortunately the accelerometer doesn't give me the actual angle, but instead detects the horizontal acceleration due to tilting into gravity. If I needed an accurate angle, I would have had to do a lot of trigonometry on a device that has no floating-point unit. I didn't want to do that. Fortunately, if I am anywhere close to level, the relationship between horizontal acceleration and tilt angle is almost linear, so I just used that to approximate the level. I can expect anyone that has the clock tilter 40º to know which way it's off.

### Weather alerts

At this time I am only showing color patterns. Integration with weather services is not yet complete.

### Mood lighting

I still have a few issues with this:
* First the pattern needs some tweaking to ensure that the time and color cycling is still visible, no matter what the mood.
* I also want to set the clock using HSV color to make it easier to cap the brightness of the clock as a whole and avoid overloading the power supple.
* The Philips Hue integration is still incomplete.
* The cross-fade between different patterns doesn't work between two moods, so I will either need to implement a fade between moods or use two instances of the mood pattern.

### Photos

This part was added a joke and it's a costly joke at that. A photo consumes 252 bytes of data. It's not possible to fade between photos because I don't have enough memory on the ATMega328 to store both the outgoing and incoming image. In fact, the image has to change as it loads.

However, the potential of this photo app is that it allows the computer to control every last pixel.

I also had to develop a protocol for this. Originally I used the serial data stream to load the photos but I was losing too much data. Performance is not an issue, but reliability and complexity are. So, instead I used the LightBlue Bean's serial characteristics to send the data six pixels at a time.

## Lessons Learned

This project is rather early in my Arduino obsession, and I learned a lot.
* If you are using `free` or `delete` to manage memory you will die. `Free` and `delete` should be called `fragment` and `disintegrate`. Remember the Arduino only has 2K of RAM, it's neither protected nor virtualized, most of it is consumed by globals, and it needs to run the application stack out of it. If you need to manage dynamic data you'll have to do it within the constraints of a dynamic structure.
* Movement is only distracting if it's jerky, haphazard, or irregular. Smooth and regular movement doesn't distract.
* Values have to remain within range (eg `uint8_t` calculations must remain between 0 and 255) at all times in the calculation. The most annoying bugs I had to fix came from letting numbers exceed their range in the the midst of the calculation.
* If you can save a byte of RAM with 100 bytes of code, do it. It's hard to run out of flash space, but easy to run out of RAM.