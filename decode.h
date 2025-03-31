#include <MKL25Z4.h>
#include "fetchData.h"
//#include "motorControl.h"

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
    data.D  = (raw_data >> 5) & 0b01;
    data.C  = (raw_data >> 4) & 0b01;
    data.B  = (raw_data >> 3) & 0b01;
    data.A  = (raw_data) & 0b111;
    
    if(data.OP == 0b00){
        int speed = (data.B) ? 100 : 75;
        int turnFactor = data.A % 4;
        int state;
        if (data.D){
            //forward with turn
            if (data.A){
                state = data.A >> 2 ? rightTurnForward : leftTurnForward;
            } else {
               state = forward;
            }
        } else if (data.C){
            if (data.A){
                state = data.A >> 2 ? rightTurnReverse : leftTurnReverse;
            } else {
                state = reverse;
            }
        } else if (data.A){
            state = data.A >> 2 ? rightTurnOnSpot : leftTurnOnSpot;
						speed = 100;
        } else {
            state = brake;
        }
        motorControl(state,speed,turnFactor);
    }

    // Audio OP
    if(data.OP == 0b10){
        if(data.D){
            // complete function
        }

    }
}
