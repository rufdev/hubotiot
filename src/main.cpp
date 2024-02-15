#include <Arduino.h>
// #include <WiFi.h>
// #include <WiFiClient.h>
// #include <WebServer.h>
// #include <SocketIoClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIoClient.h>
#include <Adafruit_MLX90614.h>
#include <DHT.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
SocketIOclient socketIO;

// const char *ssid = "DICT_Bukmobile2";
// const char *password = "d1ctbuk1dn0n";
// const char *HOST = "192.168.254.122";

// const char *ssid = "RUF.DEV";
// const char *password = "Alabagos@z7a18q";
// const char *HOST = "192.168.1.112";

// const char *ssid = "PICTD WAP";
// const char *password = "pictd*123";
// const char *HOST = "10.50.27.155";

// const char *ssid = "B593_BB6AA";
// const char *password = "5LQ758N0BM0";
// const char *HOST = "192.168.254.100";

const char *ssid = "RUF.DEV";
const char *password = "Alabagos@z7a18q";
const char *HOST = "192.168.25.203";

// const char *ssid = "DayView Tourist Home B2";
// const char *password = "DAYVIEWB2517";
// const char *HOST = "192.168.2.140";

// const char *ssid = "VDT_Theater2.4G";
// const char *password = "";
// const char *HOST = "133.222.116.71";

// IPAddress local_IP(192, 168, 2, 200);
// IPAddress gateway(192, 168, 2, 1);
// IPAddress subnet(255, 255, 255, 0);
// IPAddress primaryDNS(8, 8, 8, 8);   // optional
// IPAddress secondaryDNS(8, 8, 4, 4); // optional

WebServer server(80);
// SocketIoClient webSocket;
// Define the pins for the LEDs

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define DHT_SENSOR_PIN 33 // ESP32 pin GPIO21 connected to DHT11 sensor
#define DHT_SENSOR_TYPE DHT11

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

const int TapPin = 32;

const int led1Pin = 15;
const int led2Pin = 2;
const int led3Pin = 4;
const int led4Pin = 5;
const int buzzerPin = 32;

const int photoPin = 34;
int photoresistorvalue = 0;
double ambianttemp = 0;
double objecttemp = 0;

const int trigPin = 19;
const int echoPin = 18;

#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

void handlecors()
{
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, Content-Type");
  server.sendHeader("Access-Control-Max-Age", "86400");
  server.send(200);
}

void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case sIOtype_DISCONNECT:
    Serial.printf("[IOc] Disconnected!\n");
    break;
  case sIOtype_CONNECT:
    Serial.printf("[IOc] Connected to url: %s\n", payload);

    // join default namespace (no auto join in Socket.IO V3)
    socketIO.send(sIOtype_CONNECT, "/");
    break;
  case sIOtype_EVENT:
  {
    char *sptr = NULL;
    int id = strtol((char *)payload, &sptr, 10);
    Serial.printf("[IOc] get event: %s id: %d\n", payload, id);
    if (id)
    {
      payload = (uint8_t *)sptr;
    }
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    String eventName = doc[0];
    Serial.printf("[IOc] event name: %s\n", eventName.c_str());
    if (eventName == "bedroomstate")
    {
      Serial.println("bedroomstate");
      int state = doc[1];
      Serial.printf("state: %d\n", state);
      if (state == 1)
      {
        digitalWrite(led1Pin, HIGH);
      }
      else
      {
        digitalWrite(led1Pin, LOW);
      }
    }
    else if (eventName == "livingroomstate")
    {
      Serial.println("livingroomstate");
      int state = doc[1];
      Serial.printf("state: %d\n", state);
      if (state == 1)
      {
        digitalWrite(led2Pin, HIGH);
      }
      else
      {
        digitalWrite(led2Pin, LOW);
      }
    }
    else if (eventName == "kitchenstate")
    {
      Serial.println("kitchenstate");
      int state = doc[1];
      Serial.printf("state: %d\n", state);
      if (state == 1)
      {
        digitalWrite(led3Pin, HIGH);
      }
      else
      {
        digitalWrite(led3Pin, LOW);
      }
    }
    else if (eventName == "bathroomstate")
    {
      Serial.println("bathroomstate");
      int state = doc[1];
      Serial.printf("state: %d\n", state);
      if (state == 1)
      {
        digitalWrite(led4Pin, HIGH);
      }
      else
      {
        digitalWrite(led4Pin, LOW);
      }
    }
    // // Message Includes a ID for a ACK (callback)
    // if (id)
    // {
    //   // creat JSON message for Socket.IO (ack)
    //   DynamicJsonDocument docOut(1024);
    //   JsonArray array = docOut.to<JsonArray>();

    //   // add payload (parameters) for the ack (callback function)
    //   JsonObject param1 = array.createNestedObject();
    //   param1["now"] = millis();

    //   // JSON to String (serializion)
    //   String output;
    //   output += id;
    //   serializeJson(docOut, output);

    //   // Send event
    //   socketIO.send(sIOtype_ACK, output);
    // }
  }
  break;
  case sIOtype_ACK:
    Serial.printf("[IOc] get ack: %u\n", length);
    break;
  case sIOtype_ERROR:
    Serial.printf("[IOc] get error: %u\n", length);
    break;
  case sIOtype_BINARY_EVENT:
    Serial.printf("[IOc] get binary: %u\n", length);
    break;
  case sIOtype_BINARY_ACK:
    Serial.printf("[IOc] get binary ack: %u\n", length);
    break;
  }
}

void setupWiFi()
{
  // WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);

  WiFiMulti.addAP(ssid, password);

  Serial.println("Connecting Wifi...");
  if (WiFiMulti.run() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void setup()
{
  // Serial.begin(115200);
  // Serial.begin(921600);
  Serial.begin(115200);

  // Serial.setDebugOutput(true);
  // Serial.setDebugOutput(true);

  // Serial.println();
  // Serial.println();
  // Serial.println();

  setupWiFi();

  while (!Serial)
    ;

  if (!mlx.begin())
  {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1)
      ;
  };

  dht_sensor.begin();

  // Setup the pins for the LEDs
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
  pinMode(led4Pin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // Setup the input pins
  pinMode(photoPin, INPUT);

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input

  // Handle CORS requests
  server.onNotFound([]()
                    {
    if (server.method() == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Origin, Content-Type");
      server.sendHeader("Access-Control-Max-Age", "86400");
      server.send(200);
    } else {
      server.send(404);
    } });

  // Turn on LED 1
  server.on("/led1/on", HTTP_GET, []()
            {
    digitalWrite(led1Pin, HIGH);
    handlecors(); });

  // Turn off LED 1
  server.on("/led1/off", HTTP_GET, []()
            {
    digitalWrite(led1Pin, LOW);
    handlecors(); });

  // Turn on LED 2
  server.on("/led2/on", HTTP_GET, []()
            {
    digitalWrite(led2Pin, HIGH);
    handlecors(); });

  // Turn off LED 2
  server.on("/led2/off", HTTP_GET, []()
            {
    digitalWrite(led2Pin, LOW);
    handlecors(); });

  // Turn on LED 3
  server.on("/led3/on", HTTP_GET, []()
            {
    digitalWrite(led3Pin, HIGH);
    handlecors(); });

  // Turn off LED 3
  server.on("/led3/off", HTTP_GET, []()
            {
    digitalWrite(led3Pin, LOW);
    handlecors(); });

  // Turn on LED 4
  server.on("/led4/on", HTTP_GET, []()
            {
    digitalWrite(led4Pin, HIGH);
    handlecors(); });

  // Turn off LED 4
  server.on("/led4/off", HTTP_GET, []()
            {
    digitalWrite(led4Pin, LOW);
    handlecors(); });

  // Turn on LED 4
  server.on("/buzz/on", HTTP_GET, []()
            {
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
    delay(100);
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
    delay(100);
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
    handlecors(); });

  // server address, port and URL
  socketIO.begin(HOST, 3000, "/socket.io/?EIO=4");
  // socketIO.configureEIOping(5000); // set heartbeat timeout
  // event handler
  socketIO.onEvent(socketIOEvent);
  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  server.handleClient();

  socketIO.loop();

  if (socketIO.isConnected())
  {
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);

    // Calculate the distance
    distanceCm = duration * SOUND_SPEED / 2;

    // Convert to inches
    distanceInch = distanceCm * CM_TO_INCH;

    photoresistorvalue = analogRead(photoPin);
    // Serial.print("Photoresistor value: ");
    // Serial.println(photoresistorvalue);
    ambianttemp = mlx.readAmbientTempC();
    // Serial.print("Ambiant temperature: ");
    // Serial.println(ambianttemp);
    objecttemp = mlx.readObjectTempC();
    // Serial.print("Object temperature: ");
    // Serial.println(objecttemp);

    bool led1PinStatus = digitalRead(led1Pin);
    bool led2PinStatus = digitalRead(led2Pin);
    bool led3PinStatus = digitalRead(led3Pin);
    bool led4PinStatus = digitalRead(led4Pin);

    // read humidity
    float humi = dht_sensor.readHumidity();
    // read temperature in Celsius
    float tempC = dht_sensor.readTemperature();
    // read temperature in Fahrenheit
    float tempF = dht_sensor.readTemperature(true);

    // check whether the reading is successful or not
    if (isnan(tempC) || isnan(tempF) || isnan(humi))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
      Serial.print("Humidity: ");
      Serial.print(humi);
      Serial.print("%");

      Serial.print("  |  ");

      Serial.print("Temperature: ");
      Serial.print(tempC);
      Serial.print("°C  ~  ");
      Serial.print(tempF);
      Serial.println("°F");
    }

    delay(1000);

    // creat JSON message for Socket.IO (event)
    DynamicJsonDocument doc(1024);
    JsonArray array = doc.to<JsonArray>();

    // add evnet name
    // Hint: socket.on('event_name', ....
    array.add("iotdata");

    // add payload (parameters) for the event
    JsonObject param1 = array.createNestedObject();
    param1["pr"] = (uint32_t)photoresistorvalue;
    param1["at"] = ambianttemp;
    param1["ot"] = objecttemp;
    param1["uss"] = distanceCm;
    param1["humi"] = humi;
    param1["tempC"] = tempC;
    param1["tempF"] = tempF;
    param1["led1"] = led1PinStatus;
    param1["led2"] = led2PinStatus;
    param1["led3"] = led3PinStatus;
    param1["led4"] = led4PinStatus;

    // JSON to String (serializion)
    String output;
    serializeJson(doc, output);

    // Send event
    socketIO.sendEVENT(output);

    // Print JSON for debugging
    // Serial.println(output);
  }
}