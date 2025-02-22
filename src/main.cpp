#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>
#include <ESP32Servo.h>
#include <camera_server.h>

#define ESC_PIN 13 // Use a PWM pin
#define SERVO_PIN 14 // Do not use pin 16 on ESP32CAM!

// ESP AP credentials
const char* ssid = "ESP32-RC-Car";
const char* password = "password";
WiFiManager wm;

CameraServer cameraServer;

// UDP parameters
WiFiUDP udp;
const unsigned int localPort = 12345; // Port to listen on
char incomingPacket[255];            // Buffer for incoming packets

Servo esc;
// Define pulse width range (adjust based on your ESC)
const int escPwmFrequency = 50; // PWM frequency for ESC
const int escMinPulse = 1000; // Minimum throttle (1 ms)
const int escMaxPulse = 2000; // Maximum throttle (2 ms)

Servo servo;
// Define pulse width range (adjust based on your ESC)
const int servoPwmFrequency = 50; // PWM frequency for servo
const int servoMinPulse = 500; // Minimum throttle (0.5 ms)
const int servoMaxPulse = 2500; // Maximum throttle (2.5 ms)

// Arming not necessary, can be done manually by user.
// void armESC(int waitTimeSeconds) {
//     Serial.println("Arming the ESC...");
//     esc.writeMicroseconds(1510);  // Neutral throttle
//     delay(waitTimeSeconds * 1000);  // Wait for ESC to acknowledge throttle
//     Serial.println("ESC is armed!");
// }

void splitString(const char* input, char delimiter, char* part1, char* part2) {
  // Find the delimiter in the input string
  const char* delimiterPos = strchr(input, delimiter);
  
  if (delimiterPos != NULL) {
      // Calculate the length of the first part and copy it
      size_t lenPart1 = delimiterPos - input;

      strncpy(part1, input, lenPart1);
      part1[lenPart1] = '\0'; // Null-terminate the first part

      // Copy the second part starting after the delimiter
      strcpy(part2, delimiterPos + 1);
  } else {
      // If the delimiter is not found, copy the whole input to part1
      strcpy(part1, input);
      part2[0] = '\0'; // Set part2 as an empty string
  }
}

void setup() {
  // delay(2000); // Delay for 2 seconds to allow time to open the Serial Monitor
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  bool res;
  res = wm.autoConnect(ssid, password); // password protected ap
  if(!res) {
      Serial.println("Failed to connect");
  } 
  else {  
      Serial.println("Successfully connected to WiFi.");
  }

  cameraServer.setup();
  cameraServer.startCameraServer(); // this runs on a different core ???

  // Start listening for UDP packets
  if (udp.begin(localPort)) {
    Serial.print("Listening on UDP port ");
    Serial.println(localPort);
  } else {
    Serial.println("Failed to start UDP.");
  }

  esc.setPeriodHertz(escPwmFrequency); // PWM frequency for ESC
  esc.attach(ESC_PIN, escMinPulse, escMaxPulse); // Attach ESC to the same pin
  // armESC(5); // Arm the ESC

  servo.setPeriodHertz(servoPwmFrequency); // PWM frequency for SG90
  servo.attach(SERVO_PIN, servoMinPulse, servoMaxPulse); // Minimum and maximum pulse width (in µs) to go from 0° to 180
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    // Read incoming UDP packet
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = '\0'; // Null-terminate the string
    }

    Serial.printf("Received packet of size %d from %s:%d\n", packetSize, 
                  udp.remoteIP().toString().c_str(), udp.remotePort());
    Serial.printf("Contents: %s\n", incomingPacket);

    if (incomingPacket[0] == 'r') {
      wm.resetSettings();
      ESP.restart();
    }

    char servoString[4];
    char motorString[4];
    splitString(incomingPacket, ',', servoString, motorString);
    int servoAngle = atoi(servoString);
    int motorSpeed = atoi(motorString);
    Serial.printf("Servo angle: %d\n", servoAngle);
    Serial.printf("Motor speed: %d\n", motorSpeed);
    servo.write(servoAngle);
    esc.writeMicroseconds(motorSpeed);
    
    // // Optionally send a response back
    // udp.beginPacket(udp.remoteIP(), udp.remotePort());
    // udp.print("Message received!");
    // udp.endPacket();
  }
}
