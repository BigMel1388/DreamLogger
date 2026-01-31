
//Library includes
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>



// Define I2C address or SPI pins
#define SEALEVELPRESSURE_HPA (1013.25)
LiquidCrystal_I2C lcd(0x27,16,2);
Adafruit_BME280 bme;

void setup() {
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("BME280 + LCD");
    delay(1500);
    lcd.clear();
    if (!bme.begin(0x76)) { // Replace 0x76 with your I2C address
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (9);
    }
}

void loop() {
    float temperature = bme.readTemperature();        // °C
    float humidity    = bme.readHumidity();           // %
    float pressure    = bme.readPressure() / 100.0;   // hPa
    float altitude    = bme.readAltitude(SEALEVELPRESSURE_HPA);  // meters
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print("C");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.print("%");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Pressure: ");
    lcd.print(pressure);
    lcd.print("hPa");
    delay(2000);
    
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

  

  


  

  

