#include <DHT.h>

// === Pin Definitions ===
#define DHT_PIN       3
#define DHT_TYPE      DHT11
#define LM35_PIN      A0
#define LDR_PIN       A1
#define WATER_SENSOR  4
#define PUMP_RELAY    5

// === Sensor Objects ===
DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(WATER_SENSOR, INPUT);
  pinMode(PUMP_RELAY, OUTPUT);
  digitalWrite(PUMP_RELAY, LOW); // Start off
}

void loop() {
  // Read DHT11
  float tempDHT = dht.readTemperature();
  float humDHT = dht.readHumidity();

  // Read LM35
  int lm35Raw = analogRead(LM35_PIN);
  float lm35Temp = (lm35Raw * 5.0 / 1023.0) * 100;

  // Read LDR
  int lightVal = analogRead(LDR_PIN);

  // Water detection (HIGH = dry, LOW = wet)
  int waterState = digitalRead(WATER_SENSOR);
  String waterStatus = (waterState == LOW) ? "Dry" : "Wet";

  // Control relay (pump)
  digitalWrite(PUMP_RELAY, waterState); // Turn on pump if dry

  // Send formatted data over Serial
  Serial.print("T1:");
  Serial.print(tempDHT);
  Serial.print("|H:");
  Serial.print(humDHT);
  Serial.print("|T2:");
  Serial.print(lm35Temp);
  Serial.print("|L:");
  Serial.print(lightVal);
  Serial.print("|W:");
  Serial.println(waterStatus);

  delay(2000);
}
