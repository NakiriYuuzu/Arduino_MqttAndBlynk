#include "DHT.h"
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <MQTT.h>

#define DHTPIN D8
#define DHTTYPE DHT11

#define I2C_ADDRESS 0x3C
#define RST_PIN -1

int whiPin = 15;
int redPin = 14;
int grnPin = 12;
int bluPin = 13;

#define BLYNK_TEMPLATE_ID "TMPLpSBmr3mE"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "PhgHND-46XRy68YUkMHVmPCKQNjHjDOM"
#define BLYNK_AUTH_TOKEN_LOCAL "Qbc-UayY3ynrE3TCqS0Fdh40Sez18td9"

char ssid[] = "Xiaomi_Yuuzu";
char pass[] = "NakiriYuuzu";

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

DHT dht(DHTPIN, DHTTYPE);
SSD1306AsciiWire oled;
BlynkTimer timer;

void ledRGB(float t)
{  
  if(t >= 28)
  {
    digitalWrite(redPin, HIGH); // Red
    digitalWrite(grnPin, HIGH);
    delay(100);
    digitalWrite(bluPin, LOW);
  }

  else if((t < 28) && (t >= 23)) 
  {
    digitalWrite(redPin, HIGH); // yellow
    digitalWrite(grnPin, HIGH);
    delay(100);
    digitalWrite(bluPin, LOW);
  }
  else if((t < 23) && (t > 18)) 
  {
    digitalWrite(grnPin, HIGH); // green
    delay(100);
    digitalWrite(redPin, LOW);
    digitalWrite(bluPin, LOW);
  }
  else if((t <= 18) && (t > 13)) 
  {
    digitalWrite(grnPin, HIGH); // aqua
    digitalWrite(bluPin, HIGH);
    delay(100);
    digitalWrite(redPin, LOW);
  }
  else
  {
    digitalWrite(bluPin, HIGH); // blue
    delay(100);
    digitalWrite(grnPin, LOW);
    digitalWrite(redPin, LOW);
  } 
}

float tempT = 0;
float tempH = 0;

void showOled(float t, float h) {
  if (tempT == t && tempH == h) {
    return;
  } 

  tempT = t;
  tempH = h;

  String s = "----------";
  oled.clear();
  oled.println("House Temp");
  oled.println(s);
  oled.printf("Temp : %.0f", t);oled.print("C");
  oled.println();
  oled.printf("Humid: %.0f", h);oled.print("%");
}

void sendSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Blynk.virtualWrite(V5, t);
  Blynk.virtualWrite(V6, h);
}

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("Yuuzu", "", "")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("Blynk");
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if (payload == "true") {
    digitalWrite(whiPin, HIGH);
  }

  if (payload == "false") {
    digitalWrite(whiPin, LOW);
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin();
  Wire.setClock(400000L);

  WiFi.begin(ssid, pass);

  client.begin("125.228.31.174", 7111, net);
  client.onMessage(messageReceived);
  connect();

  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN_LOCAL, ssid, pass, IPAddress(125, 228, 31, 174), 7416);
  // Blynk.config(BLYNK_AUTH_TOKEN);
  // Blynk.connect();
  timer.setInterval(1000L, sendSensor);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
  oled.set2X();

  dht.begin();

  pinMode(redPin, OUTPUT);
  pinMode(grnPin, OUTPUT);
  pinMode(bluPin, OUTPUT);
  pinMode(whiPin, OUTPUT);
  digitalWrite(whiPin, LOW);
}

void loop() {
  client.loop();

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT...");
    return;
  }

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    String msg = "" + String(t) + "|" + String(h);
    client.publish("Blynk", msg.c_str());
  }

  ledRGB(t);
  showOled(t, h);
  Blynk.run();
  timer.run();
}
