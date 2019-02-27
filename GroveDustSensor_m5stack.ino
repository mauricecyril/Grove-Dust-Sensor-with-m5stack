//#include <AdafruitIO_MQTT.h>
//#include <AdafruitIO_Ethernet.h>
//#include <AdafruitIO_FONA.h>
//#include <AdafruitIO_Definitions.h>
//#include <AdafruitIO_Time.h>
//#include <AdafruitIO_Group.h>
//#include <AdafruitIO_Data.h>
//#include <AdafruitIO_Dashboard.h>

#include <AdafruitIO.h>
#include <AdafruitIO_WiFi.h>
#include <AdafruitIO_Feed.h>
#include <M5Stack.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

// Setup Adafruit IO Credentials
/************************ Adafruit IO Config *******************************/
// visit io.adafruit.com if you need to create an account,
// or if you need your Adafruit IO key.
//#define IO_USERNAME    "your_username"
//#define IO_KEY         "your_AIO_key"

/******************************* WIFI **************************************/
// Setup Wifi Credentials
//#define WIFI_SSID       "your_ssid"
//#define WIFI_PASS       "your_pass"


// Setup Adafruit IO Wifi Settings
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

/***************************** IO FEED ************************************/
// Setup the Adafruit IO feed for the Gove Dust Sensor
// Input your feed key in between the " " (Quotation Marks)
// 
AdafruitIO_Feed *concentrationFeed = io.feed("env-ardconc");


// Setup Variables for m5stack display
unsigned long drawTime = 0;

// Setup the Grove DustSensor
int pin = 21;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;//sampe 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;




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
    M5.Lcd.setFreeFont(FSSB24);       // Select Free Sans Serif Bold 24 point font
    M5.Lcd.println((String)"Reading");                // Move cursor down a line
    
    // Display Zero Reading on Serial
    Serial.println((String)"L: "+lowpulseoccupancy+" R: "+ratio+" C: "+concentration+" ");

    // Display Zero Reading on m5stack TFT
    M5.Lcd.println((String)"L: "+lowpulseoccupancy+" ");
    M5.Lcd.println((String)"R: "+ratio+" ");
    M5.Lcd.println((String)"C: "+concentration+" "); 
    
    delay(5000);
}

void loop() 
{
    // Run Adafruit IO
    io.run();

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
        Serial.println((String)"L: "+lowpulseoccupancy+" R: "+ratio+" C: "+concentration+" ");

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
        // High Dust Concentration Over 20K
        if (concentration >= 18000)
        {
            // Set Colour to RED
            M5.Lcd.setTextColor(TFT_RED);
            M5.Lcd.println((String)"Danger");  // Display Summary
        }


        else if (concentration >= 10000 && concentration < 18000)
        {
            // Set Colour to ORANGE
            M5.Lcd.setTextColor(TFT_ORANGE);
            M5.Lcd.println((String)"Bad Quality");  // Display Summary

        }

        else if (concentration >= 5000 && concentration < 10000)
        {
            // Set Colour to YELLOW
            M5.Lcd.setTextColor(TFT_YELLOW);
            M5.Lcd.println((String)"Med Quality");  // Display Summary

        }


        else if (concentration >= 2000 && concentration < 5000)
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

        // send data to Adafruit IO feeds
        concentrationFeed->save(concentration);

        // Confirm Post on serial
        Serial.println((String)"Sent: "+concentration);

        
        delay(5000);
        
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
