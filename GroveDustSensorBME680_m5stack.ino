/***************************************************************************
 * Environment Sensor Using m5stack + Pimoroni BME680 + Grove Dust Sensor
 * with Upload to Adafruit IO
 * 
 * Connect Pimoroni BME680 Breakout to the I2C pins on the m5stack
 * [5V, GND, SDA (Pin21), SCL (Pin22)]
 * 
 * Connect the Grove Dust Sensor (Shinyei PPD42NS)
 * 1 : COMMON(GND) [Black Wire on Grove Sensor]
 * 2 : OUTPUT(P2) [Not used on Grove Connectr] [Can be used for PM1.0]
 * 3 : INPUT(5VDC 90mA) [Red Wire on Grove Sensor]
 * 4 : OUTPUT(P1) [Yellow Wire on Grove Sensor] [Used for PM2.5 mesurements]
 * 5 : INPUT(T1)･･･FOR THRESHOLD FOR [P2] [Not used on Grove Connector]
 * 
 * Grove PIN    m5stack
 * GND <---->   GND
 * 5V  <---->   5V
 * P1  <---->   AD2 (PIN36)
 */

/***************************************************************************
//Unused Libraries
//#include <AdafruitIO_MQTT.h>
//#include <AdafruitIO_Ethernet.h>
//#include <AdafruitIO_FONA.h>
//#include <AdafruitIO_Definitions.h>
//#include <AdafruitIO_Time.h>
//#include <AdafruitIO_Group.h>
//#include <AdafruitIO_Data.h>
//#include <AdafruitIO_Dashboard.h>
/***************************************************************************/

//Included Librarires
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <AdafruitIO.h>
#include <AdafruitIO_WiFi.h>
#include <AdafruitIO_Feed.h>
#include <M5Stack.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch
//#include "config.h" // Adafruit IO credentials

// Setup Adafruit IO Credentials
/************************ Adafruit IO Config *******************************/
// visit io.adafruit.com if you need to create an account,
// or if you need your Adafruit IO key.
#define IO_USERNAME    "your_username"
#define IO_KEY         "your_AIO_key"

/******************************* WIFI **************************************/
// Setup Wifi Credentials
#define WIFI_SSID       "your_ssid"
#define WIFI_PASS       "your_pass"

// Setup Adafruit IO Wifi Settings
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

/***************************** IO FEED ************************************/
// Setup the Adafruit IO feed for the Gove Dust Sensor
// Input your feed key in between the " " (Quotation Marks)
// 
AdafruitIO_Feed *concentrationFeed = io.feed("env-ardconc");
AdafruitIO_Feed *temperatureFeed = io.feed("bme680-temp");
AdafruitIO_Feed *pressureFeed = io.feed("bme680-pres");
AdafruitIO_Feed *humidityFeed = io.feed("bme680-hum");
AdafruitIO_Feed *gasFeed = io.feed("bme680-gas");
AdafruitIO_Feed *altitudeFeed = io.feed("bme680-alt");


/***************************************************************************/

// Setup Variables for m5stack display
unsigned long drawTime = 0;

// Setup the Grove DustSensor
int pin = 36;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;//sampe 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

// Setup Variables for the BME680
float temp = 0;
float pressure = 0;
float humidity = 0;
float gas = 0;
float altitude = 0;

//Setup BME680
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C



void setup() 
{
    M5.begin();
    
    // Disable the speaker noise
    dacWrite(25, 0);

    // Display Start Message
    Serial.println((String)"Starting Sensor");
    M5.Lcd.println((String)"Starting Sensor");


     
    // Set Up Serial and Pin Mode for Dust Sensor
    Serial.begin(9600);
    pinMode(pin,INPUT);
    starttime = millis();//get the current time;

    // Test Connection of BME680 Sensor
    
    if (!bme.begin(0x76)) {
        Serial.println("Could not find a valid BME680 sensor, check wiring!");
        M5.Lcd.println("Could not find a valid BME680 sensor, check wiring!");
        while (1);
    }

    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms
  
    // Connect to io.adafruit.com
    Serial.print("Connecting to Adafruit IO");
    M5.Lcd.println("Connecting to Adafruit IO");
    io.connect();

    // Wait for a connection
    while (io.status() < AIO_CONNECTED)
    {
      Serial.print(".");
      M5.Lcd.print(".");
      
      // Button preses not working.
      if(M5.BtnA.wasPressed()) {
        M5.Lcd.print("A");
        Serial.print("A");
      }
      if(M5.BtnB.wasPressed()) {
        M5.Lcd.print("B");
        Serial.print("B");
       } 
      if(M5.BtnC.wasPressed()) {
        M5.Lcd.print("C");
        Serial.print("C");
       } 
      
      delay(2500);
    }

    // we are connected to Adafruit IO
    Serial.println();
    Serial.println(io.statusText());
    M5.Lcd.println(io.statusText());

    // (Optional) Send Default Value of 10
    //io.run();
    //concentrationFeed->save(10);

    // Setup Free Fonts
    // Setup Initial Position
    int xpos =  0;
    int ypos = 40;

    // For comaptibility with Adafruit_GFX library the text background is not plotted when using the print class
    // even if we specify it.
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setCursor(xpos, ypos);    // Set cursor near top left corner of screen


    // Set the Font for the readings
    M5.Lcd.setFreeFont(FSSB12);       // Select Free Sans Serif Bold 12 point font
    M5.Lcd.println((String)"Reading");                // Move cursor down a line
    
    // Display Zero Reading on Serial
    Serial.println((String)"L: "+lowpulseoccupancy+" R: "+ratio+" C: "+concentration+" ");

    // Display BME680 Readings on Serial
    Serial.println((String)"T: "+temp+" P: "+pressure+" H: "+humidity+" G: "+gas+" A: "+altitude+" ");


    // Display Zero Reading on m5stack TFT
    M5.Lcd.println((String)"L: "+lowpulseoccupancy+" ");
    M5.Lcd.println((String)"R: "+ratio+" ");
    M5.Lcd.println((String)"C: "+concentration+" "); 

    // Print the BME680 Readings
    M5.Lcd.println((String)"T: "+temp+" *C");
    M5.Lcd.println((String)"P: "+pressure+" hPa");
    M5.Lcd.println((String)"H: "+humidity+" %");  
    M5.Lcd.println((String)"G: "+gas+" KOhms");
    M5.Lcd.println((String)"A: "+altitude+" m");

    
    delay(5000);
}

void loop() 
{
    // Run Adafruit IO
    io.run();

    // Start raeding BME680 Sensors
    if (! bme.performReading()) {
        Serial.println("Failed to perform reading :(");
        return;
      }

    temp = bme.temperature;
    pressure = bme.pressure / 100.0;
    humidity = bme.humidity;
    gas = bme.gas_resistance / 1000.0;
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

    // Start Reading from Grove Dust Sensor
    duration = pulseIn(pin, LOW);
    lowpulseoccupancy = lowpulseoccupancy+duration;
    
    if ((millis()-starttime) > sampletime_ms)//if the sampel time == 30s
    {
        ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
        concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
        Serial.print(lowpulseoccupancy);
        Serial.print(",");
        Serial.print(ratio);
        Serial.print(",");
        Serial.println(concentration);

        // Turn on the display and display values:

        // Display on Serial
        Serial.println((String)"L: "+lowpulseoccupancy+" R: "+ratio+" C: "+concentration+" ");
        Serial.println((String)"T: "+temp+" P: "+pressure+" H: "+humidity+" G: "+gas+" A: "+altitude+" ");

        // Setup Free Fonts
        // Setup Initial Position
        int xpos =  0;
        int ypos = 40;  

        // Clear the Screen, Overwite Old Values
        M5.Lcd.fillScreen(BLACK);
        delay(500);

        // For comaptibility with Adafruit_GFX library the text background is not plotted when using the print class
        // even if we specify it.
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setCursor(xpos, ypos);    // Set cursor near top left corner of screen

        // Set the Font for the readings
        M5.Lcd.setFreeFont(FSSB24);       // Select Free Sans Serif Bold 24 point font


        // Change Text colour and First Line based on Concentration Values
        // 
        if (concentration >= 10000)
        {
            // Set Colour to RED
            M5.Lcd.setTextColor(TFT_RED);
            M5.Lcd.println((String)"Danger");  // Display Summary
        }


        else if (concentration >= 3000 && concentration < 10000)
        {
            // Set Colour to ORANGE
            M5.Lcd.setTextColor(TFT_ORANGE);
            M5.Lcd.println((String)"Bad Quality");  // Display Summary

        }

        else if (concentration >= 1000 && concentration < 3000)
        {
            // Set Colour to YELLOW
            M5.Lcd.setTextColor(TFT_YELLOW);
            M5.Lcd.println((String)"Poor Quality");  // Display Summary

        }


        else if (concentration >= 500 && concentration < 1000)
        {
            // Set Colour to GREEN
            M5.Lcd.setTextColor(TFT_GREEN);
            M5.Lcd.println((String)"Avg Quality");  // Display Summary
            
        }

        else
        {
            // Set Colour to WHITE
            M5.Lcd.setTextColor(TFT_WHITE);
            M5.Lcd.println((String)"Good Quality");  // Display Summary

        }
        


        // Print the readings on the M5Stack TFT
        M5.Lcd.println((String)"L: "+lowpulseoccupancy+" ");
        M5.Lcd.println((String)"R: "+ratio+" ");
        M5.Lcd.println((String)"C: "+concentration+" ");  

        // Print the BME680 Readings
        M5.Lcd.println((String)"T: "+temp+" *C");
        M5.Lcd.println((String)"P: "+pressure+" hPa");
        M5.Lcd.println((String)"H: "+humidity+" %");  
        M5.Lcd.println((String)"G: "+gas+" KOhms");
        M5.Lcd.println((String)"A: "+altitude+" m");


        // send data to Adafruit IO feeds
        concentrationFeed->save(concentration);
        temperatureFeed->save(temp);
        pressureFeed->save(pressure);
        humidityFeed->save(humidity);
        gasFeed->save(gas);
        altitudeFeed->save(altitude);

        // Confirm Post on serial
        Serial.println((String)"Sent: "+concentration+", "+temp+", "+pressure+", "+humidity+", "+gas+", "+altitude+" ");
        

        
        delay(2000);
        
        lowpulseoccupancy = 0;
        starttime = millis();
        
        // clear screen for the next loop:
        // lcd.clear();
    }

}

// There follows a crude way of flagging that this example sketch needs fonts which
// have not been enbabled in the User_Setup.h file inside the TFT_HX8357 library.
//
// These lines produce errors during compile time if settings in User_Setup are not correct
//
// The error will be "does not name a type" but ignore this and read the text between ''
// it will indicate which font or feature needs to be enabled
//
// Either delete all the following lines if you do not want warnings, or change the lines
// to suit your sketch modifications.

#ifndef LOAD_GLCD
//ERROR_Please_enable_LOAD_GLCD_in_User_Setup
#endif

#ifndef LOAD_FONT2
//ERROR_Please_enable_LOAD_FONT2_in_User_Setup!
#endif

#ifndef LOAD_FONT4
//ERROR_Please_enable_LOAD_FONT4_in_User_Setup!
#endif

#ifndef LOAD_FONT6
//ERROR_Please_enable_LOAD_FONT6_in_User_Setup!
#endif

#ifndef LOAD_FONT7
//ERROR_Please_enable_LOAD_FONT7_in_User_Setup!
#endif

#ifndef LOAD_FONT8
//ERROR_Please_enable_LOAD_FONT8_in_User_Setup!
#endif

#ifndef LOAD_GFXFF
ERROR_Please_enable_LOAD_GFXFF_in_User_Setup!
#endif

/***************************************************************************
/****************************** Sources ************************************
/***************************************************************************
// Adafruit IO Publish Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/
