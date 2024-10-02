// https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/index.html


#include <WiFi.h>
#include "PubSubClient.h"
#include "arduino_secrets.h"


long lastMsg = 0;
String stMac;
char mac[50];
char clientId[50];
int val = 0;
int result = 0;

WiFiClient espClient;
PubSubClient client(espClient);

#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

// https://docs.espressif.com/projects/esp-dev-kits/en/latest/_images/esp32_devkitC_v4_pinlayout.png
int sensorpin = A0;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  //set the resolution to 12 bits (0-4095)
  analogReadResolution(12);

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SECRET_SSID);

  wifiConnect();

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
  stMac = WiFi.macAddress();
  stMac.replace(":", "_");
  Serial.println(stMac);
  
  client.setServer(SECRET_MQTT_SERVER, SECRET_MQTT_PORT);
  client.setCallback(callback);
}


void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}



void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    long r = random(1000);
    sprintf(clientId, "clientId-%ld", r);
    if (client.connect(clientId,SECRET_MQTT_USER, SECRET_MQTT_API_KEY)) {
      Serial.println(" connected");
      // client.subscribe("topicName/led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String stMessage;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    stMessage += (char)message[i];
  }
  Serial.println();
}

void loop() {
 
  delay(10);

  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();

   

  long now = millis();
  if (now - lastMsg > 60000) {
    lastMsg = now;
    val = analogRead(sensorpin);  // read the input pin
    
    //snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", val);
    snprintf (msg, MSG_BUFFER_SIZE, "%ld", val);
    Serial.print("Publish message: ");
    Serial.println(msg);
    // https://io.adafruit.com/avitaletti/feeds/turgidity
    result = client.publish("avitaletti/feeds/turgidity", msg);
    Serial.print("MQTT Publish Result: ");
    Serial.println(result);
  }




}
