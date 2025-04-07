#include <MKL25Z4.h>
#include "fetchData.h"
//#include "motorControl.h"
#include <stdbool.h>

#define forward 0
#define rightTurnOnSpot 1
#define reverse 2
#define leftTurnOnSpot 3
#define rightTurnForward 4
#define leftTurnForward 5
#define rightTurnReverse 6
#define leftTurnReverse 7
#define brake 8

volatile bool isComplete = false;
volatile bool isMoving = false;
volatile int state = 8;
volatile int speed = 75;
volatile uint8_t turnFactor = 0;

typedef struct {
    uint8_t A : 3;
    uint8_t B : 1;
    uint8_t C : 1;
    uint8_t D : 1;
    uint8_t OP : 2; 
} TPacket;

void decode(uint8_t raw_data){
    // Movement OP
    TPacket data;
    data.OP = (raw_data >> 6) & 0b11;
    data.D  = (raw_data >> 5) & 0b1;
    data.C  = (raw_data >> 4) & 0b1;
    data.B  = (raw_data >> 3) & 0b1;
    data.A  = (raw_data) & 0b111;
    
    if(data.OP == 0b00){
        speed = (data.B) ? 85 : 100 ;
        turnFactor = data.A % 4;
        if (data.D){
            //forward with turn
            if (data.A){
                state = data.A >> 2 ? rightTurnForward : leftTurnForward;
            } else {
                //forward only
               state = forward;
            }
            isMoving = true;
        } else if (data.C){
            if (data.A){
                //reverse with turn
                state = data.A >> 2 ? rightTurnReverse : leftTurnReverse;
            } else {
                //reverse only
                state = reverse;
            }
            isMoving = true;
        } else if (data.A){
            //turn on the spot
            state = data.A >> 2 ? rightTurnOnSpot : leftTurnOnSpot;
            isMoving = true;
        } else {
            //not moving
            state = brake;
            isMoving = false;
        }
    }

    // Audio OP
    if(data.OP == 0b10){
        if(data.C){
            // complete function
            isComplete = true;
        } else if(data.B){
						isComplete = false;
				}

    }
}
