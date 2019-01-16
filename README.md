# itsagrid 64/128 - a DIY monome compatible grid

A DIY version of [monome](https://monome.org/) 8x8 and 16x8 grid instruments, that are  *mostly* compatible with existing monome patches and software.

itsagrid is an adaptation of the 
[Adafruit Trellis board](https://www.adafruit.com/product/1616) with the a rubber button grid from Livid Instruments (no longer made by Livid) and using a [Teensy 3.2](https://www.pjrc.com/teensy/) microcontroller. The pcb was updated for different buttons spacing, SMD leds, mounting for Teensy, and now variable LED brightness.

This is not a replacement for official monome grid (which has much nicer hardware), but just a DIY project.

## itsagrid varibright

itsagrid software started as a fork of the Trellinome portion of the [DIY monome project](https://github.com/szymonkaliski/diy-monome) by szymonkaliski. It's now been extended to use a PWM chip to obtain variable brightness of the LEDs 

Adjacent pcbs are connected via the SDA, SCL, GND, 5V and INT pads on each board  ([learn.adafruit.com/adafruit-trellis-diy-open-source-led-keypad](https://learn.adafruit.com/adafruit-trellis-diy-open-source-led-keypad)). Four pcbs are used for 64 grid, eight for a 128 grid. Each pcb as a set of jumpers that need to be configured for unique addressing of the i2c driver chips (buttons and leds).


## Arduino & Hardware & Mext

See [szymonkaliski's github](https://github.com/szymonkaliski/diy-monome) for more technical details of his build figuring out the mext/OSC communication.


## serialosc / libmonome

You can get both `serialosc` and `libmonome` code from [monome github page](https://github.com/monome), and building them is well documented on official linux docs (they work for macOS as well), read part *2 Preparing your system: serialosc* (ignoring the `sudo apt-get` - I was missing `liblo`, but it's available on homebrew): [monome.org/docs/linux/](https://monome.org/docs/linux/).


## Case / Panel

Prototype case is laser cut acrylic top and bottom panels and some spacers. Laser files will be posted here.

## References 

https://github.com/szymonkaliski/diy-monome (MIT License)
https://github.com/TheKitty/Untz_Monome (MIT License)

## Libraries

This project uses modified versions of the following

Adafruit Trellis Library
https://github.com/adafruit/Adafruit_Trellis_Library

TLC59116 Library by Majenko Technologies
https://github.com/MajenkoLibraries/TLC59116
Copyright (c) 2014, Majenko Technologies