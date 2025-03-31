#include <MKL25Z4.h>
#include <stdio.h>
#include "motorControl.h"
#include "receiveUART.h"
#include "decode.h"


int main(void) {
	SystemCoreClockUpdate();
	initMotor();
	Init_UART2(9600);

	while (1) {
		uint8_t *data;
		if(received){
			//motorControl(forward, 80, 0b00);
			decode(received_byte);
			received = 0;
		}
	}
}
