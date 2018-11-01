#include <WiFi.h>

#define SENSOR_PIN_1 33
#define SENSOR_PIN_2 32
#define SENSOR_ACTIVE_VALUE 3000
#define AFTER_COUNT_DELAY 1000
#define MAX_CHECK_DELAY 5000

const char* ssid = ""; //Your ssid
const char* password = ""; //Your password

WiFiServer server(80);

String header;

int visitors = 0;

boolean sensor1Active = false;
boolean sensor2Active = false;

unsigned long previousMillis = 0; 

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type: application/json");
            client.println("Connection: close");
            client.println();
            client.print("{\"visitors\": ");
            client.print(visitors);
            client.println(" }");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  if (!sensor2Active && !sensor1Active) {
    sensor1Active = checkSensor(SENSOR_PIN_1);
    sensor2Active = checkSensor(SENSOR_PIN_2);
    if (sensor1Active || sensor2Active) {
      previousMillis = millis();
    }
  } else if (sensor1Active && !sensor2Active) {
    sensor2Active = checkSensor(SENSOR_PIN_2);
    if (sensor2Active) {
      visitors++;
      delay(AFTER_COUNT_DELAY);
      reset();
    } else if (millis() - previousMillis >= MAX_CHECK_DELAY) {
      reset();
    }
  } else if (sensor2Active && !sensor1Active) {
    sensor1Active = checkSensor(SENSOR_PIN_1);
    if (sensor1Active) {
      visitors--;
      if (visitors < 0) {
        visitors = 0;
      }
      delay(AFTER_COUNT_DELAY);
      reset();
    } else if (millis() - previousMillis >= MAX_CHECK_DELAY) {
      reset();
    }
  } else if(sensor1Active && sensor2Active) {
    delay(AFTER_COUNT_DELAY);
    reset();
  }
}

boolean checkSensor(int pin) {
  return analogRead(pin) < SENSOR_ACTIVE_VALUE;
}

void reset() {
  sensor1Active = false;
  sensor2Active = false;
}