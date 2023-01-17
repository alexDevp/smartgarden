// Wifi - Data Sent to API
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "time.h"

const char* ssid = "<WIFI NAME>";
const char* password = "<WIFI PASSWORD>";

const char* device_id = "DZRJE0991"; // Change this

const char* apiKey = "zuosZL4xqy3k8ss8ESbR731o2HH58n1v3xZjqVp3";
const char* ntpServer = "pool.ntp.org";
const char* apiBaseUrl = "https://9lies56ebj.execute-api.eu-west-2.amazonaws.com/v1/";

StaticJsonDocument<500> doc;
unsigned long epochTime;

// DHT11 (Temperature Humidity)
// https://esp32io.com/tutorials/esp32-temperature-humidity-sensor
#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// BH1750 - Light intensity sensor
// https://randomnerdtutorials.com/esp32-bh1750-ambient-light-sensor/
#include <BH1750.h>
BH1750 lightMeter;

// Soil Moisture
int PIN_SOIL_MOISTURE = 33;

// Grove - Sunlight Sensor (UV Index)
// https://wiki.seeedstudio.com/Grove-Sunlight_Sensor/
// #include "Si115X.h"
// Si115X sunlightSensor;

void connectToWiFi() {
  // Connect to Wifi
  Serial.println();
  Serial.print("[WiFi] Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int tryDelay = 500;
  int numberOfTries = 20;

  configTime(0, 0, ntpServer);

  // Wait for the WiFi event
  while (true) {
    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL:
        Serial.println("[WiFi] SSID not found");
        break;
      case WL_CONNECT_FAILED:
        Serial.print("[WiFi] Failed - WiFi not connected! Reason: ");
        return;
        break;
      case WL_CONNECTION_LOST:
        Serial.println("[WiFi] Connection was lost");
        break;
      case WL_SCAN_COMPLETED:
        Serial.println("[WiFi] Scan is completed");
        break;
      case WL_DISCONNECTED:
        Serial.println("[WiFi] WiFi is disconnected");
        break;
      case WL_CONNECTED:
        Serial.println("[WiFi] WiFi is connected!");
        Serial.print("[WiFi] IP address: ");
        Serial.println(WiFi.localIP());
        return;
        break;
      default:
        Serial.print("[WiFi] WiFi Status: ");
        Serial.println(WiFi.status());
        break;
    }
    delay(tryDelay);

    if (numberOfTries <= 0) {
      Serial.print("[WiFi] Failed to connect to WiFi!");
      // Use disconnect function to force stop trying to connect
      WiFi.disconnect();
      return;
    } else {
      numberOfTries--;
    }
  }
}

void setup() {
  Serial.begin(9600);

  connectToWiFi();

  // init sensors
  Wire.begin();
  dht.begin();
  lightMeter.begin();
  // sunlightSensor.Begin();
}

void sendData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(apiBaseUrl);
    http.addHeader("Content-Type", "text/plain");
    http.addHeader("x-api-key", apiKey);
    String json;
    serializeJson(doc, json);

    Serial.println(json);
    int httpResponseCode = http.POST(json);
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      Serial.println("Data uploaded.");
    } else {
      Serial.println("ERROR: Couldn't upload data.");
    }
  }
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return (0);
  }
  time(&now);
  return now;
}

void loop() {
  // Wait a 1 minute between measurements.
  delay(60000);  //60000 when done

  epochTime = getTime();

  // DHT11
  // Read Humidity as percentage
  float humidityLevel = dht.readHumidity();
  // Read temperature as Celsius
  float temperature = dht.readTemperature();

  if (isnan(humidityLevel) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  } else {
    Serial.println(F("-----------------------------"));
    Serial.print(F("Humidity: "));
    Serial.print(humidityLevel);
    Serial.print(F("%\nTemperature: "));
    Serial.print(temperature);
    Serial.print(F("°C\n"));
  }

  // Soil Moisture
  // Read Moisture as percentage
  int soilMoisture = analogRead(PIN_SOIL_MOISTURE);
  float soilMoisturePercent = 100.00 - ((soilMoisture / 4095.00) * 100.00);
  if (isnan(soilMoisture)) {
    Serial.println(F("Failed to read from WPSE sensor!"));
  } else {
    Serial.print(F("Soil Moisture: "));
    Serial.print(soilMoisturePercent);
    Serial.print(F("%\n"));
  }

  // Light intensity
  // https://randomnerdtutorials.com/esp32-bh1750-ambient-light-sensor/
  float lux = lightMeter.readLightLevel();
  if (isnan(lux)) {
    Serial.println(F("Failed to read from Light sensor!"));
  } else {
    Serial.print(F("Light Intensity: "));
    Serial.print(lux);
    Serial.print(F(" lx\n"));
  }

  // UV index
  // https://wiki.seeedstudio.com/Grove-Sunlight_Sensor/
  // float uvIndex = sunlightSensor.ReadHalfWord_UV();
  // if (isnan(uvIndex)) {
  //   Serial.println(F("Failed to read from UV sensor!"));
  // } else {
  //   Serial.print(F("UV Index: "));
  //   Serial.print(uvIndex);
  //   Serial.print(F("\n"));
  // }

  doc["device_id"] = device_id;
  doc["mac_address"] = WiFi.macAddress();
  doc["ip_address"] = WiFi.localIP();
  doc["temperature"] = temperature;
  doc["humidity_level"] = humidityLevel;
  doc["soil_moisture"] = soilMoisturePercent;
  doc["light_intensity"] = lux;
  doc["uv_index"] = 0.0; // WIP
  doc["happened_at"] = epochTime;

  Serial.println("Uploading data... ");
  sendData();
}
