#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Stepper.h>

char ssid[] = "ssid";
char pass[] = "password";
const float ROTATIONS_TO_UNROLL_SCREEN = 1.0;
const float ROTATIONS_TO_RAISE_SCREEN = 1.0;
const int UP = HIGH;
const int DOWN = LOW;
const int ROTATION_PULSES = 1600;
float currentElevation = 0;
float currentRotation = 0;
// belt drive motor
const int BELT_STEP_PIN = 14; 
const int BELT_DIR_PIN = 4; 
const int BELT_EN_PIN = 15;
// canvas lift motor
const int CANVAS_STEP_PIN = 12;
const int CANVAS_DIR_PIN = 5;
const int CANVAS_EN_PIN = 16;

const String CANVAS = "CANVAS";
const String BELT = "BELT";

ESP8266WebServer server(80);

void setup()
{
  Serial.begin(300);
  connectToWifi();

  server.on("/", handleRotate);
  server.on("/raiseScreen", handleRotate);
  server.on("/lowerScreen", handleRotate);
  server.on("/recalibrate", handleRotate);
  server.on("/stop", handleStop);
  server.begin();

  // Sets the two pins as Outputs
  pinMode(BELT_STEP_PIN,OUTPUT); 
  pinMode(BELT_DIR_PIN,OUTPUT);
  pinMode(CANVAS_STEP_PIN, OUTPUT);
  pinMode(CANVAS_DIR_PIN, OUTPUT);

  pinMode(BELT_EN_PIN,OUTPUT);
  digitalWrite(BELT_EN_PIN,LOW);
  pinMode(CANVAS_EN_PIN, OUTPUT);
  digitalWrite(CANVAS_EN_PIN, LOW);
}

void loop() {
  server.handleClient();
  delay(1000);

  Serial.println("looped");
}

void connectToWifi() {
  // connect to wifi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void handleStop() {
  // stop the motor
  Serial.println("stopping motor");

  for(int x = 0; x < 8000; x++) {
    digitalWrite(BELT_STEP_PIN,LOW); 
    delayMicroseconds(25); 
    digitalWrite(BELT_STEP_PIN,LOW); 
    delayMicroseconds(25); 
  }

  server.send(200, "text/plain", "Hello world");
}

void handleRotate() {
  float spins = 0;
  int _direction = 0;
  String component = "";
  for(int i = 0; i < server.args(); i++) {
    yield();
    Serial.print(server.argName(i));
    Serial.print(":");
    Serial.println(server.arg(i));
    if(server.argName(i) == "spins") {
      spins = server.arg(i).toFloat();
    } else if(server.argName(i) == "direction") {
      if(server.arg(i) == "Up" || server.arg(i) == "UP" || server.arg(i) == "up") {
        _direction = UP;
      } else if(server.arg(i) == "Down" || server.arg(i) == "DOWN" || server.arg(i) == "down"){
        _direction = DOWN;
      }
    } else if(server.argName(i) == "component") {
      if(server.arg(i) == "Roller" || server.arg(i) == "ROLLER" || server.arg(i) == "roller") {
        component = CANVAS;
      } else if(server.arg(i) == "Elevation" || server.arg(i) == "ELEVATION" || server.arg(i) == "elevation"){
        component = BELT;
      }
    }
  }
  Serial.println("rotations:" + String(spins));
  Serial.println("direction:" + String(_direction));
  Serial.println("component:" + component);

  rotate(spins, _direction, component);
  
  server.send(200, "text/plain", "Hello world");
}

void rotate(float rotations, int _direction, String component) {
  int numPulses = rotations * ROTATION_PULSES;
  int dirPin, stepPin;
  if(component == CANVAS) {
    dirPin = CANVAS_DIR_PIN;
    stepPin = CANVAS_STEP_PIN;
  } else {
    dirPin = BELT_DIR_PIN;
    stepPin = BELT_STEP_PIN;
  }
  digitalWrite(dirPin, _direction);

  Serial.println("Rotating " + component + " direction:" + String(_direction) + ", with " + String(numPulses) + " pulses");
  for(int x = 0; x < numPulses; x++) {
    yield(); // avoid watchdog timeout
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(175); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(175); 
  }
}
