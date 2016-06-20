#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include "Adafruit_VCNL4010.h"

// Pick analog outputs, for the UNO these three work well
// use ~560 ohm resistor between Green & Blue (I use 680), ~1K for red (its brighter)
//#define redpin 3
//#define greenpin 5
//#define bluepin 6
// for a common anode LED, connect the common pin to +5V
// for common cathode, connect the common to ground
// set to false if using a common cathode LED
#define commonAnode true

//#define buttonPin 11
#define sensorLEDPin 12
#define ledPin 13     // the board LED pin to verify button is working

#define NEO_PIN 9
#define NEO_PIN2 3
#define NUM_LEDS 24
#define NUM_LEDS2 16
#define BRIGHTNESS 10

// our RGB -> eye-recognized gamma color
byte gammatable[256];
uint16_t clear, red, green, blue;
// variables will change:
//int buttonState = 0;         // variable for reading the pushbutton status
int proximityValue = 0;
int temp = 0;
int count = 0;

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NEO_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUM_LEDS2, NEO_PIN2, NEO_GRB + NEO_KHZ800);

Adafruit_VCNL4010 vcnl;

enum Color { PINK, RED, ORANGE, YELLOW, GREEN,  BLUE, PURPLE, RAINBOW, RAINBOWFADE, RAINBOWCYCLE, ALLCOLORS };
enum MODE { TOUCHPOINT, FASTPASS };

Color clr = GREEN;
MODE mode = TOUCHPOINT;

void setup() {
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  strip2.setBrightness(20);
  strip2.begin();
  strip2.show(); // Initialize all pixels to 'off'

  Serial.begin(9600);
  
  Serial.println("TCS34725 RGB Color Sensor Test. . .");
  if (tcs.begin()) {
    Serial.println("TCS34725 RGB Sensor Found!");
  } else {
    Serial.println("TCS34725 NOT found ... check your connections :(");
    while (1); // halt!
  }

  Serial.println("VCNL4010 Proximity Sensor Test. . .");
  if (vcnl.begin()){
    Serial.println("VCNL4010 Proximity Sensor Found!");
  } else {
    Serial.println("VCNL4010 NOT found ... check your connections :(");
    while (1); //halt!
  }
  
  // use these three pins to drive an LED
  //pinMode(redpin, OUTPUT);
  //pinMode(greenpin, OUTPUT);
  //pinMode(bluepin, OUTPUT);
  //pinMode(buttonPin, INPUT); // initialize the pushbutton pin as an input:
  pinMode(ledPin, OUTPUT); // initialize the LED pin as an output:
  pinMode(sensorLEDPin, OUTPUT); // initialize the bright white LED for the RGB Color Sensor
  
  // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
      
    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;      
    }
    //Serial.println(gammatable[i]);
  }
}

void loop() {
  // read the state of the pushbutton value:
  //buttonState = digitalRead(buttonPin);
  proximityValue = vcnl.readProximity();

  //Get range of Proximity sensor
  //Serial.print("Ambient: "); Serial.println(vcnl.readAmbient());
  Serial.print("Proximity: "); Serial.println(proximityValue);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is LOW:
  if (proximityValue >= 5000) {
  //if (buttonState == LOW) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
    digitalWrite(sensorLEDPin, HIGH);
    readRGBColor();
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
    digitalWrite(sensorLEDPin, LOW);
    if(mode == 0) {
      //whiteCycle();
      doubleWhiteRing();
    }
    else {
      delay(500); 
    }
  }
}

void readRGBColor() {
  
    tcs.setInterrupt(false);      // turn on LED
  
    delay(60);  // takes 50ms to read 
    
    tcs.getRawData(&red, &green, &blue, &clear);

    digitalWrite(sensorLEDPin, LOW);
    tcs.setInterrupt(true);  // turn off LED
    
    Serial.print("C:\t"); Serial.print(clear);
    Serial.print("\tR:\t"); Serial.print(red);
    Serial.print("\tG:\t"); Serial.print(green);
    Serial.print("\tB:\t"); Serial.print(blue);
  
    // Figure out some basic hex code for visualization
    uint32_t sum = clear;
    float r, g, b;
    r = red; r /= sum;
    g = green; g /= sum;
    b = blue; b /= sum;
    r *= 256; g *= 256; b *= 256;
    Serial.print("\t");
    Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
    Serial.print("\t");
    Serial.print((int)r); Serial.print((int)g); Serial.print((int)b);
    Serial.println();
    //Serial.print((int)r ); Serial.print(" "); Serial.print((int)g);Serial.print(" ");  Serial.println((int)b );

    if(mode == 0) {
      //authenticateTouchPoint();
      rainbowRing();
      //analogWrite(redpin, gammatable[(int)r]);
      //analogWrite(greenpin, gammatable[(int)g]);
      //analogWrite(bluepin, gammatable[(int)b]);
      Serial.println("TOUCHPOINT authenticated!");
      //colorPulse((int)r, (int)g, (int)b, 35);
      transitionToWhiteCycle(0, 0, 0, 20);
    }
    else if(mode == 1){
      authenticateFastPass();
      //analogWrite(redpin, gammatable[(int)r]);
      //analogWrite(greenpin, gammatable[(int)g]);
      //analogWrite(bluepin, gammatable[(int)b]);
      Serial.println("FASTPASS authenticated!");
      colorPulse((int)r, (int)g, (int)b, 35);
    }  
}

void whiteCycle() {
  
  for(int i=NUM_LEDS-1;i>=0;i--){
      strip.setPixelColor((i+8)%NUM_LEDS, strip.Color(63, 63, 63)); //change RGB color value here
      strip.setPixelColor((i+7)%NUM_LEDS, strip.Color(63, 63, 63)); //change RGB color value here
      strip.setPixelColor((i+6)%NUM_LEDS, strip.Color(95, 95, 95)); //change RGB color value here
      strip.setPixelColor((i+5)%NUM_LEDS, strip.Color(255, 255, 255)); //change RGB color value here
      strip.setPixelColor((i+4)%NUM_LEDS, strip.Color(255, 255, 255)); //change RGB color value here
      strip.setPixelColor((i+3)%NUM_LEDS, strip.Color(255, 255, 255)); //change RGB color value here
      strip.setPixelColor((i+2)%NUM_LEDS, strip.Color(95, 95, 95)); //change RGB color value here
      strip.setPixelColor((i+1)%NUM_LEDS, strip.Color(63, 63, 63)); //change RGB color value here
      strip.setPixelColor(i%NUM_LEDS, strip.Color(63, 63, 63)); //change RGB color value here
      strip.show();
      
      if(NUM_LEDS == 16) { delay(185); } //correct timing but speed makes it look spotty instead of a smooth moving ring
      if(NUM_LEDS  == 24) { delay(115); } //delay(85); } //maybe try for 24 leds. 115 is exact timing, 85 looks nicer.
      clearRing();
   }
}

void clearRing(){
  for(int i=0;i<NUM_LEDS;i++){
      strip.setPixelColor(i, strip.Color(0, 0, 0)); //change RGB color value here
   }
   for(int i=0;i<NUM_LEDS2;i++){
      strip2.setPixelColor(i, strip2.Color(0, 0, 0)); //change RGB color value here
   }
}

void clearColor(int r, int g, int b, uint8_t wait) {
  for(int i=NUM_LEDS-1;i>=0;i--){
      strip.setPixelColor(i, strip.Color(r, g, b)); //change RGB color value here
      strip.show();
      delay(wait);
   }
}

void authenticateTouchPoint() {
  Serial.println("Authenticating TOUCHPOINT . . .");
  if(NUM_LEDS == 16) { whiteSpeed(35, 4); whiteSpeed(28, 4); }
  if(NUM_LEDS  == 24) { whiteSpeed(15, 4); whiteSpeed(10, 4); }
  colorWipe(strip.Color(127, 127, 127), 0);
  colorWipe(strip.Color(255, 255, 255), 0);
  delay(1000);
  clearColor(0,0,0,35);
  delay(250);
  strip.show();
}

void authenticateFastPass() {
  Serial.println("Authenticating FASTPASS . . .");
  if(NUM_LEDS == 16) { whiteSpeed(28, 1);}
  if(NUM_LEDS  == 24) { whiteSpeed(15, 1);}
}

void whiteSpeed(int wait, int cycles){
  for(int i=(NUM_LEDS*cycles)-1;i>=0;i--){
      strip.setPixelColor((i+8)%NUM_LEDS, strip.Color(63, 63, 63)); //change RGB color value here
      strip.setPixelColor((i+7)%NUM_LEDS, strip.Color(63, 63, 63)); //change RGB color value here
      strip.setPixelColor((i+6)%NUM_LEDS, strip.Color(95, 95, 95)); //change RGB color value here
      strip.setPixelColor((i+5)%NUM_LEDS, strip.Color(255, 255, 255)); //change RGB color value here
      strip.setPixelColor((i+4)%NUM_LEDS, strip.Color(255, 255, 255)); //change RGB color value here
      strip.setPixelColor((i+3)%NUM_LEDS, strip.Color(255, 255, 255)); //change RGB color value here
      strip.setPixelColor((i+2)%NUM_LEDS, strip.Color(95, 95, 95)); //change RGB color value here
      strip.setPixelColor((i+1)%NUM_LEDS, strip.Color(63, 63, 63)); //change RGB color value here
      strip.setPixelColor(i%NUM_LEDS, strip.Color(63, 63, 63)); //change RGB color value here
      strip.show();
      delay(wait);
      clearRing();
   }
}

void transitionToWhiteCycle(int r, int g, int b, uint8_t wait) {
  for(int i=NUM_LEDS-1;i>8;i--){
      strip.setPixelColor(i, strip.Color(r, g, b)); //change RGB color value here
      strip.show();
      delay(wait);
   }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  //for(uint16_t i=0; i<strip.numPixels(); i++) {
  for(int i=NUM_LEDS-1;i>=0;i--){
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void colorPulse(int r, int g, int b, uint8_t wait) {
  clearRing();
  strip.show();
  delay(wait);
  for(int i=NUM_LEDS-1;i>=0;i--) {
    strip.setPixelColor(i, strip.Color(r/4, g/4, b/4));
    strip.show();
  }
  delay(wait);
  for(int i=NUM_LEDS-1;i>=0;i--) {
    strip.setPixelColor(i, strip.Color(r/3, g/3, b/3));
    strip.show();
  }
  delay(wait);
  for(int i=NUM_LEDS-1;i>=0;i--) {
    strip.setPixelColor(i, strip.Color(r/2, g/2, b/2));
    strip.show();
  }
  delay(wait);
  for(int i=NUM_LEDS-1;i>=0;i--) {
    strip.setPixelColor(i, strip.Color(r, g, b));
    strip.show();
  }
  delay(50*14);
  for(int i=NUM_LEDS-1;i>=0;i--) {
    strip.setPixelColor(i, strip.Color(r/2, g/2, b/2));
    strip.show();
  }
  delay(wait);
  for(int i=NUM_LEDS-1;i>=0;i--) {
    strip.setPixelColor(i, strip.Color(r/3, g/3, b/3));
    strip.show();
  }
  delay(wait);
  for(int i=NUM_LEDS-1;i>=0;i--) {
    strip.setPixelColor(i, strip.Color(r/4, g/4, b/4));
    strip.show();
  }
  delay(wait);
  if(mode == 1){
    for(int i=NUM_LEDS-1;i>=0;i--) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
      strip.show();
    }
  }
}

void test() {
  for(int i=0;i<NUM_LEDS;i++){
      strip.setPixelColor(i%NUM_LEDS, strip.Color(255, 0, 0)); //change RGB color value here
      strip.show();
      delay(500);
      clearRing();
   }
}

/*----------Fun Stuff----------*/

void rainbowRing() {
   int i = 0;
   while(i<NUM_LEDS*4) {
      strip.setPixelColor(i%NUM_LEDS, strip.Color(255, 255, 255)); //change RGB color value here
      strip.setPixelColor((i+1)%NUM_LEDS, strip.Color(255, 5, 180)); //change RGB color value here
      strip.setPixelColor((i+2)%NUM_LEDS, strip.Color(255, 0, 0)); //change RGB color value here
      strip.setPixelColor((i+3)%NUM_LEDS, strip.Color(255, 150, 0)); //change RGB color value here
      strip.setPixelColor((i+4)%NUM_LEDS, strip.Color(255, 255, 5)); //change RGB color value here
      strip.setPixelColor((i+5)%NUM_LEDS, strip.Color(0, 255, 0)); //change RGB color value here
      strip.setPixelColor((i+6)%NUM_LEDS, strip.Color(0, 0, 255)); //change RGB color value here
      strip.setPixelColor((i+7)%NUM_LEDS, strip.Color(135, 10, 215)); //change RGB color value here
      strip.setPixelColor((i+8)%NUM_LEDS, strip.Color(255, 255, 255)); //change RGB color value here
      strip.setPixelColor((i+9)%NUM_LEDS, strip.Color(255, 5, 180)); //change RGB color value here
      strip.setPixelColor((i+10)%NUM_LEDS, strip.Color(255, 0, 0)); //change RGB color value here
      strip.setPixelColor((i+11)%NUM_LEDS, strip.Color(255, 150, 0)); //change RGB color value here
      strip.setPixelColor((i+12)%NUM_LEDS, strip.Color(255, 255, 5)); //change RGB color value here
      strip.setPixelColor((i+13)%NUM_LEDS, strip.Color(0, 255, 0)); //change RGB color value here
      strip.setPixelColor((i+14)%NUM_LEDS, strip.Color(0, 0, 255)); //change RGB color value here
      strip.setPixelColor((i+15)%NUM_LEDS, strip.Color(135, 10, 215)); //change RGB color value here
      strip.setPixelColor((i+16)%NUM_LEDS, strip.Color(255, 255, 255)); //change RGB color value here
      strip.setPixelColor((i+17)%NUM_LEDS, strip.Color(255, 5, 180)); //change RGB color value here
      strip.setPixelColor((i+18)%NUM_LEDS, strip.Color(255, 0, 0)); //change RGB color value here
      strip.setPixelColor((i+19)%NUM_LEDS, strip.Color(255, 150, 0)); //change RGB color value here
      strip.setPixelColor((i+20)%NUM_LEDS, strip.Color(255, 255, 5)); //change RGB color value here
      strip.setPixelColor((i+21)%NUM_LEDS, strip.Color(0, 255, 0)); //change RGB color value here
      strip.setPixelColor((i+22)%NUM_LEDS, strip.Color(0, 0, 255)); //change RGB color value here
      strip.setPixelColor((i+23)%NUM_LEDS, strip.Color(135, 10, 215)); //change RGB color value here

      if(count>15){
        count = 0;
      }
      temp = i;
      i = (NUM_LEDS2-1) - count;

      strip2.setPixelColor(i%NUM_LEDS2, strip2.Color(255, 255, 255)); //change RGB color value here
      strip2.setPixelColor((i+1)%NUM_LEDS2, strip2.Color(255, 5, 180)); //change RGB color value here
      strip2.setPixelColor((i+2)%NUM_LEDS2, strip2.Color(255, 0, 0)); //change RGB color value here
      strip2.setPixelColor((i+3)%NUM_LEDS2, strip2.Color(255, 150, 0)); //change RGB color value here
      strip2.setPixelColor((i+4)%NUM_LEDS2, strip2.Color(255, 255, 5)); //change RGB color value here
      strip2.setPixelColor((i+5)%NUM_LEDS2, strip2.Color(0, 255, 0)); //change RGB color value here
      strip2.setPixelColor((i+6)%NUM_LEDS2, strip2.Color(0, 0, 255)); //change RGB color value here
      strip2.setPixelColor((i+7)%NUM_LEDS2, strip2.Color(135, 10, 215)); //change RGB color value here
      strip2.setPixelColor((i+8)%NUM_LEDS2, strip2.Color(255, 255, 255)); //change RGB color value here
      strip2.setPixelColor((i+9)%NUM_LEDS2, strip2.Color(255, 5, 180)); //change RGB color value here
      strip2.setPixelColor((i+10)%NUM_LEDS2, strip2.Color(255, 0, 0)); //change RGB color value here
      strip2.setPixelColor((i+11)%NUM_LEDS2, strip2.Color(255, 150, 0)); //change RGB color value here
      strip2.setPixelColor((i+12)%NUM_LEDS2, strip2.Color(255, 255, 5)); //change RGB color value here
      strip2.setPixelColor((i+13)%NUM_LEDS2, strip2.Color(0, 255, 0)); //change RGB color value here
      strip2.setPixelColor((i+14)%NUM_LEDS2, strip2.Color(0, 0, 255)); //change RGB color value here
      strip2.setPixelColor((i+15)%NUM_LEDS2, strip2.Color(135, 10, 215)); //change RGB color value here
      strip2.show();
      
      count++;
      i = temp; 
      i++;
      strip.show();
      delay(75);
   }
}

void rainbowFade(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void allColors(int i){   
    if(i<4){
      colorWipe(strip.Color(255, 5, 180), 25);  //pink
      colorWipe(strip.Color(255, 0, 0), 25);    //red
      colorWipe(strip.Color(255, 150, 0), 25);  //orange
      colorWipe(strip.Color(255, 255, 5), 25);  //yellow
      colorWipe(strip.Color(0, 255, 0), 25);    //green
      colorWipe(strip.Color(0, 0, 255), 25);    //blue
      colorWipe(strip.Color(135, 10, 215), 25); //purple
      i++;
      allColors(i);
    }  
}

void doubleWhiteRing(){
  for(int i=NUM_LEDS-1;i>=0;i--){

      if(count>15){
        count = 0;
      }
      
      strip.setPixelColor((i+8)%NUM_LEDS, strip.Color(63, 63, 63)); //change RGB color value here
      strip.setPixelColor((i+7)%NUM_LEDS, strip.Color(63, 63, 63)); //change RGB color value here
      strip.setPixelColor((i+6)%NUM_LEDS, strip.Color(95, 95, 95)); //change RGB color value here
      strip.setPixelColor((i+5)%NUM_LEDS, strip.Color(255, 255, 255)); //change RGB color value here
      strip.setPixelColor((i+4)%NUM_LEDS, strip.Color(255, 255, 255)); //change RGB color value here
      strip.setPixelColor((i+3)%NUM_LEDS, strip.Color(255, 255, 255)); //change RGB color value here
      strip.setPixelColor((i+2)%NUM_LEDS, strip.Color(95, 95, 95)); //change RGB color value here
      strip.setPixelColor((i+1)%NUM_LEDS, strip.Color(63, 63, 63)); //change RGB color value here
      strip.setPixelColor(i%NUM_LEDS, strip.Color(63, 63, 63)); //change RGB color value here
      strip.show();

      temp = i;
      i = (NUM_LEDS2-1) - count;

      strip2.setPixelColor((i+5)%NUM_LEDS2, strip2.Color(63, 63, 63)); //change RGB color value here
      strip2.setPixelColor((i+4)%NUM_LEDS2, strip2.Color(95, 95, 95)); //change RGB color value here
      strip2.setPixelColor((i+3)%NUM_LEDS2, strip2.Color(255, 255, 255)); //change RGB color value here
      strip2.setPixelColor((i+2)%NUM_LEDS2, strip2.Color(255, 255, 255)); //change RGB color value here
      strip2.setPixelColor((i+1)%NUM_LEDS2, strip2.Color(95, 95, 95)); //change RGB color value here
      strip2.setPixelColor(i%NUM_LEDS2, strip2.Color(63, 63, 63)); //change RGB color value here
      strip2.show();
      
      
      if(NUM_LEDS  == 24) { delay(115); } //delay(85); } //maybe try for 24 leds. 115 is exact timing, 85 looks nicer.
      clearRing();
      count++;
      i = temp;
   }
}

