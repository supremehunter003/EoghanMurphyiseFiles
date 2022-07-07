
#include "DHT.h"
#include <AccelStepper.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>

#define DHTPIN 14
#define DHTTYPE DHT11
#define motorPin1  5
#define motorPin2  4
#define motorPin3  0
#define motorPin4  2

const char *ssid     = "WIFI-NAME";
const char *password = "WIFI-PASSWORD";
const long utcOffsetInSeconds = 3600;
const char* mqtt_server = "MQQT-SERVER-IP";
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
bool night_time = false;    // this is daytime
bool blinds_open = true;         // true is up false is down
bool temp_hot = true;       // true is hot false is cold
int stepsPerRev = 2048;
int down = stepsPerRev;
int up = 0;
int move_To = 0;
int time_mins = 0;
float stat = 0;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
// Set stepper mode
AccelStepper stepper(AccelStepper::HALF4WIRE, motorPin1, motorPin3, motorPin2, motorPin4);
// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("room/lamp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println("DHTxx test!");
  dht.begin();
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  timeClient.begin();
  stepper.setMaxSpeed(1000.0);
  stepper.setAcceleration(400.0);
  stepper.setSpeed(1000);
  stepper.currentPosition();

    setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

   if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");
  // Wait a few seconds between measurements
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);
  float hi = dht.computeHeatIndex(f, h);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");

  timeClient.update();
  Serial.print(daysOfTheWeek[timeClient.getDay()]);
  Serial.print(", ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());

  if (t >= 30) {   //t
    temp_hot = true;
  }
  else {
    temp_hot = false;
  }
  Serial.println(temp_hot);
  
  time_mins = timeClient.getHours();
  Serial.println(time_mins);

  if ((time_mins <= 20) and (time_mins >= 7)) {        // for testing swapped get hours with get minuts
    night_time = true;
  }
  else
  {
    night_time = false;
  }

  Serial.println(night_time);

  if ((night_time == false) and (temp_hot == false)) {     // open blinds
    blinds_open = true;
    move_To = up;
    Serial.println("move_To = up");
  }
  else if (night_time == true) {
    blinds_open = false;      //close blinds
    move_To = down;
     Serial.println("move_To = down");
  }
  else if ((night_time == false) and (temp_hot == true))
  {
    blinds_open = false;
    move_To = down;
      Serial.println("move_To = down");
  }
  else {
    Serial.println("move_To = invalid");
  }
  Serial.println(move_To);


  if (!stepper.isRunning() and stepper.distanceToGo() == 0)
  {
    stepper.moveTo(move_To);
  }
  stepper.run();

   if (blinds_open == true)
  {
  stat = 10;
  }
  else
  {
    stat = 0;
  }
  uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_STATUS, 1, true, String(stat).c_str());

  
}
