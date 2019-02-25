#include <Wire.h>
#include <rgb_lcd.h>


int pin = 8;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;//sampe 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

// Setup LCD
rgb_lcd lcd;



void setup() 
{
    // Set Up Serial and Pin Mode for Dust Sensor
    Serial.begin(9600);
    pinMode(pin,INPUT);
    starttime = millis();//get the current time;

    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    lcd.setRGB(75, 0, 130);
    lcd.print((String)"Starting Sensor");
    delay(5000);

    
    // Display Zero Reading
    // Set Colour to White
    lcd.setRGB(255, 255, 255);
    // set the cursor to (0,0):
    lcd.setCursor(0, 0);
    lcd.print((String)"L: "+lowpulseoccupancy+" R: "+ratio+" ");

    // set the cursor to (0,1):
    lcd.setCursor(0,1);
    lcd.print((String)"C: "+concentration+" ");
    
    delay(5000);
}

void loop() 
{
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

        // Change LED colour based on Concentration Values
        // High Dust Concentration Over 20K
        if (concentration >= 18000)
        {
            // Set Colour to RED
            lcd.setRGB(255, 0, 0);
        }


        else if (concentration >= 10000 && concentration < 18000)
        {
            // Set Colour to ORANGE
            lcd.setRGB(255, 127, 0);
        }

        else if (concentration >= 5000 && concentration < 10000)
        {
            // Set Colour to YELLOW
            lcd.setRGB(255, 255, 0);
        }


        else if (concentration >= 2000 && concentration < 5000)
        {
            // Set Colour to GREEN
            lcd.setRGB(0, 255, 0);
        }

        else
        {
            // Set Colour to Blue
            lcd.setRGB(0, 0, 255);
        }
        
        // Turn on the display and display values:
        // set the cursor to (0,0):
        lcd.setCursor(0, 0);
        lcd.print((String)"L: "+lowpulseoccupancy+" R: "+ratio+" ");

        // set the cursor to (0,1):
        lcd.setCursor(0,1);
        lcd.print((String)"C: "+concentration+" ");
        
        // delay(500);        
        // lcd.setRGB(r, g, b);
        
        lowpulseoccupancy = 0;
        starttime = millis();
        
        // clear screen for the next loop:
        // lcd.clear();
    }
}
