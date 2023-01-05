#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <Wire.h>
#include "MPU6050.h"


MPU6050 mpu;

const char ssid[] = "krti2021"; // SSID
const char pass[] = "krti2022";  // password

static WiFiUDP wifiUdp; 
static const char *kRemoteIpadr = "192.168.100.29"; //IP RASPI
static const int kRmoteUdpPort = 9000; // PORT

static void WiFi_setup()
{
  static const int kLocalPort = 7000;  //ignore
  WiFi.begin(ssid, pass);
  while( WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...")
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }  
  wifiUdp.begin(kLocalPort);
}

static void Serial_setup()
{
  Serial.begin(115200);
  Serial.println(""); // to separate line  
}
              // PORT

int PITCH = 0;
int ROLL = 0;
int PITCH_MAX = 0;
int PITCH_MIN = 0;
int ROLL_MAX = 0;
int ROLL_MIN = 0;
int last_PITCH = 0;
int last_ROLL = 0;
int m_roll = 0;
int m_pitch = 0;
bool isCalibrated = false;
const int PIN_LED_FRONT = 5;  //D5
const int PIN_LED_RIGHT = 8;  //D8
const int PIN_LED_BACK = 7;   //D7
const int PIN_LED_LEFT = 6;   //D6
int CALIBRATE_STEP = 50;
int cal_cnt = 0;
int SIGNAL_FRONT = 0;
int SIGNAL_RIGHT = 0;
int SIGNAL_BACK = 0;
int SIGNAL_LEFT = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_LED_FRONT, OUTPUT);
  pinMode(PIN_LED_RIGHT, OUTPUT);
  pinMode(PIN_LED_BACK, OUTPUT);
  pinMode(PIN_LED_LEFT, OUTPUT);
  
  digitalWrite(PIN_LED_FRONT, LOW);
  digitalWrite(PIN_LED_RIGHT, LOW);
  digitalWrite(PIN_LED_BACK, LOW);
  digitalWrite(PIN_LED_LEFT, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  
  
  Serial_setup();
  WiFi_setup();
 
 
  while (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)) {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }



// put your setup code here, to run once:
}

void loop() {

  Vector normAccel = mpu.readNormalizeAccel();
  PITCH = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis * normAccel.YAxis + normAccel.ZAxis * normAccel.ZAxis)) * 180.0) / M_PI;
  ROLL = (atan2(normAccel.YAxis, normAccel.ZAxis) * 180.0) / M_PI;
  int KEY_ARMING = 0;
  int KEY_LAND = 0;
  int KEY_TAKEOFF = 0;

  if (!isCalibrated) {
   
    Serial.println("Cari limit sudut ...");
    
    if (cal_cnt == 0){
      
      PITCH_MIN = PITCH;
      PITCH_MAX = PITCH;
      ROLL_MIN = ROLL;
      ROLL_MAX = ROLL;
      
      
    }else{
    
    if (PITCH < PITCH_MIN ) PITCH_MIN = PITCH;
    if (PITCH > PITCH_MAX) PITCH_MAX = PITCH;
    if (ROLL < ROLL_MIN) ROLL_MIN = ROLL;
    if (ROLL > ROLL_MAX) ROLL_MAX = ROLL;
      
    }

    digitalWrite(PIN_LED_FRONT, HIGH);
    digitalWrite(PIN_LED_RIGHT, HIGH);
    digitalWrite(PIN_LED_BACK, HIGH);
    digitalWrite(PIN_LED_LEFT, HIGH);
    delay(100);
    
    digitalWrite(PIN_LED_FRONT, LOW);
    digitalWrite(PIN_LED_RIGHT, LOW);
    digitalWrite(PIN_LED_BACK, LOW);
    digitalWrite(PIN_LED_LEFT, LOW);
    delay(100);
    cal_cnt++;
    if (cal_cnt > CALIBRATE_STEP) isCalibrated = true;

  } else {
    if (PITCH <= PITCH_MIN) {
      SIGNAL_LEFT = 1;
      digitalWrite(PIN_LED_LEFT, HIGH);
    } else {
      SIGNAL_LEFT = 0;
      digitalWrite(PIN_LED_LEFT, LOW);
    }

    if (PITCH >= PITCH_MAX) {
      SIGNAL_RIGHT = 1;
      digitalWrite(PIN_LED_RIGHT, HIGH);
    } else {
      SIGNAL_RIGHT = 0;
      digitalWrite(PIN_LED_RIGHT, LOW);
    }

    if (ROLL <= ROLL_MIN) {
      SIGNAL_BACK = 1;
      digitalWrite(PIN_LED_BACK, HIGH);
    } else {
      SIGNAL_BACK = 0;
      digitalWrite(PIN_LED_BACK, LOW);
    }
    if (ROLL >= ROLL_MAX) {
      SIGNAL_FRONT = 1;
      digitalWrite(PIN_LED_FRONT, HIGH);
    } else {
      SIGNAL_FRONT = 0;
      digitalWrite(PIN_LED_FRONT, LOW);
    }

    char buffer[10];

    sprintf(buffer, "%d%d%d%d%d%d%d",
            SIGNAL_FRONT,
            SIGNAL_RIGHT,
            SIGNAL_BACK,
            SIGNAL_LEFT,
            KEY_ARMING,
            KEY_LAND,
            KEY_TAKEOFF);

    wifiUdp.beginPacket(kRemoteIpadr, kRmoteUdpPort);
    if (isCalibrated) {
      wifiUdp.print(buffer);
      Serial.print("Sending to ROS ! = "); Serial.println(buff);
    }
    wifiUdp.endPacket();
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    // put your main code here, to run repeatedly:
  }
}
