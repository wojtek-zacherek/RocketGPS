/*
LiquidCrystal Library - Hello World
This code is for an Arduino Nano

Demonstrates the use a 16x2 LCD display. The LiquidCrystal
library works with all LCD displays that are compatible with the
Hitachi HD44780 driver. There are many of them out there, and you
can usually tell them by the 16-pin interface.

This sketch prints "Hello World!" to the LCD
and shows the time.

The circuit:
LCD Pins Nano Pins
------------- --------------
Pin 1) VSS Connect to - 0 volts (ground)
Pin 2) VDD Connect to + 5 volts
Pin 3) V0 Connect to center of 10K pot

Pin 4) RS Connect to Pin D2
Pin 5) RW Connect to - 0 volts (ground)
Pin 6) E Connect to Pin D3
Pin 7) D0 Not Used N/A
Pin 8) D1 Not Used N/A
Pin 9) D2 Not Used N/A
Pin 10) D3 Not Used N/A
Pin 11) D4 Connect to Pin D4
Pin 12) D5 Connect to Pin D5
Pin 13) D6 Connect to Pin D6
Pin 14) D7 Connect to Pin D7
Pin 15) A Connect to 220 ohm resistor to +5 volts
Pin 16) K Connect to - 0 volts (ground)

Library originally added 18 Apr 2008
by David A. Mellis
library modified 5 Jul 2009
by Limor Fried (http://www.ladyada.net)
example added 9 Jul 2009
by Tom Igoe
modified 22 Nov 2010
by Tom Igoe

This example code is in the public domain.

http://www.arduino.cc/en/Tutorial/LiquidCrystal
*/










/*
  Reading lat and long via UBX binary commands - no more NMEA parsing!
  By: Nathan Seidle
  SparkFun Electronics
  Date: January 3rd, 2019
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to query a u-blox module for its lat/long/altitude. We also
  turn off the NMEA output on the I2C port. This decreases the amount of I2C traffic 
  dramatically.

  Note: Long/lat are large numbers because they are * 10^7. To convert lat/long
  to something google maps understands simply divide the numbers by 10,000,000. We 
  do this so that we don't have to use floating point numbers.

  Leave NMEA parsing behind. Now you can simply ask the module for the datums you want!

  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  ZED-F9P RTK2: https://www.sparkfun.com/products/15136
  NEO-M8P RTK: https://www.sparkfun.com/products/15005
  SAM-M8Q: https://www.sparkfun.com/products/15106

  Hardware Connections:
  Plug a Qwiic cable into the GNSS and a BlackBoard
  If you don't have a platform with a Qwiic connection use the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
  Open the serial monitor at 115200 baud to see the output
*/

#include <Wire.h> //Needed for I2C to GNSS
// include the library code:
#include <LiquidCrystal.h>

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;

long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to u-blox module.
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
byte packet[34]; // 18 + 16data

void setup()
{
  Serial.begin(9600);
  while (!Serial); //Wait for user to open terminal
  Serial.println("SparkFun u-blox Example");

  Wire.begin();

  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

  lcd.begin(16, 2);
  lcd.print("Hello World!");
  delay(1000);

  packet[0] = 0x7E;
  packet[1] = 0x00;
  packet[2] = 0x1E;
  packet[3] = 0x10;
  packet[4] = 0x00;
  packet[5] = 0x00;
  packet[6] = 0x13;
  packet[7] = 0xA2;
  packet[8] = 0x00;
  packet[9] = 0x41;
  packet[10] = 0xD3;
  packet[11] = 0x60;
  packet[12] = 0xF2;
  packet[13] = 0xFF;
  packet[14] = 0xFE;
  packet[15] = 0x00;
  packet[16] = 0x00;
}


void makePacket(long lonn, long latt, long alt, long siv){
  uint8_t temp = 0;
  packet[17] = (byte)(lonn >> 24);
  packet[18] = (byte)(lonn >> 16);
  packet[19] = (byte)(lonn >> 8);
  packet[20] = (byte)(lonn);
  packet[21] = (byte)(latt >> 24);
  packet[22] = (byte)(latt >> 16);
  packet[23] = (byte)(latt >> 8);
  packet[24] = (byte)(latt);
  packet[25] = (byte)(alt >> 24);
  packet[26] = (byte)(alt >> 16);
  packet[27] = (byte)(alt >> 8);
  packet[28] = (byte)(alt);
  packet[29] = (byte)(siv >> 24);
  packet[30] = (byte)(siv >> 16);
  packet[31] = (byte)(siv >> 8);
  packet[32] = (byte)(siv);

  for(int i = 3; i < 33; i++){
    temp += packet[i];
  }
  temp = 0xFF - temp;
  packet[33] = temp;

  Serial.write(packet,34);
}

void loop()
{
  //Query module only every second. Doing it more often will just cause I2C traffic.
  //The module only responds when a new position is available
  if (millis() - lastTime > 100)
  {
    lastTime = millis(); //Update the timer
    
    long latitude = myGNSS.getLatitude();
    Serial.print(F("Lat: "));
    Serial.print(latitude);

    long longitude = myGNSS.getLongitude();
    Serial.print(F(" Long: "));
    Serial.print(longitude);
    Serial.print(F(" (degrees * 10^-7)"));

    long altitude = myGNSS.getAltitude();
    Serial.print(F(" Alt: "));
    Serial.print(altitude);
    Serial.print(F(" (mm)"));

    byte SIV = myGNSS.getSIV();
    Serial.print(F(" SIV: "));
    Serial.print(SIV);

    Serial.println();

    makePacket(longitude,latitude,altitude,SIV);

    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 0);
    // print the number of seconds since reset:
    lcd.print(abs(latitude/10000000));
    lcd.print('.');
    lcd.print(abs((latitude%10000000)/10000));
    if(latitude > 0){
      lcd.print('N');
    }else{
      lcd.print('S');
    }
    lcd.print(' ');
    lcd.print(abs(longitude/10000000));
    lcd.print('.');
    lcd.print(abs((longitude%10000000)/10000));
    if(longitude > 0){
      lcd.print('E');
    }else{
      lcd.print('W');
    }

    lcd.setCursor(0,1);
    lcd.print("alt: ");
    lcd.print((altitude/1000));
    lcd.print(".");
    lcd.print(altitude%1000/10);
    lcd.print("m   ");
  }
}

void serialEvent(){
  while(Serial.available()){
    Serial.read();
  }
}
