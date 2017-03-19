struct Point {
  float x;
  float y;
};

struct Vector {
  float x;
  float y;
  double direction;
};

struct Agent {
  Vector position;
  float dir;
  float movement_time;
  float delay;
};

