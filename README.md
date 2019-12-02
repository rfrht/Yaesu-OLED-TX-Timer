# Yaesu OLED TX and RX Timer
*Yaesu radio TX/RX timer in a OLED display controlled by an Arduino*

[Video demo](https://www.youtube.com/watch?v=AhdzXkJfJBs)

## What?

This is a small OLED on-air (TX) timer and RX (AF out), counting for how long you are transmitting or receiving a signal. This is compatible with every recent (in ham radio lingo, recent mean 20 last years) Yaesu HF transceiver.

## How?

It is controlled by the `TUN/LIN` port `TX GND` pin for the TX and `DATA/RTTY` port `SQL` pin for RX.
In older Yaesu radios look for the `BAND DATA`, `LINEAR`, `ACC`, `DATA`, `RTTY/PKT` ports and check for the `TX GND` and `SQL` outputs and respective pinouts.

When the radio enters transmit mode, the `TX GND` signal will bring the Arduino's GPIO line down to GND level, triggering the clock.

If the radio is actually receiving a signal (open squelch), the `SQL` line goes high to +5V and triggers the RX timer.

With no transmit and RX squelched, the screen goes dark and a small dot blinks at the right bottom corner of the display, as a watchdog.

A splashscreen is shown with your callsign (or your favourite text) on power-up.

[![FT-991A and TX Timer](https://rf3.org:8443/q/yaesu-timer/yaesu-ft991a-tx-counter-scaled.jpg)](https://rf3.org:8443/q/yaesu-timer/yaesu-ft991a-tx-counter.jpg)

*The OLED timer - 6 seconds of TX time*

## Why?

As a ragchewer, I miss a TX timer. I'm not talking about a TOT; but just to know how long have I been transmitting. And I'm now amused gauging how long are the 40m QSOs, heh.

Since Yaesu won't be providing me this anytime soon in their firmwares, I decided to give use to a Arduino sleeping in the drawer.

## What do I need?

* Any Arduino board
* Jumper wire
* [OLED display](https://www.aliexpress.com/item/32819880935.html?spm=a2g0s.9042311.0.0.63724c4drSiqbI)

## How do I?

This is dead simple and any Arduino-compatible board should be up to the task - If you are using other Arduino variants, just remember to fix the pin numbers in the source code.

Below is a simplified schematic of the entire enterprise. Do **not** forget the resistor between GPIOs `3` and `13`.

[![Yaesu Arduino OLED timer Connection Diagram](https://rf3.org:8443/q/yaesu-timer/yaesu-arduino-tx-rx-timer-pinout.jpg)](https://rf3.org:8443/q/yaesu-timer/yaesu-arduino-tx-timer-pinout.png)

*Connection diagram - click to enlarge*

## Configurable stuff
There are a few configurable stuff in [source code](/yaesu-oled-tx-timer.ino). Namely:

* The GPIO lines:
  * `TX_GND` - INPUT - The GPIO port receiving the radio's `TX_GND`
  * `PULLUP` - OUTPUT - The GPIO port that will provide the +5V pull-up to `TX_GND`
  * `RX_ON` - INPUT - The GPIO port receiving the radio's `SQL` signal. Can be commented out to disable the RX counter.
* The threshold timer (`TIME_ALERT`), can be commented out to disable
* The callsign/text to be shown at power on (`CALLSIGN`), can be commented out to disable

## Whistles and bells
* It features a splash screen on power-on;
* Also features a RX timer (smaller font size, in hh:mm:ss format). Only counts when squelch is open.
* You can also configure a [alert threshold](https://github.com/rfrht/Yaesu-OLED-TX-Timer/blob/master/yaesu-oled-tx-timer.ino#L19) - when your TX exceeds this transmission time, the display inverts color schema for extra warning.

## Any gotchas?

Just don't forget the pull-up and pull-down resistors!!!

**But you use the 13.8V output of the radio to power the arduino?**

Yup. This line is rated 1.5A and is more than enough to power your Arduino. The Arduino Uno voltage regulator takes up to 15V. Use a choked cable, just in case, for bonus points.
