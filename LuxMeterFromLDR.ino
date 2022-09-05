/* 
 *  LDR Lux Meter
 *  
 *  This sketch calculates the lux from a voltage reading
 *  
 *  The hardware connected to Analog Pin 0 should be a voltage divider circuit between an LDR and a 
 *  resistor (nominally 5kohm).  The resistor should be connected to ground and the LDR should be
 *  connected to 5V.  The point in between should be connected to Analog Pin 0.
 *  1) Calculate voltage based on reading
 *  2) Calculate resistance of LDR based on voltage
 *  3) Calculate the lux that must be falling on LDR based on the resistance
 *  
 *  The lux reading is then output to an Adafruit LCD shield.
 *  Created Dec. 4, 2015
 *  By David Williams
*/


#include <Wire.h>
//  I2C and LCD Shield libraries
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>


   


// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

// These constants, define values needed for the LDR readings and ADC
#define LDR_PIN                   0
#define MAX_ADC_READING           1023
#define ADC_REF_VOLTAGE           5.0
#define REF_RESISTANCE            5030  // measure this for best results
#define LUX_CALC_SCALAR           12518931
#define LUX_CALC_EXPONENT         -1.405





/**************************************************************************/
/*
    Configure the LCD to be 16 char by 2 rows.
    Background is set to white.
    LCD says hello and please wait.
*/
/**************************************************************************/
void configureLCD(void)
{
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.setBacklight(WHITE); 

  // Print a message to the LCD. We track how long it takes since
  // this library has been optimized a bit and we're proud of it :)
  //int time = millis();
  lcd.print(F("Initializing!"));
  delay(555);
  
  lcd.setCursor(0,1);
  lcd.print(F("Please wait..."));
  
  //time = millis() - time;
  //Serial.print("Took "); Serial.print(time); Serial.println(" ms");

  delay(1000);
  lcd.clear();
}


/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void) 
{
  Serial.begin(9600);
  Serial.println(F("Light Sensor Test")); Serial.println("");
  
  /* Setup the LCD to be ready to display */
  configureLCD();
  
  /* We're ready to go! */
  Serial.println(F("Sensor, LCD and SD Card are Configured."));
}


/**************************************************************************/

    

/**************************************************************************/
void loop(void) 
{  
  
  int   ldrRawData;
  float resistorVoltage, ldrVoltage;
  float ldrResistance;
  float ldrLux;
  
  // Perform the analog to digital conversion  
  ldrRawData = analogRead(LDR_PIN);
  
  // RESISTOR VOLTAGE_CONVERSION
  // Convert the raw digital data back to the voltage that was measured on the analog pin
  resistorVoltage = (float)ldrRawData / MAX_ADC_READING * ADC_REF_VOLTAGE;

  // voltage across the LDR is the 5V supply minus the 5k resistor voltage
  ldrVoltage = ADC_REF_VOLTAGE - resistorVoltage;
  
  // LDR_RESISTANCE_CONVERSION
  // resistance that the LDR would have for that voltage  
  ldrResistance = ldrVoltage/resistorVoltage * REF_RESISTANCE;
  
  // LDR_LUX
  // Change the code below to the proper conversion from ldrResistance to
  // ldrLux
  ldrLux = LUX_CALC_SCALAR * pow(ldrResistance, LUX_CALC_EXPONENT);
 
  // print out the results
  Serial.print("LDR Raw Data   : "); Serial.println(ldrRawData);
  Serial.print("LDR Voltage    : "); Serial.print(ldrVoltage); Serial.println(" volts");
  Serial.print("LDR Resistance : "); Serial.print(ldrResistance); Serial.println(" Ohms");
  Serial.print("LDR Illuminance: "); Serial.print(ldrLux); Serial.println(" lux");

  // Print out the LDR Illuminance to the LCD
  lcd.setCursor(0,0); // set cursor to column 0, line 1
  lcd.print("LDR Lux Meter");
    
  lcd.setCursor(0,1); // set cursor to column 0, line 2
  lcd.print(ldrLux);
  lcd.setCursor(7, 1); // set cursor to column 4, line 1
  lcd.print(" lux");
  
  delay(250);
}
