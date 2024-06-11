#include <SoftwareSerial.h>
#include "ABlocksIOTMQTTESP8266.h"

// MQTT Config

const char mqtt_broker[]="iot.eclipse.org";
const int mqtt_port=1883;
const char mqtt_user[]="";
const char mqtt_pass[]="";
const char mqtt_clientid[]="xxxxxxxxxx";
const char mqtt_wifi_ssid[]="xxxxxxxxxxx";
const char mqtt_wifi_pass[]="xxxxxxxxx";
SoftwareSerial mqtt_esp8266_serial(3,2);
ESP8266 mqtt_esp8266_wifi(&mqtt_esp8266_serial);
char mqtt_payload[64];
unsigned long task_time_ms=0;

double mqtt_payload2double(unsigned char *_payload, int _length){
  int i;
  for (i = 0; i<_length && i<64; i++){
    mqtt_payload[i] = _payload[i];
  }
  mqtt_payload[i] = 0;
  return atof(mqtt_payload);
}

String mqtt_payload2string(unsigned char *_payload, int _length){
  int i;
  for (i = 0; i<_length && i<64; i++){
    mqtt_payload[i] = _payload[i];
  }
  mqtt_payload[i] = 0;
  return String(mqtt_payload);
}
void mqtt_callback(char* _topic, unsigned char* _payload, unsigned int _payloadlength){
  double v=mqtt_payload2double(_payload,_payloadlength);
  String vt=mqtt_payload2string(_payload,_payloadlength);
  if(String(_topic).equals(String(String("Distancia/Frontal"))))EstadoLed=v;
}

void mqtt_subscribe(){
  ABlocksIOT.Subscribe(String(String("Distancia/Frontal")));
}

// Pines setup

const int trig1 = 11;
const int echo1 = 8;

const int trig2 = 7;
const int echo2 = 4;

const int trig3 = 13;
const int echo3 = 12;

const int motor1Pin1 = 5;
const int motor1Pin2 = 6;
const int motor2Pin1 = 9;
const int motor2Pin2 = 10;

const int irSensor = A0;

bool left_turn = false;
bool obstacle_end = false;

int i = 1;
int i2 = 1;
int i3 = 1;
int leftTurnTime=850;
int rightTurnTime=1700;
int frontTime=850;

long frontDuration;
long leftDuration;
long rightDuration;

int distance;
int frontdistance;
int leftdistance;
int rightdistance;

void setup() {
  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);

  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);

  pinMode(trig3, OUTPUT);
  pinMode(echo3, INPUT);

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);

  pinMode(irSensor, INPUT);

  Serial.begin(9600);
  
  mqtt_esp8266_serial.begin(9600);
  ABlocksIOT.begin(mqtt_broker,mqtt_port, mqtt_user,mqtt_pass, mqtt_clientid, mqtt_esp8266_wifi, mqtt_wifi_ssid, mqtt_wifi_pass, mqtt_callback, mqtt_subscribe);
}

void loop() {
  mqtt_esp8266_serial.listen(); ABlocksIOT.loop();
  
  while(){
    if (obstacle_end == true) {
      Serial.println("Obstacle end maneuver started");
      movefront();
      frontDistance();
      if (distance <= 15) {
        halt();
        delay(500);
        if (left_turn == false) {
          moveleft();
          delay(leftTurnTime);
          movefront();
          delay(frontTime);
          moveleft();
          delay(leftTurnTime);
          movefront();
          while (i == 1) {
            frontDistance();
            if (distance <= 15) {
              halt();
              delay(500);
              moveleft();
              delay(leftTurnTime);
              movefront();
              delay(1000);
              moveright();
              delay(rightTurnTime);
              movefront();
              i = 0;
            }
            else {
              movefront();
            }
          }
        }
        else {
          moveright();
          delay(rightTurnTime);
          movefront();
          delay(frontTime);
          moveright();
          delay(rightTurnTime);
          movefront();
          while (i == 1) {
            frontDistance();
            if (distance <= 15) {
              halt();
              delay(500);
              moveright();
              delay(rightTurnTime);
              movefront();
              delay(1000);
              moveleft();
              delay(leftTurnTime);
              movefront();
              i = 0;
            }
            else {
              movefront();
            }
          }
        }

        if (left_turn == false) {
          movefront();
          delay(1000);
          halt();
          while (i2 == 1) {
            rightDistance();
            if (distance <= 50) {
              movefront();
            }
            else {
              halt();
              delay(500);
              movefront();
              delay(1500);
              moveright();
              delay(rightTurnTime);
              movefront();
              i2 = 2;
            }
          }
          while (i3 == 1) {
            frontDistance();
            if (distance <= 10) {
              halt();
              delay(500);
              moveleft();
              delay(leftTurnTime);
              i3 = 2;
            }
            else {
              movefront();
            }
          }
        }
        else {
          movefront();
          delay(1000);
          halt();
          while (i2 == 1) {
            leftDistance();
            delay(500);
            if (distance <= 10) {
              movefront();
            }
            else {
              halt();
              delay(500);
              movefront();
              delay(1500);
              moveleft();
              delay(leftTurnTime);
              movefront();
              i2=2;
              while (i3 == 1) {
                frontDistance();
                if (distance <= 10) {
                  halt();
                  delay(500);
                  moveright();
                  delay(rightTurnTime);
                  movefront();
                  i3 = 3;
                }
                else {
                  movefront();
                }
              }
            }
          }
        }
        i = 1;
        i2 = 1;
        i3 = 1;
        obstacle_end = false;
        Serial.println(" ");
        Serial.println(" 2 Part obstacle maneuver finish ");
        left_turn=!left_turn;
      }
      else {
        movefront();
      }
    }
    else {
      movefront();
      frontDistance();
      if (distance <= 10) {
        halt();
        delay(500);
        if (left_turn == false) {
          Serial.println("Right U turn");
          halt();
          rightDistance();
          if (distance >= 30) {
            moveright();
            delay(rightTurnTime);
            movefront();
            delay(frontTime);
            halt();
            leftDistance();
            if (distance >= 15) {
              obstacle_end = true;
            }
            moveright();
            delay(rightTurnTime);
            movefront();
          }
          else {
            moveright();
            delay(rightTurnTime);
            moveright();
            delay(rightTurnTime);
            movefront();
          }
          left_turn = !left_turn;
        }
        else {
          Serial.println("Left u turn");
          halt();
          leftDistance();
          delay(500);
          if (distance >= 30) {
            moveleft();
            delay(leftTurnTime);
            movefront();
            delay(frontTime);
            halt();
            rightDistance();
            if (distance >= 10) {
              obstacle_end = true;
            }
            moveleft();
            delay(leftTurnTime);
            movefront();
          }
          else {
            moveleft();
            delay(leftTurnTime);
            moveleft();
            delay(leftTurnTime);
            movefront();
          }
          left_turn = !left_turn;
        }
      }
      else {
        movefront();
      }
    }
  }
}

void movefront() {
  int irValue = analogRead(irSensor);
  if (irValue != 1) { // No floor detected
    halt();
    delay(500);
    leftDistance();
    rightDistance();
    if (rightdistance > leftdistance) {
      moveright();
    } else {
      moveleft();
    }
  }
 
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
  Serial.println("Moving Front");
}

void moveback() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
  Serial.println("Moving Back");
}

void moveleft() {
  Serial.println("Turning left");
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
  delay(850);
  halt();
}

void moveright() {
  Serial.println("Turning Right");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
  delay(850);
  halt();
}

void halt() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  Serial.println("Stopped");
}

void leftDistance() {
  digitalWrite(trig2, LOW);
  delayMicroseconds(2);
  digitalWrite(trig2 , HIGH);
  delayMicroseconds(10);
  digitalWrite(trig2, LOW);
  leftDuration = pulseIn(echo2, HIGH);
  leftdistance = leftDuration * 0.034 / 2;
  distance = leftdistance;
  Serial.print("Left Distance: ");
  Serial.println(leftdistance);
}

void rightDistance() {
  digitalWrite(trig3, LOW);
  delayMicroseconds(2);
  digitalWrite(trig3 , HIGH);
  delayMicroseconds(10);
  digitalWrite(trig3, LOW);
  rightDuration = pulseIn(echo3, HIGH);
  rightdistance = rightDuration * 0.034 / 2;
  distance = rightdistance;
  Serial.print("Right Distance: ");
  Serial.println(rightdistance);
}

void frontDistance() {
  digitalWrite(trig1, LOW);
  delayMicroseconds(2);
  digitalWrite(trig1 , HIGH);
  delayMicroseconds(10);
  digitalWrite(trig1, LOW);
  frontDuration = pulseIn(echo1, HIGH);
  frontdistance = frontDuration * 0.034 / 2;
  distance = frontdistance;
  Serial.print("Front Distance: ");
  Serial.println(frontdistance);
}
