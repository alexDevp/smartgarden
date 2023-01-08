// DHT 11 (Temperature Humidity)
#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// WPSE303 (Soil Moisture) 
int PIN3 = 3;

// KY-018 (Ligth resistance)
int PIN5 = 5;

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {

  // Wait a 1 minute between measurements.
  delay(4000); //60000 when done

  // DHT11
  // Read Humidity as percentage
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();

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
}
