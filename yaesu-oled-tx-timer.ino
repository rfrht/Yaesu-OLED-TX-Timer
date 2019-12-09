// Libraries to be used
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configure the Display
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels
#define OLED_RESET     4  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configure the GPIO pins
#define TX_GND         2  // The GPIO pin where you connected the TXGND signal
#define PULLUP        13  // The GPIO pin that is driving TXGND up
#define RX_ON          4  // The GPIO pin where you connected the SQL signal
                          // Comment out to disable monitoring RX.

// Configure a time threshold (seconds)
// When reaching this threshold, the display will invert so it can catch
// your attention. 
#define TIME_ALERT 360    // 6 minutes alert

// Your callsign. Comment out to disable splash screen.
#define CALLSIGN "PY2RAF"

// Monitor squelched time? If not desired, comment.
#define MONITOR_SQUELCH 1

// General variables
uint32_t t;         // Timer (secs)
uint8_t h;          // Derived hours
uint8_t m;          // Derived Minutes
uint8_t s;          // Derived seconds (60-second fraction)
String LastState;   // The last active state used for proper timer tracking

void setup() {
  Serial.begin(9600);  // Setup serial port in case of display failure

  // Display initialization
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  #ifdef RX_ON
    pinMode(RX_ON, INPUT);
  #endif

  // Splash Screen
  #ifdef CALLSIGN
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(3);
    display.setCursor(10,6); // a small padding
    display.print(CALLSIGN);
    display.display();
    delay(4000);
    display.invertDisplay(true);
    display.display();
    delay(300);
    display.invertDisplay(false);
    display.display();
    delay(300);
  #endif

  // Configure display runtime defaults
  display.clearDisplay();
  display.setTextColor(WHITE);

  // Configure the pin that will be providing the pullup level to TX_GND
  pinMode(PULLUP, OUTPUT);
  digitalWrite(PULLUP, HIGH);

  // Configure the TX_GND pin as a input GPIO
  pinMode(TX_GND, INPUT);
}

// Blinking dot function
void blinkingdot() {
  display.clearDisplay();
  display.invertDisplay(false);
  if ( millis() / 1000 % 2 == 0 ) display.drawPixel(127, 31, WHITE);
  display.display();
  delay(500);
}

// Print Squelched timer
void printsquelch() {
  // Reset timer if transitioned state
  if (LastState != "Squelch") {
    t=0;
    display.invertDisplay(false);
  }

  // Calculating the seconds, minutes and hours
  s = t % 60;
  m = t / 60 %60;
  h = t / 3600;

  // Clear the display buffer and prepare for new data
  display.clearDisplay();
  display.setTextSize(1);  // Smaller size
  display.setCursor(0,25); // Bottom left of the screen

  // Populate the timer. Used the 'if' trick to pad the seconds and minutes with a zero
  // when the actual number count is less than 10
  if (h < 10) display.print(0);
  display.print(h); display.print(":");
  if (m < 10) display.print(0);
  display.print(m); display.print(":");
  if (s < 10) display.print(0);
  display.print(s);
  display.display();

  // Give it a second to sleep until next poll
  delay(1000);

  // Add another second to the timer and set state
  t++;
  LastState = "Squelch";
}


void loop() {
    // If voltage is present - meaning no Transmission; radio receiving or squelched
    if (digitalRead(TX_GND) == HIGH) {

      // If we are monitoring RX time too
      #ifdef RX_ON

      // Yaesu radio: Squelch Open, RX=HIGH
      if (digitalRead(RX_ON) == HIGH) {
      // Icom and Kenwood radio: Squelch open, RX=LOW
      // Comment the above "if" and uncomment the below "if".
      // (digitalRead(RX_ON) == LOW) {
      
        // Reset timer if transitioned state
        if (LastState != "RX") {
          t=0;
          display.invertDisplay(false);
        }

        // Calculating the seconds, minutes and hours
        s = t % 60;
        m = t / 60 %60;
        h = t / 3600;

        // Clear the display buffer and prepare for new data
        display.clearDisplay();
        display.setTextSize(1);  // Smaller size
        display.setCursor(80,0); // Better screen adjustment

        // Populate the timer. Used the 'if' trick to pad the seconds and minutes with a zero
        // when the actual number count is less than 10
        if (h < 10) display.print(0);
        display.print(h); display.print(":");
        if (m < 10) display.print(0);
        display.print(m); display.print(":");
        if (s < 10) display.print(0);
        display.print(s);
        display.display();

        // Give it a second to sleep until next poll
        delay(1000);

        // Add another second to the timer and set state
        t++;
        LastState = "RX";
      }

    else {
      // Print a blinking dot or squelch counter
      #ifndef MONITOR_SQUELCH
        blinkingdot();
      #else
        printsquelch();
      #endif
    }
  }

  // If we do not monitor RX stage, insert this block
  #else
      blinkingdot();
      LastState = "Squelch";
  }
  #endif

  // However if line is grounded... (meaning: TX is ON)
  else {
    // Reset timer if transitioned state
    if (LastState != "TX") {
      t=0 ;
      display.invertDisplay(false);
    }

    // Calculating the seconds and minutes (no cap for minutes)
    s = t % 60;
    m = t / 60;

    // Clear the display buffer and prepare for new data
    display.clearDisplay();
    display.setTextSize(4);
    display.setCursor(5,2); // Better screen adjustment

    // Populate the timer. Used the 'if' trick to pad the seconds and minutes with a zero
    // when the actual number count is less than 10
    if (m < 10) display.print(0);
    display.print(m); display.print(":");
    if (s < 10) display.print(0);
    display.print(s);

    // Are we over time alert threshold?
    #ifdef TIME_ALERT 
      if (t > TIME_ALERT) display.invertDisplay(true);
    #endif
    display.display();

    // Give it a second to sleep until next poll
    delay(1000);

    // Add another second to the timer and set state
    t++;
    LastState = "TX";
    }
}
