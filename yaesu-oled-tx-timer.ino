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

// Are we using a temperature sensor? If we are not, comment this line.
// The Temperature Sensor code is compatible with LM-35 sensors.
#define TEMP_SENS      A0 // The Port where the temperature is being read

// Configure the GPIO pins
#define TX_GND         2  // The GPIO pin where you connected the TXGND signal
#define PULLUP        13  // The GPIO pin that is driving TXGND up
#define GPIO_FAN      10  // The GPIO pin where we control the fan relay.
#define RX_ON          4  // The GPIO pin where you connected the SQL signal
                          // Comment out to disable monitoring RX.

// Configure a time threshold (seconds)
// When reaching this threshold, the display will invert so it can catch
// your attention. 
#define TIME_ALERT      360    // 6 minutes alert

// Your callsign. Comment out to disable splash screen.
#define CALLSIGN "PY2RAF"

// Do we want to show the radio temperature in TX mode?
#define TEMP_ON_TX      1

// Temperature Threshold - kicks in the fan when larger than this
#define TEMP_THRESHOLD  33

// Monitor squelched time? If not desired, comment.
#define MONITOR_SQUELCH 1

// Monitor uptime? If not desired, comment.
#define MONITOR_UPTIME  1

// Print SQ/RX state? If not desired, comment.
#define MONITOR_STATE   1

// General variables
uint32_t t;         // Timer (secs)
uint8_t h;          // Derived hours
uint8_t m;          // Derived Minutes
uint8_t s;          // Derived seconds (60-second fraction)
uint32_t u;         // Uptime (secs)
uint8_t hu;         // Derived uptime hours
uint8_t mu;         // Derived uptime Minutes
int temperature;        // Current temp; Celsius
int temp_high_counter;  // Temperature above Threshold counter
int temp_low_counter;   // Temperature under Threshold counter
bool fan_state;         // Self Explanatory

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

  #ifdef GPIO_FAN // If we are controlling a fan...
    pinMode(GPIO_FAN, OUTPUT);
    digitalWrite(GPIO_FAN, LOW);
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

#ifndef MONITOR_SQUELCH
// Blinking dot function
void blinkingdot() {
  display.clearDisplay();
  display.invertDisplay(false);
  if ( millis() / 1000 % 2 == 0 ) display.drawPixel(127, 31, WHITE);
  display.display();
  delay(500);
}
#endif

#ifdef MONITOR_UPTIME
// Print the current uptime
void print_uptime(){
  u = millis()/1000;
  mu = u / 60 % 60;
  hu = u / 3600;

  // Uptime block
  display.setTextSize(2);  // Medium size
  display.setCursor(0, 0); // Bottom top of the screen
  if (hu < 10) display.print(0);
  display.print(hu); display.print("h");
  if (mu < 10) display.print(0);
  display.print(mu);
  display.setTextSize(1);

  // The temperature block
  #ifdef TEMP_SENS
  // The uptime text
  display.setCursor(12,16);
  display.print("Uptime");
  // The temperature info
  display.setCursor(7,25);
  display.print("Temp:");
  display.print(temperature);
  display.print("C");

  // Blink a dot when fan is on
  #ifdef GPIO_FAN
    if (fan_state == 1) {
      if ( millis() / 1000 % 2 == 0 ) {
        display.setCursor(69,15);
        display.print((char)7); } } 
  #endif

  #else
  // If no temperature selected, just print uptime.
  display.setCursor(12,24);
  display.print("Uptime");
  #endif

}
#endif

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

  #ifdef MONITOR_UPTIME
  print_uptime();
  #endif

  #ifdef MONITOR_STATE
  display.setTextSize(2);
  display.setCursor(92,0);
  display.print("SQ");
  #endif

  display.setTextSize(1);  // Smaller size
  display.setCursor(80,25); // Bottom left of the screen

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
  #ifdef TEMP_SENS
  temperature = (float(analogRead(TEMP_SENS))*5/(1023))/0.01;
  #endif

  // Control logic for fan. Two minutes to chill off if temperature gets below threshold,
  // 15 seconds to kick in if temperature gets above threshold.
  #ifdef GPIO_FAN
  if (temperature > TEMP_THRESHOLD) {
    temp_high_counter++;
    temp_low_counter=0; }
  else if (fan_state == 1 && temp_low_counter < 120 ) {
    temp_low_counter++;
    temp_high_counter=0; }
  else {
    temp_low_counter++;
    temp_high_counter=0;
    digitalWrite(GPIO_FAN, LOW);
    fan_state=0;
    }

  if ( temp_high_counter > 15 ) {
    digitalWrite(GPIO_FAN, HIGH);
    fan_state=1; }
  #endif

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
        #ifdef MONITOR_UPTIME
        print_uptime();
        #endif
        #ifdef MONITOR_STATE
        display.setTextSize(2);
        display.setCursor(92,15);
        display.print("RX");
        #endif
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
    display.print(m); display.print(" ");
    if (s < 10) display.print(0);
    display.print(s);

    #ifdef TEMP_SENS
    #ifdef TEMP_ON_TX
    #ifdef GPIO_FAN
    if (fan_state == 1 ) { // Fan on, temperature font/color inverted
      display.fillRect(54,9,18,15,WHITE);
      display.setTextSize(1);
      display.setTextColor(BLACK);
      display.setCursor(58,13);
      display.print(temperature); 
      display.setTextColor(WHITE); }
    else {  // Fan off, just temperature inside a box.
      display.drawRect(54,9,18,15,WHITE);
      display.setTextSize(1);
      display.setCursor(58,13);
      display.print(temperature); }

    #else // No Fan Control
      display.drawRect(54,9,18,15,WHITE);
      display.setTextSize(1);
      display.setCursor(58,13);
      display.print(temperature); }
    #endif

    #else // No Temperature on TX
    display.setTextSize(4);
    display.setCursor(55,2); // Print colon
    display.print(":");
    #endif

    #else // No Temperature sensor
    display.setTextSize(4);
    display.setCursor(55,2); // Print colon
    display.print(":");
    #endif

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
