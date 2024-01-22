// Uses M5 Stack Core and INA2266 Sensor module
// Displays graphically the Current Volt and Amp readings from Eurorack Powersupply
// 

#include "INA226.h"
#include <TFT_eSPI.h>     // Hardware-specific library
#include <TFT_eWidget.h> 
#include "Free_Fonts.h"


// Meter colour schemes
#define RED2RED     0
#define GREEN2GREEN 1
#define BLUE2BLUE   2
#define BLUE2RED    3
#define GREEN2RED   4
#define RED2GREEN   5
#define RAINBOW     6

INA226 INA(0x40);

float maxVoltage = 13.00;
float maxAmps = 8.00;

float voltReading = 0;
float ampReading = 0;
float curVoltage = 0;
float curAmps = 0;
float oldV = 0;
float oldA = 0;

TFT_eSPI tft  = TFT_eSPI();      // Invoke custom library

MeterWidget   amps  = MeterWidget(&tft);
MeterWidget   volts = MeterWidget(&tft);
MeterWidget   ohms  = MeterWidget(&tft);

#define LOOP_PERIOD 35 // Display updates every 35 ms

int calculateAmperageSegments(float currentAmperage, float maxAmperage) {
    if (maxAmperage == 0) {
        // Avoid division by zero
        return 0;
    }

    float percentage = (currentAmperage / maxAmperage) * 100;
    int segmentsToDisplay = (percentage / 100) * 20;

    // Ensure the result is within the bounds of 0 to 20
    if (segmentsToDisplay > 20) {
        segmentsToDisplay = 20;
    } else if (segmentsToDisplay < 0) {
        segmentsToDisplay = 0;
    }

    return segmentsToDisplay;
}

int calculateVoltageSegmentDisplay(float currentVoltage, float maxVoltage) {
    if (maxVoltage == 0) {
        // Avoid division by zero
        return 0;
    }

    float percentage = (currentVoltage / maxVoltage) * 100;
    int segmentsToDisplay = (percentage / 100) * 20;

    // Ensure the result is within the bounds of 0 to 20
    if (segmentsToDisplay > 20) {
        segmentsToDisplay = 20;
    } else if (segmentsToDisplay < 0) {
        segmentsToDisplay = 0;
    }

    return segmentsToDisplay;
}


// #########################################################################
//  Draw the linear meter
// #########################################################################
// val =  reading to show (range is 0 to n)
// x, y = position of top left corner
// w, h = width and height of a single bar
// g    = pixel gap to next bar (can be 0)
// n    = number of segments
// s    = colour scheme
void linearMeter(int val, int x, int y, int w, int h, int g, int n, byte s)
{
  // Variable to save "value" text colour from scheme and set default
  int colour = TFT_BLUE;
  // Draw n colour blocks
  for (int b = 1; b <= n; b++) {
    if (val > 0 && b <= val) { // Fill in coloured blocks
      switch (s) {
        case 0: colour = TFT_RED; break; // Fixed colour
        case 1: colour = TFT_GREEN; break; // Fixed colour
        case 2: colour = TFT_BLUE; break; // Fixed colour
        case 3: colour = rainbowColor(map(b, 0, n, 127,   0)); break; // Blue to red
        case 4: colour = rainbowColor(map(b, 0, n,  63,   0)); break; // Green to red
        case 5: colour = rainbowColor(map(b, 0, n,   0,  63)); break; // Red to green
        case 6: colour = rainbowColor(map(b, 0, n,   0, 159)); break; // Rainbow (red to violet)
      }
      tft.fillRect(x + b*(w+g), y, w, h, colour);
    }
    else // Fill in blank segments
    {
      tft.fillRect(x + b*(w+g), y, w, h, TFT_DARKGREY);
    }
  }
}

/***************************************************************************************
** Function name:           rainbowColor
** Description:             Return a 16 bit rainbow colour
***************************************************************************************/
  // If 'spectrum' is in the range 0-159 it is converted to a spectrum colour
  // from 0 = red through to 127 = blue to 159 = violet
  // Extending the range to 0-191 adds a further violet to red band
 
uint16_t rainbowColor(uint8_t spectrum)
{
  spectrum = spectrum%192;
  
  uint8_t red   = 0; // Red is the top 5 bits of a 16 bit colour spectrum
  uint8_t green = 0; // Green is the middle 6 bits, but only top 5 bits used here
  uint8_t blue  = 0; // Blue is the bottom 5 bits

  uint8_t sector = spectrum >> 5;
  uint8_t amplit = spectrum & 0x1F;

  switch (sector)
  {
    case 0:
      red   = 0x1F;
      green = amplit; // Green ramps up
      blue  = 0;
      break;
    case 1:
      red   = 0x1F - amplit; // Red ramps down
      green = 0x1F;
      blue  = 0;
      break;
    case 2:
      red   = 0;
      green = 0x1F;
      blue  = amplit; // Blue ramps up
      break;
    case 3:
      red   = 0;
      green = 0x1F - amplit; // Green ramps down
      blue  = 0x1F;
      break;
    case 4:
      red   = amplit; // Red ramps up
      green = 0;
      blue  = 0x1F;
      break;
    case 5:
      red   = 0x1F;
      green = 0;
      blue  = 0x1F - amplit; // Blue ramps down
      break;
  }

  return red << 11 | green << 6 | blue;
}
void setup(void) 
{

 // Serial.begin(115200);
 // Serial.println(__FILE__);
 // Serial.print("INA226_LIB_VERSION: ");
 // Serial.println(INA226_LIB_VERSION);
  Wire.begin();
  if (!INA.begin() )
  {
    Serial.println("could not connect. Fix and Reboot");
  }
  INA.setMaxCurrentShunt(1, 0.002);


  tft.init();
  tft.invertDisplay(1);
  tft.setRotation(1);
 // Serial.begin(115200); // For debug
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(FSB12);
 
  tft.setCursor(20, 30);
  tft.print("Voltage:");
  tft.setCursor(20, 128);
  tft.print("Amps:");

  tft.setFreeFont(FSB9);
  tft.setCursor(220, 105);
  tft.print("Max: " + String(maxVoltage));
  tft.setCursor(220, 205);
  tft.print("Max: " + String(maxAmps));

}

void loop() {

oldV = curVoltage;
oldA = curAmps;

curVoltage = INA.getBusVoltage();
curAmps = INA.getCurrent_mA();

voltReading = calculateVoltageSegmentDisplay(curVoltage, maxVoltage);
ampReading = calculateAmperageSegments(curAmps, maxAmps);

//  linearMeter(reading, 10,  10, 5, 25, 3, 20, RED2RED);
//  linearMeter(reading, 10,  40, 5, 25, 3, 20, GREEN2GREEN);
//  linearMeter(reading, 10,  70, 5, 25, 3, 20, BLUE2BLUE);
  linearMeter(voltReading, 0, 40, 11, 50, 4, 20, BLUE2RED);
  linearMeter(ampReading, 0, 140, 11, 50, 4, 20, GREEN2RED);
//  linearMeter(reading, 10, 160, 5, 25, 3, 20, RED2GREEN);
//  linearMeter(reading, 10, 190, 5, 25, 3, 20, RAINBOW);

  // reading++;
  // if (reading > 20) { reading = 0; delay(1000); }
  // tft.fillRect(140, 5, 150, 30, TFT_BLACK);
  // tft.fillRect(140, 110, 150, 25, TFT_BLACK);     


  tft.setFreeFont(FSB12);
  
  tft.setCursor(150, 30);
  tft.setTextColor(TFT_BLACK);
  tft.print(String(oldV) + " V");
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(150, 30);
  tft.print(String(curVoltage) + " V");
  
  tft.setCursor(150, 130);
  tft.setTextColor(TFT_BLACK);
  tft.print(String(oldA) + " A");
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(150, 130);
  tft.print(String(curAmps) + " A");

  
// Debug
//  Serial.println("\nBUS\tSHUNT\tCURRENT\tPOWER");
//   for (int i = 0; i < 20; i++)
//   {
//     Serial.print(INA.getBusVoltage(), 3);
//     Serial.print("\t");
//     Serial.print(INA.getShuntVoltage_mV(), 3);
//     Serial.print("\t");
//     Serial.print(INA.getCurrent_mA(), 3);
//     Serial.print("\t");
//     Serial.print(INA.getPower_mW(), 3);
//     Serial.println();
//   }


  delay (100);
}

