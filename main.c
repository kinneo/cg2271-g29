#include <MKL25Z4.h>
#include <stdio.h>
#include "motorControl.h"
#include "receiveUART.h"
#include "decode.h"
#include "LED.h"
#include "audio.h"

#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"



void Init_RTOX(){
    serialFlag = osSemaphoreNew(1,1,NULL);
     motorFlag = osSemaphoreNew(1,0,NULL);
     audioRunningFlag = osSemaphoreNew(1,1,NULL);
     audioEndFlag = osSemaphoreNew(1,0,NULL);
     greenMovingFlag = osSemaphoreNew(1,0,NULL);
     greenStationaryFlag = osSemaphoreNew(1,1,NULL);
     redMovingFlag = osSemaphoreNew(1,0,NULL);
     redStationaryFlag = osSemaphoreNew(1,1,NULL);
}

// ----------------------------------------------------------------------------
//     Buzzer Control
// ----------------------------------------------------------------------------

void runningAudioThread(void *argument){
  int TOTAL_DURATION = sizeof(freqRunning) / sizeof(uint16_t);
  for (;;){
    for (int i = 0; i < TOTAL_DURATION; i++) {
        osSemaphoreAcquire(audioRunningFlag, osWaitForever);
        TPM0->MOD = CLOCK / freqRunning[i];  // Set PWM frequency
        TPM0_C0V = (CLOCK / freqRunning[i]) / 8; // 12.5% duty cycle
        osDelay(70); // Play note for 100 ms 
        TPM0_C0V = 0x0; // Turn off buzzer between notes
        isComplete ? osSemaphoreRelease(audioEndFlag) : osSemaphoreRelease(audioRunningFlag);
        osDelay(70);
      }    
  }
}

void stopAudioThread(void *argument){
    int TOTAL_DURATION = sizeof(freqEnd) / sizeof(uint16_t);
    for (;;){
        for (int i = 0; i < TOTAL_DURATION; i++) {
            osSemaphoreAcquire(audioEndFlag, osWaitForever);
            TPM0->MOD = CLOCK / freqEnd[i];
            TPM0_C0V = (CLOCK / freqEnd[i]) / 8; // 12.5% duty cycle
            osDelay(100);
            TPM0_C0V = 0x0;
            isComplete ? osSemaphoreRelease(audioEndFlag) : osSemaphoreRelease(audioRunningFlag);
            osDelay(100);
          }
    }
}

// ----------------------------------------------------------------------------
//     UART Control
// ----------------------------------------------------------------------------
void serialThread(void *argument){
    uint8_t *data;
    for (;;){
        osSemaphoreAcquire(serialFlag, osWaitForever);
		decode(received_byte);
		osSemaphoreRelease(motorFlag);
		osDelay(100);
    }
}

// ----------------------------------------------------------------------------
//     Motor Control
// ----------------------------------------------------------------------------
void motorThread(void *argument){
    for (;;){
        osSemaphoreAcquire(motorFlag,osWaitForever);
        motorControl(state,speed,turnFactor);
        osDelay(100);
    }
}

// ----------------------------------------------------------------------------
//     LED Control
// ----------------------------------------------------------------------------
void greenMovingThread(void *argument){
    const int NUM_LEDs = sizeof(GREEN_LEDS) / sizeof(GREEN_LEDS[0]);
    for (;;){
        for (int i = 0; i < NUM_LEDs; i++) {
            osSemaphoreAcquire(greenMovingFlag,osWaitForever);
            GreenOff(); // Turn off all LEDs before lighting the next
            PTC->PSOR = GREEN_LEDS[i]; // Turn on one LED at a time
            isMoving ? osSemaphoreRelease(greenMovingFlag) : osSemaphoreRelease(greenStationaryFlag);
					osDelay(100);
        }
    }
}

void greenStationaryThread(void *argument){
    for (;;){
        osSemaphoreAcquire(greenStationaryFlag,osWaitForever);
        GreenOff(); // Ensure all LEDs are off first
        PTC->PSOR = (MASK(GREEN_PTC4) | MASK(GREEN_PTC5) | MASK(GREEN_PTC6) | MASK(GREEN_PTC10) | MASK(GREEN_PTC11) | MASK(GREEN_PTC12) | MASK(GREEN_PTC13) | MASK(GREEN_PTC16)); // Turn on all LEDs
        isMoving ? osSemaphoreRelease(greenMovingFlag) : osSemaphoreRelease(greenStationaryFlag);
    }
}

void redMovingThread(void *argument){
    for (;;){
        osSemaphoreAcquire(redMovingFlag,osWaitForever);
        PTC->PCOR = MASK(7); // Turn ON Red LED
        osDelay(500); // Keep ON for 500ms
        PTC->PSOR = MASK(7); // Turn OFF Red LED
        osDelay(500); // Keep OFF for 500ms
        isMoving ? osSemaphoreRelease(redMovingFlag) : osSemaphoreRelease(redStationaryFlag);
    }
}

void redStationaryThread(void *argument){
    for (;;){
        osSemaphoreAcquire(redStationaryFlag,osWaitForever);
        PTC->PSOR = MASK(7); // Turn ON Red LED
        osDelay(250); // Keep ON for 250ms
        PTC->PCOR = MASK(7); // Turn OFF Red LED
        osDelay(250); // Keep OFF for 250ms
        isMoving ? osSemaphoreRelease(redMovingFlag) : osSemaphoreRelease(redStationaryFlag);
    }
}

int main(void) {
	SystemCoreClockUpdate();
	initMotor();
	Init_UART2(9600);
    InitAudio();
    InitLED();

    osKernelInitialize();
    Init_RTOX();
	osThreadNew(runningAudioThread, NULL, NULL);
    osThreadNew(stopAudioThread, NULL, NULL);
    osThreadNew(serialThread, NULL, NULL);
    osThreadNew(motorThread, NULL, NULL);
    osThreadNew(greenMovingThread, NULL, NULL);
    osThreadNew(greenStationaryThread, NULL, NULL);
    osThreadNew(redMovingThread, NULL, NULL);
    osThreadNew(redStationaryThread, NULL, NULL);
    osKernelStart();   
    for (;;) {}
}
