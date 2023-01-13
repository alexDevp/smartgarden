// Wifi - Data Sent to API
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "time.h"
const char* ssid     = "Ethernet";
const char* password = "d7na2f2w";
const char* device_id = "DZRJE0991";
const char* mac_address = "57-FA-E4-86-76-C0";
const char* ip_address = "249.4.196.145";
StaticJsonDocument<500> doc;
unsigned long epochTime; 

// DHT 11 (Temperature Humidity)
#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


// WPSE303 (Soil Moisture) 
int PIN3 = 3;

// KY-018 (Ligth resistance)
int PIN5 = 5;

const char* ntpServer = "pool.ntp.org";
const char* serverName = "https://9lies56ebj.execute-api.eu-west-2.amazonaws.com/v1/";

void setup() {
  Serial.begin(9600);

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
        
        switch(WiFi.status()) {
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
        
        if(numberOfTries <= 0){
          Serial.print("[WiFi] Failed to connect to WiFi!");
          // Use disconnect function to force stop trying to connect
          WiFi.disconnect();
          return;
        } else {
          numberOfTries--;
        }
    }

  dht.begin();
}

void POSTData()
{
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      http.begin(serverName);
      http.addHeader("Content-Type", "text/plain");
      http.addHeader("x-api-key", "zuosZL4xqy3k8ss8ESbR731o2HH58n1v3xZjqVp3");
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
    return(0);
  }
  time(&now);
  return now;
}

void loop() {

  // Wait a 1 minute between measurements.
  delay(60000); //60000 when done

  // DHT11
  // Read Humidity as percentage
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();

  epochTime = getTime();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  } else{
    Serial.println(F("-----------------------------"));   
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%\nTemperature: "));
    Serial.print(t);
    Serial.print(F("°C\n"));
  }

  // WPSE
  // Read Moisture as percentage
  float m = analogRead(PIN3);
  if(isnan(m)){
    Serial.println(F("Failed to read from WPSE sensor!"));
  } else{
    Serial.print(F("Moisture: "));
    Serial.print(m/10);
    Serial.print(F("%\n"));
  }

  // Photoresistor
  // Read Resistance to ligth
  float r = analogRead(PIN5);
  if(isnan(r)){
    Serial.println(F("Failed to read from Photoresistor!"));
  } else{
    Serial.print(F("Ligth: "));
    Serial.print(r);
    Serial.print(F("%\n"));
  }

  doc["device_id"] = device_id;
  doc["mac_address"] = mac_address;
  doc["mac_address"] = mac_address;
  doc["ip_address"] = ip_address;
  doc["temperature"] = t;
  doc["humidity_level"] = h;
  doc["soil_moisture"] = m;
  doc["light_intensity"] = r;
  doc["happened_at"] = epochTime;

  Serial.println("Uploading data... "); 
  POSTData();
}
