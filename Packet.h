#include <PS4Controller.h>

typedef struct {
  uint8_t A : 4;
  uint8_t B : 1;
  uint8_t C : 1;
  uint8_t OP : 2; 
} TPacket;

TPacket makeMovementPacket(){
  TPacket movementPacket;
  movementPacket.OP = 0b00;
  movementPacket.A = 0;
  movementPacket.B = 0;
  movementPacket.C = 0;
  //set error of margin for leftYCoordinate to be +-10, if Y is within +-10, consider it as 0;
  int8_t leftYCoordinate = abs(PS4.LStickY()) >= 10 ? PS4.LStickY() : 0;
  //set error of margin for rightXCoordinate to be +-10, if X is within +-10, consider it as 0
  int8_t rightXCoordinate =  abs(PS4.RStickX()) >= 10 ? PS4.RStickX() : 0;
  //set error of margin for rightYCoordinate to be +-10, if Y is within +-10, consider it as 0
  uint8_t rightYCoordinate =  abs(PS4.RStickY()) >= 10 ? abs(PS4.RStickY()) : 0;
  if (!leftYCoordinate && !rightXCoordinate && !rightYCoordinate){
    return movementPacket;
  }
  //set reverse bit based on leftYCoordinate
  if (leftYCoordinate < 0){
    movementPacket.C = 0b1;
  } 

  //set boost bit based if R2 is pushed more than half the maximum amount
  if (PS4.R2Value() >= 128){
      movementPacket.B = 0b1;
  } 

  //right stick is not touched, move straight
  if (!rightXCoordinate && !rightYCoordinate) {
    movementPacket.A = 0b1000;
    return movementPacket;
  }

  if (rightXCoordinate > 0){
    movementPacket.A |= 0b1000;
  }

  //consider Y = 0 to be maximum angle turn and Y = +- 128 to be no minimum angle turn
  uint8_t angleBits = 7 - (rightYCoordinate / 18);
  movementPacket.A |= angleBits;

  return movementPacket;
}

TPacket makeLEDPacket(){
  TPacket LEDPacket;
  LEDPacket.OP = 0b01;
  LEDPacket.A = 0;
  LEDPacket.B = 0;
  LEDPacket.C = 0;
  return LEDPacket;
}

TPacket makeAudioPacket(){
  TPacket audioPacket;
  audioPacket.OP = 0b10;
  audioPacket.A = 0;
  audioPacket.B = 0;
  audioPacket.C = 0b1;
  return audioPacket;
}

void sendPacket(TPacket packet){
  uint8_t data = *(uint8_t*)&packet;
  Serial2.write(data);
  Serial.println(data,HEX);
}

void controlFunction(){
  if (PS4.isConnected()){
    TPacket packetToSend;
    if (PS4.Square()){
      packetToSend = makeAudioPacket();
    } else {
      packetToSend = makeMovementPacket();
    }
    sendPacket(packetToSend);
  }
}

