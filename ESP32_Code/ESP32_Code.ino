#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <TinyGPSPlus.h>

#define EN_A_PIN 2   // Example pin connected to ENA
#define EN_B_PIN 4   // Example pin connected to ENB

// Wi-Fi credentials
const char* ssid = "SSID";
const char* password = "Password";

// Servo pins
const int servoPin1 = 26; // For Left/Right (Pan)
const int servoPin2 = 27; // For Up/Down (Tilt)
const int LSR_PIN = 14;   // Laser
Servo servo1; 
Servo servo2;

// Current servo angles
int angle1 = 90;
int angle2 = 90;

const int metalDetectorPin = 12;  // Pin connected to metal detector buzzer output

// PIR sensor
const int pirPin = 13;  // PIR sensor connected to GPIO13

// GPS
HardwareSerial mySerial(1);  // Use Serial1 (pins 16 RX, 17 TX)
TinyGPSPlus gps; 

// GPS data
float latitude = 0.0, longitude = 0.0, altitude = 0.0, speed = 0.0;
unsigned long timestamp = 0;

// Web server
WebServer server(80);

// ==== Servo control handler ====
void handleServoControl() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    Serial.println("Received servo command: " + cmd);
    
    // Always reattach servos before moving
    servo1.attach(servoPin1);
    servo2.attach(servoPin2);

    if (cmd == "left") {
      angle1 = max(0, angle1 - 4); // move left
    }
    else if (cmd == "right") {
      angle1 = min(180, angle1 + 4); // move right
    } 
    else if (cmd == "up") {
      angle2 = max(0, angle2 - 4); // move up
    } 
    else if (cmd == "down") {
      angle2 = min(180, angle2 + 4); // move down
    } 
    else if (cmd == "fire") {
      digitalWrite(LSR_PIN, HIGH);
      delay(500);
      digitalWrite(LSR_PIN, LOW);
    } 

    servo1.write(angle1);
    servo2.write(angle2);
  }

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Servo command received");
}

// Handle root endpoint
void handleRoot() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "ESP32 Web Server: PIR + GPS");
}

// Metal detector
void handleMetalDetector() {
  int metalDetected = digitalRead(metalDetectorPin);
  String message = metalDetected == HIGH ? "Metal Detected" : "No Metal";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

// Handle PIR motion check
void handlePIR() {
  int motion = digitalRead(pirPin);
  String message = motion == HIGH ? "Motion detected" : "No motion";
  Serial.println(message);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

// Handle speed control (0–255)
void handleSpeed() {
  if (server.hasArg("value")) {
    int motorSpeed = constrain(server.arg("value").toInt(), 0, 255);
    Serial.print("Motor speed: ");
    Serial.println(motorSpeed);
    
    // Convert 0-255 to 0-1023 for ESP32 PWM
    int pwmValue = map(motorSpeed, 0, 255, 0, 1023);
    analogWrite(EN_A_PIN, pwmValue);
    analogWrite(EN_B_PIN, pwmValue);
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Speed set");
  } else {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "text/plain", "Missing value");
  }
}

// Handle GPS data JSON
void handleGPSData() {
  String json = "{\"latitude\":" + String(latitude, 6) +
                ",\"longitude\":" + String(longitude, 6) +
                ",\"altitude\":" + String(altitude, 2) +
                ",\"speed\":" + String(speed, 2) +
                ",\"time\":" + String(timestamp) + "}";
  Serial.println(json);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 16, 17);  // GPS TX -> GPIO16, RX -> GPIO17
  pinMode(pirPin, INPUT);
  pinMode(metalDetectorPin, INPUT);
  pinMode(EN_A_PIN, OUTPUT);
  pinMode(EN_B_PIN, OUTPUT);
  pinMode(LSR_PIN, OUTPUT);
  digitalWrite(LSR_PIN, LOW);

  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  servo1.write(angle1);
  servo2.write(angle2);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected. IP address: ");
  Serial.println(WiFi.localIP());

  // Define endpoints
  server.on("/", handleRoot);
  server.on("/pir", handlePIR);
  server.on("/gpsdata", handleGPSData);
  server.on("/metal", handleMetalDetector);
  server.on("/servo", handleServoControl);
  server.on("/speed", handleSpeed);   
  server.begin();

  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();

  // Read and update GPS data
  while (mySerial.available() > 0) {
    gps.encode(mySerial.read());
    if (gps.location.isUpdated()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      altitude = gps.altitude.meters();
      speed = gps.speed.kmph();
      timestamp = gps.date.year() * 10000 + gps.date.month() * 100 + gps.date.day();

      Serial.printf("Lat: %.6f, Lng: %.6f, Alt: %.2f m, Speed: %.2f km/h, Date: %lu\n",
                    latitude, longitude, altitude, speed, timestamp);
    }
  }
}

