# itsagrid 64/128 - a DIY monome compatible grid

A DIY version of [monome](https://monome.org/) 8x8 and 16x8 grid instruments, fully compatible with existing monome patches and software.

itsagrid is an adaptation of the 
[Adafruit Trellis board](https://www.adafruit.com/product/1616) with the a rubber button grid from Livid Instruments (no longer made by Livid) and using a [Teensy 3.2](https://www.pjrc.com/teensy/) microcontroller. The pcb was redesinged for a larger button spacing, SMD leds, mounting for Teensy, and added a driver chip for variable LED brightness.


## itsagrid varibright

itsagrid software started as a fork of the Trellinome portion of the [DIY monome project](https://github.com/szymonkaliski/diy-monome) by szymonkaliski. The hardware has now been extended to use a PWM chip to obtain variable brightness of the LEDs 

As with the Trellis boards, adjacent pcbs are connected via the SDA, SCL, GND, 5V and INT pads on each board  ([learn.adafruit.com/adafruit-trellis-diy-open-source-led-keypad](https://learn.adafruit.com/adafruit-trellis-diy-open-source-led-keypad)). Four pcbs are used for 64 grid, eight for a 128 grid. Each pcb as a set of jumpers that need to be configured for unique addressing of the i2c driver chips (buttons and leds).

Due to driver chip address limitations in the current design, only 8 boards can be used together for a total of 128 buttons.


## mext

The mext protocol is used for serial communication - the same that is used in most recent monome devices.

See [szymonkaliski's github](https://github.com/szymonkaliski/diy-monome) for more technical details of his build figuring out the mext/OSC communication.


## serialosc / libmonome

You can get both `serialosc` and `libmonome` code from [monome github page](https://github.com/monome), and building them is well documented on official linux docs (they work for macOS as well), read part *2 Preparing your system: serialosc* (ignoring the `sudo apt-get` - I was missing `liblo`, but it's available on homebrew): [monome.org/docs/linux/](https://monome.org/docs/linux/).

Note - there is a serialosc patch szymonkaliski included with his build which may be needed on MacOS.


## Case / Panel

Prototype case is laser cut acrylic top and bottom panels and some spacers. Laser files will be posted here.

## References 

https://github.com/szymonkaliski/diy-monome (MIT License)
https://github.com/TheKitty/Untz_Monome (MIT License)

## Libraries

This project uses versions of the following which have been modified for use with the Teensy i2c_t3 library and additional TLC59116 functionality.

Adafruit Trellis Library
https://github.com/adafruit/Adafruit_Trellis_Library

TLC59116 Library by Majenko Technologies
https://github.com/MajenkoLibraries/TLC59116
Copyright (c) 2014, Majenko Technologies
