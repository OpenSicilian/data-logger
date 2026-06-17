#include <HX711.h>

HX711 scale;

float calibration_factor=-3378;

void setup()
{
    Serial.begin(115200);

    scale.begin(2,6);

    scale.tare();

    scale.set_scale(calibration_factor);

    Serial.println("+ or - changes factor");
}

void loop()
{
    Serial.print("Reading: ");
    Serial.print(scale.get_units(),2);

    Serial.print(" Factor:");

    if(Serial.available())
    {
        char c=Serial.read();

        if(c=='+')
            calibration_factor+=10;

        if(c=='-')
            calibration_factor-=10;

        scale.set_scale(calibration_factor);
    }
}