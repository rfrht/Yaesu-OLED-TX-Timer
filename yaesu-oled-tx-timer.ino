// Libraries to be used
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configure the Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configure the GPIO pins
#define TX_GND 2          // The GPIO pin where you connected the TXGND signal
#define PULLUP 13         // The GPIO pin that is driving TXGND up

// Configure a time threshold (seconds)
// When reaching this threshold, the display will invert so it can catch
// your attention. 
#define TIME_ALERT 360  // 6 minutes alert

// Your callsign. Comment out to disable splash screen.
#define CALLSIGN "PY2RAF"

// General variables
uint32_t t;         // TX time (secs)
uint8_t m;          // Derived Minutes
uint8_t s;          // Derived seconds (60-second fraction)
int LineState;      // Current GPIO line state
int LastLineState;  // Previous poll GPIO line state

void setup() {
  Serial.begin(9600);  // Setup serial port in case of display failure
  
  // Display initialization
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

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
  display.setTextSize(4);

  // Configure the pin that will be providing the pullup level to TX_GND
  pinMode(PULLUP, OUTPUT);
  digitalWrite(PULLUP, HIGH);

  // Configure the TX_GND pin as a input GPIO
  pinMode(TX_GND, INPUT);
}

void loop() {
  // Get current state from the TX_GND pin/GPIO
  LineState = digitalRead(TX_GND);

  // If voltage is present - line 13 driving and no draw from TX_GND line
  if (LineState == HIGH) {
    // Do nothing - display off; flash a dot at the bottom of the screen
    display.clearDisplay();
    display.invertDisplay(false);
    if ( millis() / 1000 % 2 == 0 ) display.drawPixel(127, 31, WHITE);
    display.display();
    delay(500);
    }

  // However if line is grounded... (meaning: TX is ON)
  else {
    // Was line state changed from the last poll? If it was changed (new transition),
    // reset the timer counter. Otherwise, we're already in TX and do not reset the
    // counter.
    if (LineState != LastLineState) t=0 ;

    // Calculating the seconds
    s = t % 60;
    // Calculating the minutes
    m = (t - s)/60;

    // Clear the display buffer and prepare for new data
    display.clearDisplay();
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

    // Add another second to the timer
    t++;
    }

  // Save state line state from this poll run.
  LastLineState =  LineState;
}
