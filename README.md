# Yaesu-OLED-TX-Timer
*Yaesu radio TX timer in a OLED display controlled by an Arduino*

[Video demo](https://www.youtube.com/watch?v=AhdzXkJfJBs)

## What?

This is a small OLED on-air (TX) timer, counting for how long is the radio active in transmit mode. It is controlled by the `TUN/LIN` `TX GND` line.

When the radio enter in transmit mode, the `TX GND` signal will bring down a GPIO line in the Arduino, triggering the clock.

When the radio is outside transmit mode, a small dot blinks at the right bottom corner of the display.

A splashscreen is shown with your callsign (or your favourite text) on power-up.

[![FT-991A and TX Timer](https://rf3.org:8443/q/yaesu-timer/yaesu-ft991a-tx-counter-scaled.jpg)](https://rf3.org:8443/q/yaesu-timer/yaesu-ft991a-tx-counter.jpg)

*The OLED timer - 6 seconds of TX time*

## Why?

As a ragchewer, I miss a TX timer. I'm not talking about a TOT; but just to know how long have I been transmitting.

Since Yaesu won't be providing me this anytime soon in their firmwares, I decided to give use to a Arduino sleeping in the drawer.

## What do I need?

* Any Arduino board
* Jumper wire
* [OLED display](https://www.aliexpress.com/item/32819880935.html?spm=a2g0s.9042311.0.0.63724c4drSiqbI)

## How do I?

This is dead simple and any Arduino-compatible board should be up to the task - If you are using other Arduino variants, just remember to fix the pin numbers in the source code.

Below is a simplified schematic of the entire enterprise. Do **not** forget the resistor between GPIOs `3` and `13`.

[![Yaesu Arduino OLED timer Connection Diagram](https://rf3.org:8443/q/yaesu-timer/yaesu-arduino-tx-timer-pinout.jpg)](https://rf3.org:8443/q/yaesu-timer/yaesu-arduino-tx-timer-pinout.png)

*Connection diagram - click to enlarge*

## Any gotchas?

Not really - So far super straight forward and no side effects.

**But you use the 13.8V output of the radio to power the arduino?**

Yup. This line is rated 1.5A and is more than enough to power your Arduino. Use a choked cable, just in case, for bonus points.
