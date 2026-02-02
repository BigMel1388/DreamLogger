
// Library includes
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include "Adafruit_CCS811.h"

// Define I2C address or SPI pins
#define SEALEVELPRESSURE_HPA (1013.25)
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_BME280 bme;
Adafruit_CCS811 ccs;

void setup()
{
    // Initialize Serial, LCD, BME280, and CCS811
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("BME280 + LCD + Co2");
    delay(1500);
    lcd.clear();
    Wire.begin(21, 22); // SDA: GPIO21, SCL: GPIO22
    if (!ccs.begin())
    {
        Serial.println("Failed to start sensor! Please check your wiring.");
        while (1)
            ;
    }
    while (!ccs.available())
        ; // Wait for the sensor to be ready
    if (!bme.begin(0x76))
    { // Replace 0x76 with your I2C address
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (9)
            ;
    }
}

void loop()
{
    // variable to hold sensor data
    float temperature = bme.readTemperature();               // °C
    float humidity = bme.readHumidity();                     // %
    float pressure = bme.readPressure() / 100.0;             // hPa
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA); // meters
    float carbonDioxide = ccs.geteCO2();                     // ppm
    // Display data on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print("C");
    lcd.setCursor(0, 1);
    lcd.print("CO2: ");
    lcd.print(carbonDioxide);
    lcd.print("ppm");
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.print("%");
    lcd.setCursor(0, 1);
    lcd.print("Pressure: ");
    lcd.print(pressure);
    lcd.print("hPa");
    delay(500);

    // Read and print CCS811 data to serial monitor because it looks cool

    if (ccs.available())
    {
        if (!ccs.readData())
        {
            Serial.print("eCO2: ");
            Serial.print(ccs.geteCO2());
            Serial.print(" ppm, TVOC: ");
            Serial.print(ccs.getTVOC());
            Serial.println(" ppb");
        }
        else
        {
            Serial.println("Error reading sensor data");
        }
    }
    // Read and print BME280 data to serial monitor because it looks cool
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" °C");

    Serial.print("Pressure = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();
    delay(2000);
}
