/************************************************************************************************
*  wakeUpLightCode.ino
*  Gabriel Cronin
*  Controls an alarm clock which turn on adafruit NeoPixel LED Strip for gradual lighting increase
*  based in part off of 
*
 * MicroOLED_Clock.ino
 * Analog Clock demo using SFE_MicroOLED Library
 * Jim Lindblom @ SparkFun Electronics
 * Original Creation Date: October 27, 2014
 * 
 * Distributed as-is; no warranty is given.
 *************************************************************************************************/
#include <Wire.h>  // Include Wire if you're using I2C
#include <SPI.h>  // Include SPI if you're using SPI
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library
#include <Adafruit_NeoPixel.h>  //Include Adafruit NexPixel library

//////////////////////////////////
// Adafruit NeoPixel Definition //
//////////////////////////////////
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 7, NEO_GRB + NEO_KHZ800);

//Functions used to set the color of LED Pixels
void setColor(int* led, byte* color){
  for(int i = 0; i < 3; i++){  
    analogWrite(led[i], 255 - color[i]);
  }
}

void setColor(int* led, const byte* color){
  byte tempByte[] = {color[0], color[1], color[2]};
  setColor(led, tempByte);
}

// This defines the starting and ending colors of the fade.
static byte colorArray[2][3]={  //2 rows, 3 columns
 {245,99,144},  //beginning color = Pink
 {255,254,60}  //ending color = Yellow
};

/********************************************
*  Color Fade Variables
*  - colorOffset is the different between beginning and ending RGB values
*  - currentColor is the raw iterated value at the currentStep (linearly interpolated between ending and beginning colors
*  - scaledColor is currentColor times an intensity factor
*  - steps is how many transitions are made before reaching the ending color
*  - currentStep is the step we are in
*  - intensity is the current intensity (quadratically increases from 0 to 100
*  - colorIncrement is how much the color goes up (when multiplied by currentStep
*  - timestamp is recorded each time a step is taken and allows us to know when the next step will be 
*  - waitDelayTime is used to track how long the full fade will last... it is set by one of potentiometers
*  - stepDelay is set later as waitDelayTime/steps
*  - quadraticCoefficientA is used to set intensity as intensity = quadraticCoefficientA * currentStep^2
 ***************************************************************/
 
static float colorOffset[3] = {colorArray[1][0] - colorArray[0][0], colorArray[1][1] - colorArray[0][1], colorArray[1][2] - colorArray[0][2]};
float currentColor[3] = {0, 0, 0};
float scaledColor[3]=  {0, 0, 0};
static float steps = 50;
byte currentStep = 3;
float intensity = 0;
//float intensityIncrement = float(100.00-intensity)/steps;
float colorIncrement = float(1.00)/steps;
unsigned long timestamp = 0;
unsigned long waitUpDelayTime = 0;
int stepDelay = 0;
float quadraticCoefficientA = 0.04;  //note that these depends on #steps and on initial intensity!!!


//////////////////////////
// MicroOLED Definition //
//////////////////////////
#define PIN_RESET 9  // Connect RST to pin 9 (SPI & I2C)
#define PIN_DC    8  // Connect DC to pin 8 (SPI only)
#define PIN_CS    10 // Connect CS to pin 10 (SPI only)

//////////////////////////////////
// MicroOLED Object Declaration //
//////////////////////////////////
MicroOLED oled(PIN_RESET, PIN_DC, PIN_CS);  // SPI Example

// Use these variables to set the initial time
int hours = 6;
int minutes = 29;
int seconds = 0;

// Use these variables to set the alarm time
int alarm_hours = 6;
int alarm_minutes = 30;
int alarm_seconds = 0;
boolean alarmFlag = false;


// How fast do you want the clock to spin? Set this to 1 for fun.
// Set this to 1000 to get _about_ 1 second timing.
const int CLOCK_SPEED = 991;

unsigned long lastDraw = 0;


//////////////////////////
//         SETUP        //
//////////////////////////
void setup()
{
  oled.begin();     // Initialize the OLED
  oled.clear(PAGE); // Clear the display's internal memory
  oled.clear(ALL);  // Clear the library's display buffer
  oled.display();   // Display what's in the buffer (splashscreen)
  
  strip.begin();  //initialize NeoPixels
  strip.show();  // show what's in the NeoPixel buffer (nothing at beginning)
  Serial.begin(9600);  //Serial connection to computer
  
  //set the first color step to be the initial color definition (first row of colorArray)
  for(int i = 0; i<3; i++)
  {
    currentColor[i] = colorArray[0][i];
  }
  
}

//////////////////////////
//         LOOP         //
//////////////////////////
void loop()
{

  readPotentiometers();
  Alarm();
  if (lastDraw + CLOCK_SPEED < millis()) //all this occurs once a "second"
  {
    lastDraw = millis();
    updateTime("up", 1);
    
    // Draw the clock:
    oled.clear(PAGE); 
    drawTime();  
    drawAlarmTime();
    oled.display(); 
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// Simple function to increment/decrement seconds and then increment/decrement minutes
// and hours if necessary.  upOrDown is a string set to "up" or "down", and controls
// whether the clock increases or decreases.  incrementRate is an integer which 
// is set with potentiometers to control how fast the clock changes
/////////////////////////////////////////////////////////////////////////////////////////
void updateTime(String upOrDown, int incrementRate)
{
  if(upOrDown == "up")
  {
    seconds = seconds + incrementRate;  // Increment seconds
    if (seconds >= 60)  // If seconds overflows (>=60)
    {
      seconds = 0;  // Set seconds back to 0
      minutes = minutes + incrementRate;    // Increment minutes
      if (minutes >= 60)  // If minutes overflows (>=60)
      {
        minutes = 0;  // Set minutes back to 0
        hours++;      // Increment hours
        if (hours >= 24)  // If hours overflows (>=12)
        {
          hours = 0;  // Set hours back to 0
        }
      }
    }
  }
  else if(upOrDown == "down")
  {
    seconds = seconds - incrementRate;  // Decrement seconds
    if (seconds <= 0)  // If seconds underflows
    {
      seconds = 59;  // Set seconds back to 59
      minutes = minutes - incrementRate;    // Decrement minutes
      if (minutes <=0)  // If minutes underflows 
      {
        minutes = 59; // Set minutes back to 59
        hours--;      // Decrement hours
        if (hours <= 0)  // If hours underflows 
        {
          hours = 24;  // Set hours back to 24
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// Simple function to increment/decrement seconds and then increment/decrement minutes
// and hours if necessary.  upOrDown is a string set to "up" or "down", and controls
// whether the alarm increases or decreases.  incrementRate is an integer which 
// is set with potentiometers to control how fast the alarm changes
/////////////////////////////////////////////////////////////////////////////////////////
void updateAlarm(String upOrDown, int incrementRate)
{
  if(upOrDown == "up")
  {
    alarm_seconds = alarm_seconds + incrementRate;  // Increment seconds
    if (alarm_seconds >= 60)  // If seconds overflows (>=60)
    {
      alarm_seconds = 0;  // Set seconds back to 0
      alarm_minutes = alarm_minutes + incrementRate;    // Increment minutes
      if (alarm_minutes >= 60)  // If minutes overflows (>=60)
      {
        alarm_minutes = 0;  // Set minutes back to 0
        alarm_hours++;      // Increment hours
        if (alarm_hours >= 24)  // If hours overflows (>=12)
        {
          alarm_hours = 0;  // Set hours back to 0
        }
      }
    }
  }
  else if(upOrDown == "down")
  {
    alarm_seconds = alarm_seconds - incrementRate;  // Decrement seconds
    if (alarm_seconds <= 0)  // If seconds underflows
    {
      alarm_seconds = 59;  // Set seconds back to 59
      alarm_minutes = alarm_minutes - incrementRate;    // Decrement minutes
      if (alarm_minutes <=0)  // If minutes underflows 
      {
        alarm_minutes = 59; // Set minutes back to 59
        alarm_hours--;      // Decrement hours
        if (alarm_hours <= 0)  // If hours underflows 
        {
          alarm_hours = 24;  // Set hours back to 24
        }
      }
    }
  }
}

/////////////////////////////////////////////////////////
// Prints the current time on the top line of the OLED //
/////////////////////////////////////////////////////////
void drawTime()
{
  oled.setFontType(0); // set font type 0, please see declaration in SFE_MicroOLED.cpp
  oled.setCursor(0, 0); 
  oled.print("Time:");
  oled.print(hours);
  oled.print(":");
  if(minutes < 10)
  {
    oled.print("0");
  }
  oled.print(minutes);

}

//////////////////////////////////////////////////////////////
// Prints the alarm set time on the bottom line of the OLED //
//////////////////////////////////////////////////////////////
void drawAlarmTime()
{
  oled.setFontType(0); // set font type 0, please see declaration in SFE_MicroOLED.cpp
  oled.setCursor(0, 12); 
  oled.print("Alarm:");
  oled.setCursor(10, 24);
  oled.print(alarm_hours);
  oled.print(":");
  if(alarm_minutes < 10)
  {
    oled.print("0");
  }
  oled.print(alarm_minutes);
}


////////////////////////////////////////////////////////////////////////////////////////
//  Read three potentiometers attached to pins A0, A1, and A2.  
//  Pot on A0 controls the setting of the time.
//  Pot on A1 controls the setting of the alarm
//  Pot on A2 controls the duration of the alarm (rate at which lighting changes
//
//  When A0 and A1 are read, the result is mapped to numbers between 0 and 6.  Each
//  numbers corresponds to a different direction ("up"/"down") and value of 
//  the variable incrementRate in functions updateTime and updateAlarm.  Values
//  are quasi logarithmic through hacked use of number of seconds and delay to slow down
//  setting for cases 2 and 4.  Case 3 corresponds to the Pot being centered, and does nothing.
//
//  When A2 is read, the result is mapped to a number between 60s and 500s, which 
//  correspond to the duration of time that the lights fade over.  Minimum is 60s other-
//  wise the alarm lighting will cycle twice through!
/////////////////////////////////////////////////////////////////////////////////////////
void readPotentiometers()
{

// READ THE FIRST POTENTIOMETER ON A0
    int sensorValue0 = analogRead(A0);
    
    int offset0 = sensorValue0 - 512;
    int range0 = map(offset0, -512, 512, 7, 0)-1;
    if(range0 != 3)
    {
      int delayTime = 0;
      switch(range0) 
      {
        case 0:
          updateTime("down", 10);
          delayTime = 0;
          break;
         case 1:
          updateTime("down", 1);
          delayTime = 500;
          break;
         case 2:
          updateTime("down", 1);
          delayTime = 1000000;  //perfect!!!
          break;
         case 4:
          updateTime("up", 1);
          delayTime = 1000000;
          break;
         case 5:
          updateTime("up", 1);
          delayTime = 500;
          break;
         case 6:
          updateTime("up", 10);
          delayTime = 0;
          break;
      }
      delayMicroseconds(delayTime);
    }

// READ THE SECOND POTENTIOMETER ON A1
    int sensorValue1 = analogRead(A1);
    
    int offset1 = sensorValue1 - 512;
    int range1 = map(offset1, -512, 512, 7, 0)-1;
    if(range1 != 3)
    {
      int delayTime1 = 0;
      switch(range1) 
      {
        case 0:
          updateAlarm("down", 10);
          delayTime1 = 0;
          break;
         case 1:
          updateAlarm("down", 1);
          delayTime1 = 500;
          break;
         case 2:
          updateAlarm("down", 1);
          delayTime1 = 1000000;  //perfect!!!
          break;
         case 4:
          updateAlarm("up", 1);
          delayTime1 = 1000000;
          break;
         case 5:
          updateAlarm("up", 1);
          delayTime1 = 500;
          break;
         case 6:
          updateAlarm("up", 10);
          delayTime1 = 0;
          break;
      }
      delayMicroseconds(delayTime1);
     
    }

// READ THE THIRD POTENTIOMETER ON A2
    waitUpDelayTime = analogRead(A2);  //seconds
    waitUpDelayTime = map(waitUpDelayTime, 0, 1023, 500000, 60000);
    stepDelay = waitUpDelayTime/steps;
    Serial.println(waitUpDelayTime);
}


////////////////////////////////////////////////////////////////////////////////////////
//  Function checks for an alarm condition, then sets NeoPixel lights at appropriate
//  values.  
//  
//  Alarm condition is that time minutes and hours match alarm minutes and hours.
//  This condition raises alarmFlag variable.  
//
//  If alarmFlag is raised, then we check to see if "stepDelay" time has passed
//  and we haven't completed "steps" steps yet.  If true, then we timestamp, increment
//  the RGB values by colorOffset/steps.  This ensures that over "steps" steps, the 
//  color changes by a total of colorOffset, which is the difference between the initial
//  and final colors.  Next the currentColor values are scaled by the intensity factor,
//  and then the NeoPixel LEDs are set to those RGB values.  Last, the value of intensity
//  is increased according to the quadratic expression 
//          intensity = quadraticCoefficientA * currentStep^2
//  and the step counter is increased by one.
//
//  Finally, if we have done all the steps, then the NeoPixels are turned off, alarm flag
//  is lowered, currentStep and intensity are reset to zero, and currentColor is reset to
//  the initial color.
//
//  Note a minor bug... if steps are all accomplished within one minute, then reset will
//  occur, but alarmFlag will be tripped again and so the light cycle will happen multiple
//  times until one minute has elapsed.
////////////////////////////////////////////////////////////////////////////////////////////
void Alarm()
{
    if(alarm_hours == hours && alarm_minutes == minutes)
    {
      alarmFlag = true;
    }
    if(alarmFlag)
    {
      if(millis() - timestamp  > stepDelay && currentStep < steps)
      {
        timestamp = millis();
        for(int i = 0; i<3; i++)
        {
          currentColor[i] += colorOffset[i]*colorIncrement;
        }
        
        for(int i = 0; i<3; i++)
        {
          scaledColor[i] = currentColor[i]*intensity/100.0;
        }
        
        for(int i = 0; i< strip.numPixels(); i++)
        {
          strip.setPixelColor(i,strip.Color( scaledColor[0], scaledColor[1], scaledColor[2]));
        }      
        strip.show();
        intensity = quadraticCoefficientA*pow(currentStep, 2);
        //intensity += intensityIncrement;
        currentStep++;
      }
      else if(currentStep >= steps)
      {
        for(int i = 0; i< strip.numPixels(); i++)
        {
          strip.setPixelColor(i,strip.Color( 0, 0, 0));
        }      
        strip.show();
        alarmFlag = false;
        currentStep = 0;
        intensity = 0;
        for(int i = 0; i<3; i++)
        {
          currentColor[i] = colorArray[0][i];
        }
      }
    }
}

