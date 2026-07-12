#include "esp_camera.h"
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#include <WiFi.h>
#include <WebServer.h>

// Motor control pins
#define IN0 12
#define IN1 13
#define IN2 14
#define IN3 15

// #define PWDN_GPIO_NUM 32
// #define RESET_GPIO_NUM -1  // Not connected
// #define XCLK_GPIO_NUM 0
// #define SIOD_GPIO_NUM 26
// #define SIOC_GPIO_NUM 27

// #define Y9_GPIO_NUM 35
// #define Y8_GPIO_NUM 34
// #define Y7_GPIO_NUM 39
// #define Y6_GPIO_NUM 36
// #define Y5_GPIO_NUM 21
// #define Y4_GPIO_NUM 19
// #define Y3_GPIO_NUM 18
// #define Y2_GPIO_NUM 5

// #define VSYNC_GPIO_NUM 25
// #define HREF_GPIO_NUM 23
// #define PCLK_GPIO_NUM 22

//Flash LED
#define FLASH_GPIO_NUM 4

// Replace with your Wi-Fi credentials
const char* ssid = "Redmi Note 13 5G";
const char* password = "87654321";

WebServer server(80);

// Mototr Controll Section
void printPinStatus() {
  Serial.print("IN0: ");
  Serial.println(digitalRead(IN0));
  Serial.print("IN1: ");
  Serial.println(digitalRead(IN1));
  Serial.print("IN2: ");
  Serial.println(digitalRead(IN2));
  Serial.print("IN3: ");
  Serial.println(digitalRead(IN3));
}

void stopMotors() {
  digitalWrite(IN0, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
}
void flashledON(){
 digitalWrite(FLASH_GPIO_NUM, HIGH);
  Serial.println("Flash turned ON");
}
void flashledOFF(){
  digitalWrite(FLASH_GPIO_NUM, LOW);
   Serial.println("Flash turned OFF");
}

//capture image
void handleCapture() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  // Convert the frame to JPEG if not already
  if (fb->format != PIXFORMAT_JPEG) {
    Serial.println("Converting to JPEG...");
    uint8_t *jpeg_buf = NULL;
    size_t jpeg_len = 0;
    
    if (!frame2jpg(fb, 80, &jpeg_buf, &jpeg_len)) {
      Serial.println("JPEG conversion failed");
      esp_camera_fb_return(fb);
      server.send(500, "text/plain", "JPEG conversion failed");
      return;
    }

    // Send the JPEG image
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.setContentLength(jpeg_len);
    server.send(200, "image/jpeg", "");
    WiFiClient client = server.client();
    client.write(jpeg_buf, jpeg_len);
    free(jpeg_buf);
    Serial.println("JPEG image sent");

  } else {
    // Send the JPEG image directly
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.setContentLength(fb->len);
    server.send(200, "image/jpeg", "");
    WiFiClient client = server.client();
    client.write(fb->buf, fb->len);
    Serial.println("JPEG image sent directly");
  }

  esp_camera_fb_return(fb);
}



void handleControl() {
  Serial.println("[CONTROL] Request received");

  if (!server.hasArg("cmd")) {
    Serial.println("[CONTROL] Missing 'cmd' argument.");
    server.send(400, "text/plain", "Missing command");
    return;
  }

  String cmd = server.arg("cmd");
  cmd.toUpperCase();
  Serial.println("[CONTROL] cmd = " + cmd);

  stopMotors();  // Stop before executing new command

  if (cmd == "B") {
    Serial.println("Moving Forward");
    digitalWrite(IN0, HIGH);
    digitalWrite(IN2, HIGH);
  } else if (cmd == "F") {
    Serial.println("Moving Backward");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN3, HIGH);
  } else if (cmd == "L") {
    Serial.println("Turning Left");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, HIGH);
  } else if (cmd == "R") {
    Serial.println("Turning Right");
    digitalWrite(IN0, HIGH);
    digitalWrite(IN3, HIGH);
  } else if (cmd == "S") {
    Serial.println("Stopping");
    stopMotors();
  }
    else if (cmd == "ON") {
    Serial.println("Flash ON");
    flashledON();
  }
    else if (cmd == "OFF") {
    Serial.println("Flash OFF");
    flashledOFF();
  } 
  else {
    Serial.println("Unknown command: " + cmd);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "text/plain", "Unknown command");
    return;
  }

  printPinStatus();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK");
}

void streamVideo(void* pvParameters) {
  WiFiClient client = *((WiFiClient*)pvParameters);
  delete (WiFiClient*)pvParameters;  // Clean up allocated memory

  String response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "\r\n";
  client.print(response);

  while (client.connected()) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      break;
    }

    uint8_t* _jpg_buf = NULL;
    size_t _jpg_buf_len = 0;
    bool converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);

    if (converted) {
      client.print("--frame\r\n");
      client.print("Content-Type: image/jpeg\r\n");
      client.print("Content-Length: " + String(_jpg_buf_len) + "\r\n\r\n");
      client.write(_jpg_buf, _jpg_buf_len);
      client.print("\r\n");
      free(_jpg_buf);
    }

    esp_camera_fb_return(fb);
    delay(100);  // ~10 FPS
  }

  client.stop();
  Serial.println("Stream client disconnected");
  vTaskDelete(NULL);  // Delete the task
}

camera_fb_t* fb = nullptr;  // Pointer to hold the captured frame

// Start the camera and web server
void startCameraServer() {

  server.on("/stream", HTTP_GET, []() {
    WiFiClient client = server.client();
    WiFiClient* clientCopy = new WiFiClient(client);  // Allocate a copy for the task
    xTaskCreatePinnedToCore(
      streamVideo,   // Function to run
      "streamTask",  // Name
      8192,          // Stack size
      clientCopy,    // Parameter (client pointer)
      1,             // Priority
      NULL,          // Task handle
      1              // Core 1
    );
    // WiFiClient client = server.client();

    // String response =
    //   "HTTP/1.1 200 OK\r\n"
    //   "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
    //   "Access-Control-Allow-Origin: *\r\n"
    //   "\r\n";
    // client.print(response);

    // while (client.connected()) {
    //   // sensor_t* s = esp_camera_sensor_get();
    //   // Serial.printf("Sensor PID: 0x%x\n", s->id.PID);
    //   camera_fb_t* fb = esp_camera_fb_get();
    //   if (!fb) {
    //     Serial.println("Camera capture failed");
    //     break;
    //   }
    //   uint8_t* _jpg_buf = NULL;
    //   size_t _jpg_buf_len = 0;
    //   bool converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);

    //   if (converted) {
    //     client.print("--frame\r\n");
    //     client.print("Content-Type: image/jpeg\r\n");
    //     client.print("Content-Length: " + String(_jpg_buf_len) + "\r\n\r\n");
    //     client.write(_jpg_buf, _jpg_buf_len);
    //     client.print("\r\n");
    //     free(_jpg_buf);
    //   } else {
    //     Serial.println("JPEG compression failed");
    //   }

    //   esp_camera_fb_return(fb);
    //   delay(100);  // ~10 FPS
    // }

    // client.stop();
    // Serial.println("Stream client disconnected");
  });

  server.on("/control", handleControl);
  server.on("/capture", handleCapture);

  server.begin();
  Serial.println("Camera server started.");
}

void captureFrame() {
  // Capture a new frame only if the previous one has been sent
  if (!fb) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected.");
  Serial.print("Camera Stream URL: http://");
  Serial.println(WiFi.localIP());

  // Camera config
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;
  // config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.frame_size = FRAMESIZE_QVGA;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 2;


  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }

  // Motor pins setup
  pinMode(IN0, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  stopMotors();

  startCameraServer();
}

void loop() {
  captureFrame();         // Capture a new frame
  server.handleClient();  // Handle HTTP requests
}
