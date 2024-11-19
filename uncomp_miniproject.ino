#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//=================================================================================================
//Ultrasonic Servo OLED
#define TRIG_PIN 4
#define ECHO_PIN 15

Servo servo1;
#define SERVO_PIN 2

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String username = "";
String current_time = "";

//=================================================================================================
// WiFi และ MQTT Configuration
const char* ssid = "WIFINAME";
const char* password = "PASSWORD";
const char* mqtt_broker = "BROKER";
const int mqtt_port = 1883;
const char* topic_username = "CENEXTGEN/oled/username";
const char* topic_time = "CENEXTGEN/oled/time";
const char* publish_topic = "CENEXTGEN/doorStatus";
const char* distance_topic = "CENEXTGEN/distance";

WiFiClient espClient;
PubSubClient client(espClient);

//=================================================================================================

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message: ");
  Serial.println(message);
  Serial.println("-----------------------");

  if (strcmp(topic, topic_username) == 0) {
    username = message;
  } 
  // your code for current_time
}

void reconnect() {
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());

    if (client.connect(client_id.c_str())) {
      Serial.println("Public MQTT broker connected");
      client.subscribe(topic_username);
      client.subscribe(topic_time);
    } else {
      Serial.print("Failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

float measureDistance() {
  //your code return ระยะทางที่วัดได้
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  
  reconnect();
 
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  servo1.attach(SERVO_PIN);
  
  if (!OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
  } else {
    Serial.println("OLED Start Work !!!");
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  unsigned long currentMillis = millis();

  float distance = measureDistance();

  //your code for sending mqtt and close-open the door (อย่าลืมกันการ spam ค่าที่มีการเปลี่ยนแปลงตลอดเวลา อาจทำให้ servo ทำงานรัว)

  OLED.clearDisplay();
  OLED.setTextColor(WHITE, BLACK);
  OLED.setCursor(0, 0);
  OLED.setTextSize(2);
  OLED.println("Welcome..");
  OLED.setCursor(0, 20);
  OLED.setTextSize(2);
  OLED.println(username);

  OLED.setCursor(0, 40);
  OLED.setTextSize(1);
  OLED.print("time : ");
  OLED.setTextColor(BLACK, WHITE); 
  OLED.println(current_time);

  OLED.setCursor(0, 50);
  OLED.setTextColor(WHITE, BLACK); 
  OLED.print("Distance: ");
  OLED.print(distance);
  OLED.println(" cm");
  OLED.display();
  client.loop();
}
