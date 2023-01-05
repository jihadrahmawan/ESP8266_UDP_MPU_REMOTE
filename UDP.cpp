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
int PITCH_RIGHT = 0;
int PITCH_LEFT = 0;
int ROLL_FRONT = 0;
int ROLL_BACK = 0;
int last_PITCH = 0;
int last_ROLL = 0;
int m_roll = 0;
int m_pitch = 0;
bool isCalibrated = false;
const int PIN_LED_FRONT = 5;  //D5
const int PIN_LED_RIGHT = 8;  //D8
const int PIN_LED_BACK = 7;   //D7
const int PIN_LED_LEFT = 6;   //D6
int CALIBRATE_STEP = 100;
int cal_cnt = 0;
int SIGNAL_FRONT = 0;
int SIGNAL_RIGHT = 0;
int SIGNAL_BACK = 0;
int SIGNAL_LEFT = 0;
int step_calibrate = 0;
int total_ = 0;
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
    
    if (step_calibrate == 0){
        cal_cnt++;
        digitalWrite(PIN_LED_FRONT, 1);
        digitalWrite(PIN_LED_RIGHT, 0);
        digitalWrite(PIN_LED_BACK, 0);
        digitalWrite(PIN_LED_LEFT, 0);
        total_ = total_ + ROLL;
        if (cal_cnt>=CALIBRATE_STEP){
          ROLL_FRONT = total_ / cal_cnt;
          step_calibrate = 1;
          cal_cnt = 0;
          total_ = 0;
        } 
    }
    if (step_calibrate == 1){
       cal_cnt++;
        digitalWrite(PIN_LED_FRONT, 0);
        digitalWrite(PIN_LED_RIGHT, 1);
        digitalWrite(PIN_LED_BACK, 0);
        digitalWrite(PIN_LED_LEFT, 0);
        total_ = total_ + PITCH;
        if (cal_cnt >= CALIBRATE_STEP){
          PITCH_RIGHT = total_ / cal_cnt;
          step_calibrate = 2;
          cal_cnt = 0;
          total_ = 0;
        }  
    }
    if (step_calibrate == 2){
        cal_cnt++;
        digitalWrite(PIN_LED_FRONT, 0);
        digitalWrite(PIN_LED_RIGHT, 0);
        digitalWrite(PIN_LED_BACK, 1);
        digitalWrite(PIN_LED_LEFT, 0);
        total_ = total_ + ROLL;
        if (cal_cnt >= CALIBRATE_STEP){
          ROLL_BACK = total_ / cal_cnt;
          step_calibrate = 3;
          cal_cnt = 0;
          total_ = 0;
        }  
    }
    if (step_calibrate == 3){
        cal_cnt++;
        digitalWrite(PIN_LED_FRONT, 0);
        digitalWrite(PIN_LED_RIGHT, 0);
        digitalWrite(PIN_LED_BACK, 0);
        digitalWrite(PIN_LED_LEFT, 1);
        total_ = (total_) + (PITCH);
        if (cal_cnt >= CALIBRATE_STEP){
          PITCH_LEFT = total_ / cal_cnt;
          //step_calibrate = 4;
          cal_cnt = 0;
          total_ = 0;
          isCalibrated = true;
        }  
    }
   delay(50);
  } else {
    if (ROLL >= ROLL_FRONT) {
      SIGNAL_FRONT = 1;
      digitalWrite(PIN_LED_FRONT, HIGH);
    } else {
      SIGNAL_FRONT = 0;
      digitalWrite(PIN_LED_FRONT, LOW);
    }

    if (PITCH >= PITCH_RIGHT) {
      SIGNAL_RIGHT = 1;
      digitalWrite(PIN_LED_RIGHT, HIGH);
    } else {
      SIGNAL_RIGHT = 0;
      digitalWrite(PIN_LED_RIGHT, LOW);
    }

    if (ROLL <= ROLL_BACK) {
      SIGNAL_BACK = 1;
      digitalWrite(PIN_LED_BACK, HIGH);
    } else {
      SIGNAL_BACK = 0;
      digitalWrite(PIN_LED_BACK, LOW);
    }
    if (PITCH >= PITCH_LEFT) {
      SIGNAL_LEFT = 1;
      digitalWrite(PIN_LED_LEFT, HIGH);
    } else {
      SIGNAL_LEFT = 0;
      digitalWrite(PIN_LED_LEFT, LOW);
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
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    // put your main code here, to run repeatedly:
  }
}
