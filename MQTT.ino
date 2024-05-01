#include <WiFi.h>
#include <PubSubClient.h>

#include <Arduino.h>
#include "MH-Z14A.h"

char ssid[] = "789988a";
char pass[] = "23699322";
char mqtt_server[] = "broker.hivemq.com";
char user_name[] = "789988a";
char user_password[] = "23699322";
char client_Id[] = "haha4wola";
char Pub_topic[] = "MH-Z14A/co2";

unsigned long prevMillis = 0;
unsigned long interval = 10000;
String msgStr = "";

int status = WL_IDLE_STATUS;
WiFiClient mtclient;
PubSubClient client(mtclient);

MHZ14A sensor(Serial1, Serial); 

void setup()
{
   Serial.begin(115200);
   setup_wifi();
   client.setServer(mqtt_server, 1883);

   sensor.begin(4000);
   sensor.setDebug(true);
}

void loop()
{
  delay(10000);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - prevMillis > interval) {
    prevMillis = millis();
    
    co2_publish();
  }
}

void co2_publish(){
  msgStr = msgStr + "CO2 concentration: " + String(sensor.readConcentrationPPM(0x01)) + " ppm";
  byte arrSize = msgStr.length() + 1;
  char msg[arrSize];
  msgStr.toCharArray(msg, arrSize);
  client.publish(Pub_topic, msg);
  msgStr = ""; 
}

void setup_wifi() {  
   Serial.print("Attempting to connect to SSID: ");
   Serial.println(ssid);
   WiFi.begin(ssid, pass);
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   Serial.println("Connected to wifi");
   printWifiStatus();
}

void printWifiStatus() {
   Serial.print("SSID: ");
   Serial.println(WiFi.SSID());

   Serial.print("IP Address: ");
   Serial.println(WiFi.localIP());

   Serial.print("signal strength (RSSI):");
   Serial.print(WiFi.RSSI());
   Serial.println(" dBm");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(client_Id, user_name, user_password)) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
