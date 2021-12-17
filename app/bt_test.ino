
char data = 0;

void setup() {
  Serial.begin(38400);
}

void loop() {
  data = Serial.read();
  Serial.print(data);
}
