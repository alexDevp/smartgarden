// Wifi - Data Sent to API
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "time.h"

const char* ssid = "<WIFI NAME>";
const char* password = "<WIFI PASSWORD>";

const char* device_id = "DZRJE0991"; // Change this

const char* apiKey = "<API KEY>";
const char* ntpServer = "pool.ntp.org";
const char* apiBaseUrl = "https://9lies56ebj.execute-api.eu-west-2.amazonaws.com/v1/";

StaticJsonDocument<500> doc;

// Definition of sleep mode properties
//                           seconds
//                              v
const uint32_t SLEEP_DURATION = 60 * 1000000; // µs

// Definition of time control parameters
const uint16_t LOOP_FREQUENCY = 25;                    // Hz
const uint16_t WAIT_PERIOD    = 1000 / LOOP_FREQUENCY; // ms

struct Timer {
    uint32_t laptime;
    uint32_t ticks;
};

Timer timer;

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

  // initializes the timer
  timer = { millis(), 0 };

  // init sensors
  Wire.begin();
  dht.begin();
  lightMeter.begin();
}

void sendData() {
  Serial.println("Uploading data... ");

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

// Time control of the main loop
void waitForNextCycle() {
    uint32_t now;
    do { 
      now = millis(); 
    } while (now - timer.laptime < WAIT_PERIOD);
    timer.laptime = now;
    timer.ticks++;
}

void collectData() {
  unsigned long epochTime = getTime();

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

  doc["device_id"] = device_id;
  doc["mac_address"] = WiFi.macAddress();
  doc["ip_address"] = WiFi.localIP();
  doc["temperature"] = temperature;
  doc["humidity_level"] = humidityLevel;
  doc["soil_moisture"] = soilMoisturePercent;
  doc["light_intensity"] = lux;
  doc["happened_at"] = epochTime;
}

void work() {
  connectToWiFi();
  collectData();
  sendData();
}

void loop() {
  Serial.println("Waking up...");

  uint32_t startFreeHeap = ESP.getFreeHeap();

  uint32_t startTime = millis();

  work();

  uint32_t memoryUsed = startFreeHeap - ESP.getFreeHeap();
  Serial.print(F("Memory used: "));
  Serial.print(memoryUsed);
  Serial.print(F(" bytes\n"));  

  uint32_t executionTime = millis() - startTime;
  Serial.print(F("Execution Time: "));
  Serial.print(executionTime);
  Serial.print(F(" ms\n"));

  hibernate();

  waitForNextCycle();
}

void deepSleep() {
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
  esp_deep_sleep_start();
}

void hibernate() {
  Serial.println("Hibernation Start...");

  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
    
  deepSleep();
}
