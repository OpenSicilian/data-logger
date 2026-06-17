#include <HX711.h>
#include <SPI.h>
#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>


//--------------------
// Pin definitions
//--------------------

#define DT1 2
#define DT2 3
#define DT3 4
#define DT4 5

#define SCK1 6
#define SCK2 8
#define SCK3 9
#define SCK4 A0

#define TEMP_PIN 7
#define SD_CS 10


//--------------------
// Objects
//--------------------

HX711 scale1;
HX711 scale2;
HX711 scale3;
HX711 scale4;

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

File logfile;


//--------------------
// Timing
//--------------------

unsigned long lastLog = 0;

const unsigned long LOG_INTERVAL =
30000UL;


//--------------------
// Calibration values
//--------------------

float cal1 = -3782;
float cal2 = -3600.5;
float cal3 = -3610;
float cal4 = -3720.6;
//Machine 2
float cal5 = -3841; //-3628
float cal6 = -3927; //-3858
float cal7 = -3700; //-2428
float cal8 = -3723; //-3018
//Machine 3
float cal9 = -3763; 
float cal10 = -3836; 
float cal11 = -3500; //maybe lowerslightly
float cal12 = -3857; 

//--------------------
// Filter variables
//--------------------

float fc1 = 0;
float fc2 = 0;
float fc3 = 0;
float fc4 = 0;
float ftotal = 0;

bool filterInitialized = false;

const float ALPHA = 0.2;


//--------------------
// Filter initialization
//--------------------

void initializeFilter(
float c1,
float c2,
float c3,
float c4,
float total)
{
    fc1 = c1;
    fc2 = c2;
    fc3 = c3;
    fc4 = c4;
    ftotal = total;

    filterInitialized = true;

    Serial.println("Filter Initialized");
}


//--------------------
// Setup
//--------------------

void setup()
{
    Serial.begin(115200);

    Serial.println("Starting Logger");


    scale1.begin(DT1, SCK1);
    scale2.begin(DT2, SCK2);
    scale3.begin(DT3, SCK3);
    scale4.begin(DT4, SCK4);


    scale1.set_scale(cal5);
    scale2.set_scale(cal6);
    scale3.set_scale(cal7);
    scale4.set_scale(cal8);


    Serial.println("Remove load");

    delay(60000);


    scale1.tare();
    scale2.tare();
    scale3.tare();
    scale4.tare();

    Serial.println("Tare complete");


    sensors.begin();

    pinMode(SD_CS, OUTPUT);

    if(!SD.begin(SD_CS))
    {
        Serial.println("SD FAILED");

        while(1);
    }


    if(!SD.exists("weights.csv"))
    {
        logfile =
        SD.open(
        "weights.csv",
        FILE_WRITE);

        logfile.println(
        "Time_s,TempC,Corner1,Corner2,Corner3,Corner4,Total");

        logfile.close();
    }

    Serial.println("Logger Ready");
    Serial.println("Place load, wait stable, then send 'r' to reset filter");
}


//--------------------
// Main Loop
//--------------------

void loop()
{
    //-------------------
    // Serial commands
    //-------------------

    if(Serial.available())
    {
        char ch = Serial.read();

        if(ch == 'r')
        {
            filterInitialized = false;

            Serial.println("Filter Reset Requested");
        }
    }


    //-------------------
    // Logging timer
    //-------------------

    if(millis() - lastLog >= LOG_INTERVAL)
    {
        lastLog = millis();


        //-------------------
        // Temperature
        //-------------------

        sensors.requestTemperatures();

        float temp =
        sensors.getTempCByIndex(0);


        //-------------------
        // Load cells
        //-------------------

        float c1 =
        scale1.get_units(50);

        float c2 =
        scale2.get_units(50);

        float c3 =
        scale3.get_units(50);

        float c4 =
        scale4.get_units(50);


        float total =
        c1 + c2 + c3 + c4;


        //-------------------
        // Exponential filter
        //-------------------

        if(!filterInitialized)
        {
            initializeFilter(
            c1,
            c2,
            c3,
            c4,
            total);
        }
        else
        {
            fc1 =
            ALPHA * c1 +
            (1.0 - ALPHA) * fc1;

            fc2 =
            ALPHA * c2 +
            (1.0 - ALPHA) * fc2;

            fc3 =
            ALPHA * c3 +
            (1.0 - ALPHA) * fc3;

            fc4 =
            ALPHA * c4 +
            (1.0 - ALPHA) * fc4;

            ftotal =
            ALPHA * total +
            (1.0 - ALPHA) * ftotal;
        }


        //-------------------
        // Log to SD
        //-------------------

        logfile =
        SD.open(
        "weights.csv",
        FILE_WRITE);


        if(logfile)
        {
            logfile.print(
            millis()/1000);

            logfile.print(",");

            logfile.print(
            temp,1);

            logfile.print(",");

            logfile.print(
            fc1,2);

            logfile.print(",");

            logfile.print(
            fc2,2);

            logfile.print(",");

            logfile.print(
            fc3,2);

            logfile.print(",");

            logfile.print(
            fc4,2);

            logfile.print(",");

            logfile.println(
            ftotal,2);

            logfile.close();


            //-------------------
            // Serial output
            //-------------------

            Serial.println("Logged to SD");

            Serial.print(
            millis()/1000);

            Serial.print(",");

            Serial.print(
            temp,1);

            Serial.print(",");

            Serial.print(
            fc1,2);

            Serial.print(",");

            Serial.print(
            fc2,2);

            Serial.print(",");

            Serial.print(
            fc3,2);

            Serial.print(",");

            Serial.print(
            fc4,2);

            Serial.print(",");

            Serial.println(
            ftotal,2);
        }

        else
        {
            Serial.println(
            "SD write failed");
        }
    }
}