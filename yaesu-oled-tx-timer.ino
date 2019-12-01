#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configure the Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint32_t t; // TX time (secs)
uint8_t m;  // Derived Minutes
uint8_t s;  // Derived seconds (60-second fraction)

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
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(10,6);
  display.print("PY2RAF");  // YOUR CALLSIGN HERE!! \o/
  display.display();
  delay(4000);
  display.invertDisplay(true);
  display.display();
  delay(300);
  display.invertDisplay(false);
  display.display();
  delay(300);
  display.clearDisplay();

  // Configure display runtime defaults
  display.setTextColor(WHITE);
  display.setTextSize(4);

  // Use pin 13 as a output to generate the 5V to drive pin 2 to the default high state
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  // Pin 2 will be used as input, where we will monitor the line level
  pinMode(2, INPUT);
}

void loop() {
  // Get current state from line 2
  LineState = digitalRead(2);
  // If voltage is present - line 13 driving and no draw from TX_GND line
  if (LineState == HIGH) {
    // Do nothing - display off; flash a dot at the bottom of the screen
    display.clearDisplay();
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
    display.setCursor(0,0);

    // Populate the timer. Used the 'if' trick to pad the seconds and minutes with a zero
    // when the actual number count is less than 10
    if (m < 10) display.print(0);
    display.print(m); display.print(":");
    if (s < 10) display.print(0);
    display.print(s);
    display.display();

    // Give it a second to sleep until next poll
    delay(1000);
    // Add another second to the timer
    t++;
    }
 // Save state line state from this poll run.
 LastLineState =  LineState;
}
