#include "fetchData.h"
#include "motorControlV3.c"
#include "circularQueue.h"

void decode(TPacket *data){
    // Movement OP
    if(data->OP == 0b00){
        int speed = (data->B) ? 100 : 75;
        if(data->A){
            motorControl()
        } else if(data->B) {
            // reverse true
        } else {

            // turn on spot
        }
    }

    // Audio OP
    if(data->OP == 0b10){
        if(data-D){
            // complete function
        }

    }
}