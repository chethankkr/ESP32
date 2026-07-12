**IoT-Based Smart Multi-Application Surveillance Robot**
Overview

The IoT-Based Smart Multi-Application Surveillance Robot is a Wi-Fi controlled robotic surveillance system developed using ESP32, ESP32-CAM, and a web-based control dashboard. The robot provides real-time video streaming, GPS tracking, motion detection, metal detection, camera pan-tilt control, speed control, and image capture through an interactive browser interface.

The project demonstrates embedded systems, IoT communication, web technologies, and real-time hardware integration.

**Features**
📹 Live video streaming using ESP32-CAM
📸 Capture and download images remotely
💡 Remote flash LED control
🤖 Robot movement control (Forward, Backward, Left, Right, Stop)
🎮 Motor speed control using PWM
🎥 Pan-Tilt camera control using dual servo motors
📍 Real-time GPS location tracking
🗺️ Live GPS visualization using OpenStreetMap (Leaflet)
🚨 PIR motion detection
🔩 Metal detection monitoring
🔫 Laser module control
🌐 Browser-based responsive control dashboard
⚡ FreeRTOS task-based video streaming
📡 Wi-Fi based HTTP communication between browser and ESP32 devices

**Technologies Used**
**Hardware**
ESP32 Development Board
ESP32-CAM (AI Thinker)
OV2640 Camera Module
GPS Module
PIR Motion Sensor
Metal Detector Sensor
Servo Motors (Pan & Tilt)
Laser Module
DC Motors
Motor Driver Module

**Software**
Arduino IDE
Embedded C++
HTML5
CSS3
JavaScript (ES6)
Leaflet.js
OpenStreetMap
HTTP REST APIs
FreeRTOS

**REST API Endpoints**
ESP32-CAM
Endpoint	Description
/stream	Live video streaming
/capture	Capture image
/control	Robot movement and flash control

ESP32 Controller
Endpoint	Description
/servo	Camera pan-tilt control
/gpsdata	GPS location data
/pir	Motion detection status
/metal	Metal detection status
/speed	Motor speed control

**Installation**
Clone Repository
git clone https://github.com/YOUR_USERNAME/IoT_Based_Smart_Multi-Application_Surveillance_Robot.git

Open Arduino IDE
Open the following sketches:
ESP32_Cam/ESP32_Cam.ino
ESP32_Code/ESP32_Code.ino

Install these libraries through the Arduino Library Manager:
WiFi
WebServer
ESP32Servo
TinyGPSPlus
esp32-camera

Update the Wi-Fi credentials in both ESP32 programs:
const char* ssid = "Your_WiFi_Name";
const char* password = "Your_WiFi_Password";

Upload the Code
Upload ESP32_Cam.ino to the ESP32-CAM.
Upload ESP32_Code.ino to the ESP32 Development Board.
Note the IP addresses displayed in the Serial Monitor.
Update the IP addresses in ESP.js.

Example:
const controlIP = "192.168.x.x";
const motionIP = "192.168.x.x";

**Run the Dashboard**
Open:
frontend/webpage_html.html

**Future Improvements**
Mobile application
Cloud database integration
AI object detection
Face recognition
Obstacle avoidance
Night vision support
Cloud image storage
MQTT communication
Voice control

Author:
Chethan Kumar K R
