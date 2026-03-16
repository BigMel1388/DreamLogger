#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include "Adafruit_CCS811.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>

#define SEALEVELPRESSURE_HPA (1013.25)

// Static IP configuration - updated to match actual ESP32 IP
IPAddress staticIP(172, 23, 83, 73);
IPAddress gateway(172, 23, 83, 1);
IPAddress subnet(255, 255, 255, 0);

// Sensor objects
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_BME280 bme;
Adafruit_CCS811 ccs;
WebServer server(80);
WiFiManager wifiManager;

// Sensor readiness flags
bool bmeReady = false;
bool ccsReady = false;

// Function prototypes
void updateLCDDisplay();
void printSensorDataToSerial();

void handleSensor()
{
    Serial.println("🔄 Received /sensor request from React Native app");

    float temperature = bmeReady ? bme.readTemperature() : 0.0;
    float humidity = bmeReady ? bme.readHumidity() : 0.0;
    float pressure = bmeReady ? bme.readPressure() / 100.0F : 0.0;

    float co2 = 0;
    if (ccsReady && ccs.available() && !ccs.readData())
    {
        co2 = ccs.geteCO2();
    }

    String json = "{";
    json += "\"temperature\":" + String(temperature, 2) + ",";
    json += "\"humidity\":" + String(humidity, 2) + ",";
    json += "\"pressure\":" + String(pressure, 2) + ",";
    json += "\"co2\":" + String(co2);
    json += "}";

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.sendHeader("Content-Type", "application/json");
    server.send(200, "application/json", json);

    Serial.print("✅ Sent sensor data: ");
    Serial.println(json);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("\n🚀 ESP32 Sleep Tracker Sensor Starting...");

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Sleep Tracker");
    lcd.setCursor(0, 1);
    lcd.print("Initializing...");
    delay(2000);
    lcd.clear();

    Wire.begin(21, 22);
    Serial.println("🔧 I2C initialized");

    lcd.setCursor(0, 0);
    lcd.print("Init CO2 Sensor");
    ccsReady = false;
    if (!ccs.begin())
    {
        Serial.println("❌ Failed to start CO2 sensor!");
        lcd.setCursor(0, 1);
        lcd.print("CO2 Sensor Error");
        delay(2000);
    }
    else
    {
        ccsReady = true;
        Serial.println("✅ CO2 sensor initialized");

        unsigned long startTime = millis();
        while (!ccs.available() && (millis() - startTime) < 10000)
        {
            delay(100);
        }

        if (ccs.available())
        {
            Serial.println("✅ CO2 sensor ready");
        }
        else
        {
            Serial.println("⚠️ CO2 sensor timeout, continuing anyway");
            ccsReady = false;
        }
    }

    lcd.setCursor(0, 0);
    lcd.print("Init Env Sensor");
    bmeReady = false;
    if (!bme.begin(0x76))
    {
        Serial.println("❌ Could not find BME280 sensor!");
        lcd.setCursor(0, 1);
        lcd.print("BME280 Error");
        delay(2000);
    }
    else
    {
        bmeReady = true;
        Serial.println("✅ BME280 sensor initialized");
    }

    lcd.setCursor(0, 0);
    lcd.print("WiFi Setup");
    Serial.println("📡 Setting up WiFi...");

    IPAddress dns1(8, 8, 8, 8);
    wifiManager.setSTAStaticIPConfig(staticIP, gateway, subnet, dns1);
    wifiManager.setSaveConfigCallback([]()
                                      { Serial.println("✅ WiFi config saved!"); });

    Serial.println("🔄 Attempting WiFi connection...");
    if (!wifiManager.autoConnect("SleepTracker-ESP32", "password123"))
    {
        Serial.println("❌ Failed to connect to WiFi");
        lcd.setCursor(0, 1);
        lcd.print("WiFi Failed");
        delay(3000);
        ESP.restart();
    }

    Serial.println("✅ WiFi connected!");
    Serial.print("📍 IP Address: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("IP Address:");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());
    delay(3000);

    server.on("/sensor", HTTP_GET, handleSensor);
    server.on("/", HTTP_GET, []()
              { server.send(200, "text/plain", "Sleep Tracker ESP32 is running"); });

    server.begin();
    delay(500);

    Serial.println("✅ HTTP server started on port 80");
    Serial.print("🔗 Access at: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/sensor");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Server Ready!");
    lcd.setCursor(0, 1);
    lcd.print("IP: " + WiFi.localIP().toString().substring(0, 15));
}

void loop()
{
    server.handleClient();

    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate > 3000)
    {
        updateLCDDisplay();
        lastDisplayUpdate = millis();
    }

    static unsigned long lastSerialUpdate = 0;
    if (millis() - lastSerialUpdate > 5000)
    {
        printSensorDataToSerial();
        lastSerialUpdate = millis();
    }
}

void updateLCDDisplay()
{
    static int screen = 0;
    static unsigned long lastScreenChange = 0;

    if (millis() - lastScreenChange > 2000)
    {
        screen = (screen + 1) % 3;
        lastScreenChange = millis();
    }

    lcd.clear();

    if (screen == 0)
    {
        if (bmeReady)
        {
            float temp = bme.readTemperature();
            float hum = bme.readHumidity();
            lcd.setCursor(0, 0);
            lcd.print("Temp: ");
            lcd.print(temp, 1);
            lcd.print("C");
            lcd.setCursor(0, 1);
            lcd.print("Hum: ");
            lcd.print(hum, 1);
            lcd.print("%");
        }
        else
        {
            lcd.setCursor(0, 0);
            lcd.print("BME280 Sensor");
            lcd.setCursor(0, 1);
            lcd.print("Not Available");
        }
    }
    else if (screen == 1)
    {
        if (bmeReady)
        {
            float pres = bme.readPressure() / 100.0F;
            lcd.setCursor(0, 0);
            lcd.print("Pressure:");
            lcd.setCursor(0, 1);
            lcd.print(pres, 1);
            lcd.print(" hPa");
        }
        else
        {
            lcd.setCursor(0, 0);
            lcd.print("Pressure Sensor");
            lcd.setCursor(0, 1);
            lcd.print("Not Available");
        }
    }
    else
    {
        if (ccsReady && ccs.available() && !ccs.readData())
        {
            float co2 = ccs.geteCO2();
            lcd.setCursor(0, 0);
            lcd.print("CO2 Level:");
            lcd.setCursor(0, 1);
            lcd.print((int)co2);
            lcd.print(" ppm");
        }
        else
        {
            lcd.setCursor(0, 0);
            lcd.print("CO2 Sensor");
            lcd.setCursor(0, 1);
            lcd.print("Not Available");
        }
    }
}

void printSensorDataToSerial()
{
    Serial.println("\n📊 Sensor Data for React Native App:");

    if (bmeReady)
    {
        float temperature = bme.readTemperature();
        float humidity = bme.readHumidity();
        float pressure = bme.readPressure() / 100.0F;

        Serial.print("🌡️  Temperature: ");
        Serial.print(temperature, 2);
        Serial.println(" °C");

        Serial.print("💧 Humidity: ");
        Serial.print(humidity, 2);
        Serial.println(" %");

        Serial.print("📊 Pressure: ");
        Serial.print(pressure, 2);
        Serial.println(" hPa");
    }
    else
    {
        Serial.println("🌡️  BME280 Sensor: Not Available");
    }

    if (ccsReady && ccs.available() && !ccs.readData())
    {
        float co2 = ccs.geteCO2();
        Serial.print("🌬️  CO2: ");
        Serial.print((int)co2);
        Serial.println(" ppm");
    }
    else
    {
        Serial.println("🌬️  CO2 Sensor: Not Available");
    }

    Serial.println("📡 Ready for React Native app requests!");
    Serial.println("---");
}