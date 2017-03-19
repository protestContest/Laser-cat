#include <Servo.h>
#include <math.h>
#include <stdlib.h>
#include "name.h"

// 1/100s sec
const int SLOW = 200;
const int FAST = 20;

const int SWITCH_PIN = 2;
const int SWITCH_GND = 3;

// servo angle
const int X_MOTOR_MIN = 17;
const int X_MOTOR_MAX = 180;
const int Y_MOTOR_MIN = 35;
const int Y_MOTOR_MAX = 180;

// servo angle
float min_x = 110;
float max_x = X_MOTOR_MAX;
float min_y = Y_MOTOR_MIN;
float max_y = 120;
float origin_x = 130;
float origin_y = 35;
float minimal_movement = 5;

// ms
int min_freeze = 50;
int max_freeze = 300;

// servo angle
float x_position = origin_x;
float y_position = origin_y;

float lSpeed = SLOW;

Agent agent;

Servo x_servo;
Servo y_servo;

String type = "standard";
int playTtl = 0;
int laserTtl = 0;
float movement_time = SLOW;
bool laserIsOn = false;

void setup() {
  Serial.begin(9600);
  y_servo.attach(9);
  x_servo.attach(6);
  x_servo.write(origin_x);
  y_servo.write(origin_y);
  pinMode (12, OUTPUT); // laser
  pinMode (SWITCH_GND, OUTPUT);
  digitalWrite(SWITCH_GND, LOW);
  pinMode (SWITCH_PIN, INPUT); // switch
  reset();
  laserOn();
}

void loop() {
  if (Serial.available() > 0) {
    if (Serial.peek() == 'o' || 
        Serial.peek() == 'l' || 
        Serial.peek() == 'p' || 
        Serial.peek() == 'r' || 
        Serial.peek() == 'b') {
      char c = Serial.read();
      if (c == 'l') {
        Serial.print("Laser ");
        toggleLaser();
        if (laserIsOn) { Serial.println("on"); } else { Serial.println("off"); }
      } else if (c == 'o') {
        Serial.println("Stopping");
        playTtl = 1;
      } else if (c == 'p') {
        Serial.println("Playing");
        playTtl = 100;
        laserOn();
      } else if (c == 'r') {
        reset();
      } else if (c == 'b') {
        drawBorders();
      }
    } else {
      commandPoint();
    }
  }

  int switchVal = digitalRead(SWITCH_PIN);
  if (switchVal == LOW) {
    Serial.println("Triggered");
    if (playTtl > 0) {
      playTtl = 1;
    } else {
      laserOn();
      playTtl = 100;
    }
  }

  if (playTtl > 1) {
    playTtl--;
    standard();;
  } else if (playTtl == 1) {
    playTtl--;
    reset();
    laserTtl = 1000000;
  }

  if (laserTtl > 1) {
    laserTtl--;
  } else if (laserTtl == 1) {
    laserTtl--;
//    laserOff();
  }

}

void reset() {
  Serial.print("Reset to ");
  Serial.print(origin_x);
  Serial.print(", ");
  Serial.println(origin_y);
  
  moveTo(origin_x, origin_y, SLOW);
}

void standard() {
//  float movement_time = random(FAST,SLOW);
  lSpeed += random(-5, 5);
  lSpeed = max(min(movement_time, FAST), SLOW);

  int newX = getNewX();
  int newY = getNewY();
  float dist = sqrt(pow(newX, 2) + pow(newY, 2));
  float raw_t = dist / lSpeed;
  float normal_t = (raw_t - 0.7) * (1/0.3);
  float movement_time = SLOW - normal_t * (SLOW - FAST) + FAST;

  Serial.println(movement_time);  
  int random_delay = random(min_freeze, max_freeze);

  moveTo(newX, newY, movement_time);
  delay(random_delay);
}

void commandPoint() {
  int x = Serial.parseInt();
  int y = Serial.parseInt();
  if (true || x >= X_MOTOR_MIN && x <= X_MOTOR_MAX && y >= Y_MOTOR_MIN && y <= Y_MOTOR_MAX) {
    Serial.print("Moving to ");
    Serial.print(x);
    Serial.print(", ");
    Serial.println(y);
    moveTo(x, y, SLOW);
  } else {
    Serial.println("Out of bounds");
  }

  delay(5000);
}

void moveTo(float x, float y, float t) {
  x = min(max(x, min_x), max_x);
  y = min(max(y, min_y), max_y);
  
  float x_dist = x - x_position;
  float y_dist = y - y_position;
  float cur_x = x_position;
  float cur_y = y_position;

  for(int i = 1; i <= t; i++) {
    cur_x = floor(x_position + i*(x_dist/t));
    cur_y = floor(y_position + i*(y_dist/t));
    
    x_servo.write(floor(cur_x));
    y_servo.write(floor(cur_y));
    delay(10);
  }

  x_position = x;
  y_position = y;
}

void drawBorders() {
  moveTo(min_x, min_y, FAST);
  delay(1000);
  moveTo(min_x, max_y, SLOW);
  delay(1000);
  moveTo(max_x, max_y, SLOW);
  delay(1000);
  moveTo(max_x, min_y, SLOW);
  delay(1000);
  moveTo(min_x, min_y, SLOW);

  delay(2000);
}

float getNewX() {
  float x_new_position = random(min_x + minimal_movement, max_x - minimal_movement);

  if( (x_new_position > x_position) && (abs(x_new_position - x_position) < minimal_movement )) {
    x_new_position = x_new_position + minimal_movement;
  }  else if ( (x_new_position < x_position) && (abs(x_new_position - x_position) < minimal_movement )) {
    x_new_position = x_new_position - minimal_movement;
  }

  return x_new_position;
}

float getNewY() {
  float y_new_position = random(min_y + minimal_movement, max_y - minimal_movement);

  if( (y_new_position > y_position) && (abs(y_new_position - y_position) < minimal_movement )) {
    y_new_position = y_new_position + minimal_movement;
  }  else if ( (y_new_position < y_position) && (abs(y_new_position - y_position) < minimal_movement )) {
    y_new_position = y_new_position - minimal_movement;
  }

  return y_new_position;
}

Vector getNearVector(Vector origin) {
  float dist = random(minimal_movement*2, minimal_movement*4);
  float r = static_cast <float>(rand()) / static_cast <float>(RAND_MAX);
  double dir = origin.direction + 0.1*r - 0.5;
  float x = constrain(origin.x + dist*cos(dir), min_x, max_x);
  float y = constrain(origin.y + dist*sin(dir), min_y, max_y);
  
  Vector v = Vector { x, y, dir };

  printVector(v);
  return v;
}

void laserOn() {
  laserIsOn = true;
  digitalWrite (12, HIGH);
}

void laserOff() {
  laserIsOn = false;
  digitalWrite (12, LOW);
}

void toggleLaser() {
  if (laserIsOn) {
    laserOff();
  } else {
    laserOn();
  }
}

void printVector(Vector v) {
  Serial.print(v.x);
  Serial.print(", ");
  Serial.print(v.y);
  Serial.print(", ");
  Serial.println(v.direction);
}

void sine() {
  for (double i = -1.0; i <= 1.0; i = i + 0.1) {
    Serial.println (sin(i));
    delay(10);
  }
 
  // Describe the downward swing of the sine wave
  for (double i = 1.0; i >= -1.0; i = i - 0.1) {
    Serial.println (sin(i));
    delay(10);
  }
}

