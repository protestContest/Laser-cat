#include <Servo.h>
#include <math.h>
#include <stdlib.h>

// 1/100s sec
const int SLOW = 200;
const int FAST = 40;

const int SWITCH_PIN = 2;
const int GND = 3;
const int POT_PIN = A5;

// servo angle
const int X_MOTOR_MIN = 0;
const int X_MOTOR_MAX = 160;
const int Y_MOTOR_MIN = 85;
const int Y_MOTOR_MAX = 250;

// servo angle
float min_x = 50;
float max_x = 100;
float min_y = 85;
float max_y = 150;
float origin_x = 64;
float origin_y = 85;
float minimal_movement = 5;

// ms
int min_freeze = 50;
int max_freeze = 300;

// servo angle
float x_position = origin_x;
float y_position = origin_y;

float lSpeed = SLOW;

Servo x_servo;
Servo y_servo;

String type = "standard";
volatile int playTtl = 0;
int laserTtl = 0;
float movement_time = SLOW;
bool laserIsOn = false;

float analogReads[10];
int analogReadsHead = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Startup");
  y_servo.attach(9);
  x_servo.attach(6);
  x_servo.write(origin_x);
  y_servo.write(origin_y);
  pinMode (12, OUTPUT); // laser
  pinMode (GND, OUTPUT);
  digitalWrite(GND, LOW);
  pinMode (SWITCH_PIN, INPUT); // switch
//  reset();
  laserOn();

  attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), switchInterrupt, LOW);
}

void loop() {
  origin_y = analogReadAverage();
  
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
        Serial.println("Manual reset");
        reset();
      } else if (c == 'b') {
        Serial.println("Drawing borders");
        drawBorders();
      }
    } else {
      commandPoint();
    }
  }

  if (playTtl > 1) {
    playTtl--;
    standard();;
  } else if (playTtl == 1) {
    Serial.println("Untriggered");
    playTtl--;
    reset();
    laserTtl = 1000000;
  } else {
    y_servo.write(origin_y);
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
  float movement_time = random(FAST,SLOW);
  lSpeed += random(-5, 5);
  lSpeed = max(min(movement_time, FAST), SLOW);

  int newX = getNewX();
  int newY = getNewY();
  float dist = sqrt(pow(newX, 2) + pow(newY, 2));
  float raw_t = dist / lSpeed;
  float normal_t = (raw_t - 0.7) * (1/0.3);
//  float movement_time = SLOW - normal_t * (SLOW - FAST) + FAST;

  //Serial.println(movement_time);  
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

    x_servo.write(round(cur_x));
    y_servo.write(round(cur_y));
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

int analogReadAverage() {
  float potVal = 0.5*analogRead(POT_PIN);
  analogReads[analogReadsHead] = potVal;
  analogReadsHead = (analogReadsHead + 1) % 10;

  float analogSum = 0;
  for (int i = 0; i < 10; i++) {
    analogSum += analogReads[i];
  }
  int avg = floor(analogSum / 10);
  return avg;
}

void switchInterrupt() {
  if (playTtl > 0 && playTtl < 99) {
    playTtl = 1;
  } else if (playTtl == 0) {
    Serial.println("Triggered");
    laserOn();
    playTtl = 100;
  }
}

