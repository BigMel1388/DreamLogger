#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include "Adafruit_CCS811.h"
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>

#define SEALEVELPRESSURE_HPA (1013.25)

/* SUPABASE */
const char *supabaseUrl = "https://wjvjxxrgwipmarpwpebp.supabase.co/rest/v1/sensor_data";
const char *supabaseKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Indqdmp4eHJnd2lwbWFycHdwZWJwIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzM2NjExNjcsImV4cCI6MjA4OTIzNzE2N30.5z2fmWyVL5X8PImk4fLkn_M5J11YGYlGSMVOzHyqJPM";

/* LCD */
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* Sensors */
Adafruit_BME280 bme;
Adafruit_CCS811 ccs;

/* WiFi */
WiFiManager wifiManager;

/* Sensor flags */
bool bmeReady = false;
bool ccsReady = false;

/* Send data to Supabase */
void sendDataToSupabase()
{
    if (WiFi.status() != WL_CONNECTED)
        return;

    float temperature = bmeReady ? bme.readTemperature() : 0;
    float humidity = bmeReady ? bme.readHumidity() : 0;
    float pressure = bmeReady ? bme.readPressure() / 100.0F : 0;

    float co2 = 0;
    if (ccsReady && ccs.available() && !ccs.readData())
        co2 = ccs.geteCO2();

    String json = "{";
    json += "\"temperature\":" + String(temperature, 2) + ",";
    json += "\"humidity\":" + String(humidity, 2) + ",";
    json += "\"pressure\":" + String(pressure, 2) + ",";
    json += "\"co2\":" + String(co2);
    json += "}";

    HTTPClient http;

    http.begin(supabaseUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", supabaseKey);
    http.addHeader("Authorization", String("Bearer ") + supabaseKey);
    http.addHeader("Prefer", "return=minimal");

    int response = http.POST(json);

    Serial.print("Supabase response: ");
    Serial.println(response);

    http.end();
}

/* LCD display */
void updateLCDDisplay()
{
    static int screen = 0;

    lcd.clear();

    if (screen == 0 && bmeReady)
    {
        lcd.setCursor(0, 0);
        lcd.print("Temp:");
        lcd.print(bme.readTemperature(), 1);

        lcd.setCursor(0, 1);
        lcd.print("Hum:");
        lcd.print(bme.readHumidity(), 1);
    }

    if (screen == 1 && bmeReady)
    {
        lcd.setCursor(0, 0);
        lcd.print("Pressure");

        lcd.setCursor(0, 1);
        lcd.print(bme.readPressure() / 100.0F);
    }

    if (screen == 2 && ccsReady && ccs.available() && !ccs.readData())
    {
        lcd.setCursor(0, 0);
        lcd.print("CO2");

        lcd.setCursor(0, 1);
        lcd.print((int)ccs.geteCO2());
        lcd.print("ppm");
    }

    screen = (screen + 1) % 3;
}

void setup()
{
    Serial.begin(115200);

    lcd.init();
    lcd.backlight();

    Wire.begin(21, 22);

    /* BME280 */
    if (bme.begin(0x76))
    {
        bmeReady = true;
        Serial.println("BME280 ready");
    }

    /* CCS811 */
    if (ccs.begin())
    {
        ccsReady = true;
        Serial.println("CO2 sensor ready");
    }

    /* WiFi */
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi");

    if (!wifiManager.autoConnect("SleepTracker-ESP32", "password123"))
    {
        ESP.restart();
    }

    Serial.println("WiFi Connected");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
}

void loop()
{
    static unsigned long lastDisplay = 0;
    static unsigned long lastSupabase = 0;

    /* Update LCD every 3s */
    if (millis() - lastDisplay > 3000)
    {
        updateLCDDisplay();
        lastDisplay = millis();
    }

    /* Send to Supabase every 5s */
    if (millis() - lastSupabase > 5000)
    {
        sendDataToSupabase();
        lastSupabase = millis();
    }
}