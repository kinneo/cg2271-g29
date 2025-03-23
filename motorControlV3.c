#include <MKL25Z4.h>

#define forward 0
#define rightTurnOnSpot 1
#define reverse 2
#define leftTurnOnSpot 3
#define rightTurnForward 4
#define leftTurnForward 5
#define rightTurnReverse 6
#define leftTurnReverse 7
#define brake 8

#define PWM_MAX 4999
#define PWM_HIGH PWM_MAX
#define PWM_LOW 0

#define CW PWM_HIGH
#define CCW PWM_LOW


// steps to initialize PWM on KL25Z
// 1. Enable clock for TPM (Timer/PWM Module)
// 2. Configure PWM Pins
// 3. Set TPM to Edge-Alighned PWM Mode
// 4. Configure PWM Duty Cycle
// 5. Enable TPM Counter

// PTB0 -> TPM1 Channel 0 -> Left Motor Speed (FL & BL)
// PTB1 -> TPM1 Channel 1 -> Right Motor Speed (FR & BR)
// PTB2 -> TPM2 Channel 0 -> Left Motor Direction (IN1 & IN2)
// PTB3 -> TPM2 Channel 1 -> Right Motor Direction (IN1 & IN2)

void initMotor(){
	
	// 1. Enable clock gating for TPM (Timer/PWM Module)
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK; // TPM1 for speed
	SIM->SCGC6 |= SIM_SCGC6_TPM2_MASK; // TPM2 for direction
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK; // Port B for PWM
	// SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK; // Port D
	
	// 2. Configure PWM Pins
	PORTB->PCR[0] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[0] = PORT_PCR_MUX(3); // enable ALT3 (TPM1_CH0) on PTB0 (Left Speed)
	
	PORTB->PCR[1] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[1] = PORT_PCR_MUX(3); // enable ALT3 (TPM1_CH1) on PTB1 (Right Speed)
	
	PORTB->PCR[2] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[2] = PORT_PCR_MUX(3); // enable ALT3 (TPM2_CH0) on PTB2 (Left Direction)
	
	PORTB->PCR[3] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[3] = PORT_PCR_MUX(3); // enable ALT3 (TPM2_CH1) on PTB3 (Right Direction)
		
	// set TPM clock source (i dont exactly know what this does)
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
  SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
	
	// set PWM period
	TPM1->MOD = PWM_MAX;
	TPM2->MOD = PWM_MAX;
	
	// clear CMOD and set CMOD
	TPM1->SC &= ~TPM_SC_CMOD_MASK;
	TPM1->SC |= TPM_SC_CMOD(1);
	TPM2->SC &= ~TPM_SC_CMOD_MASK;
	TPM2->SC |= TPM_SC_CMOD(1);
	
	
	// clear prescaler and set prescalar to 32
	TPM1->SC &= ~TPM_SC_PS_MASK;
	TPM1->SC |=  TPM_SC_PS(5);
	TPM2->SC &= ~TPM_SC_PS_MASK;
	TPM2->SC |=  TPM_SC_PS(5);
	
	// Step 3. Set TPM to Edge-Alighned PWM Mode
	TPM1->SC &= ~TPM_SC_CPWMS_MASK;
	TPM2->SC &= ~TPM_SC_CPWMS_MASK;
	
	// 4. Configure PWM Duty Cycle
	
	// clear all the ELSB ELSA MSB MSA bits
	TPM1_C0SC &= ~(TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA_MASK | TPM_CnSC_MSB_MASK | TPM_CnSC_MSA_MASK );
	TPM2_C0SC &= ~(TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA_MASK | TPM_CnSC_MSB_MASK | TPM_CnSC_MSA_MASK );
	TPM1_C1SC &= ~(TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA_MASK | TPM_CnSC_MSB_MASK | TPM_CnSC_MSA_MASK );
	TPM2_C1SC &= ~(TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA_MASK | TPM_CnSC_MSB_MASK | TPM_CnSC_MSA_MASK );
	
	// set ELSB and MSB to 1
	TPM1_C0SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
	TPM2_C0SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
	TPM1_C1SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
	TPM2_C1SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
	

	
	// 5. Enable TPM Counter (set the cmod bits to 1 to use internal clock)
	TPM1->SC |= TPM_SC_CMOD(1); 
  TPM2->SC |= TPM_SC_CMOD(1);
	
}


// Function to determine turn factor based on 2-bit turn angle input
// this is only for curved turns, for 90 degree and 0 degree, it shld go to the respective forward and turn on spot code
float getTurnFactor(uint8_t turnBits) {
    switch(turnBits) {
			// note: 0.5 is the lowest, any lower the wheel won't turn
        case 0b00: return 1.0;  // either 0 degrees or 90 degrees aka no change
        case 0b01: return 0.5;  // 22.5 degrees
        case 0b10: return 0.65;  // 45 degrees
        case 0b11: return 0.8;  // 67.5 degrees
        default: return 1.0;  // Default case (should not happen)
    }
}

void motorControl(int state, int speed, uint8_t turnBits){

	// Convert speed (0-100) to duty cycle
	if (speed < 0) speed = 0;
	if (speed > 100) speed = 100;
	int dutyCycle = (speed * PWM_MAX) / 100; // need to change this base on wuming's angle input

	float turnFactor = getTurnFactor(turnBits);
	
	switch(state){
		
		// motor rotate forward (CW) = 01, motor rotate reverse (CCW) = 10, stop = 00
		// TPM1 is for both left motors, TPM2 is for both right motors
		
		case forward:
			// left motors = 01, right motors = 01
			TPM1_C0V = 0; 
			TPM1_C1V = dutyCycle; 
			TPM2_C0V = 0; 
			TPM2_C1V = dutyCycle; 
			break;
		
		case rightTurnOnSpot:
			// left motors = 01, right motors = 10
			TPM1_C0V = 0; 
			TPM1_C1V = dutyCycle * turnFactor; 
			TPM2_C0V = dutyCycle  * turnFactor; 
			TPM2_C1V = 0; 
			break;
		
		case reverse:
			// left motors = 10, right motors = 10
			TPM1_C0V = dutyCycle; 
			TPM1_C1V = 0; 
			TPM2_C0V = dutyCycle; 
			TPM2_C1V = 0; 
			break;
			
		case leftTurnOnSpot:
			// left motors = 10, right motors = 01
			TPM1_C0V = dutyCycle  * turnFactor;
			TPM1_C1V = 0; 
			TPM2_C0V = 0; 
			TPM2_C1V = dutyCycle  * turnFactor; 
			break;
		
		case rightTurnForward:
			// left motors = 01, right motors = 01
			TPM1_C0V = 0; 
			TPM1_C1V = dutyCycle; 
			TPM2_C0V = 0; 
			TPM2_C1V = dutyCycle * turnFactor; 
			break;
		
		case leftTurnForward:
			// left motors = 01, right motors = 01
			TPM1_C0V = 0; 
			TPM1_C1V = dutyCycle * turnFactor; 
			TPM2_C0V = 0; 
			TPM2_C1V = dutyCycle; 
			break;
		
		case rightTurnReverse:
			// left motors = 10, right motors = 10
			TPM1_C0V = dutyCycle; 
			TPM1_C1V = 0; 
			TPM2_C0V = dutyCycle * turnFactor; 
			TPM2_C1V = 0;
			break;
			
		case leftTurnReverse:
			// left motors = 10, right motors = 10
			TPM1_C0V = dutyCycle * turnFactor; 
			TPM1_C1V = 0; 
			TPM2_C0V = dutyCycle; 
			TPM2_C1V = 0; 
			break;
		
		case brake:
			// both left right 00
			TPM1_C0V = 0; 
			TPM1_C1V = 0;
			TPM2_C0V = 0; 
			TPM2_C1V = 0; 
			break;
		
		default:
			// both left right 00
			TPM1_C0V = 0; 
			TPM1_C1V = 0;
			TPM2_C0V = 0; 
			TPM2_C1V = 0; 
			break;
	}
}


int main (void) {
     initMotor();

    while(1){
			//motorControl(forward, 80, 0b00); 
			//motorControl(rightTurnOnSpot, 80, 0b00); 
			//motorControl(reverse, 80, 0b00); 
			//motorControl(leftTurnOnSpot, 80, 0b00);
			//motorControl(rightTurnForward, 80, 0b10); 
			motorControl(leftTurnForward, 80, 0b10); 
			//motorControl(rightTurnReverse, 80, 0b00); 
			//motorControl(leftTurnReverse, 80, 0b00); 
			//motorControl(brake, 80, 0b00); 
		};
}
