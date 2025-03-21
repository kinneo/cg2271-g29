#include <PS4Controller.h>
#include <Packet.h>
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, -1);
  PS4.begin();
  Serial.println("Ready.");
}

void loop() {
    controlFunction();
}