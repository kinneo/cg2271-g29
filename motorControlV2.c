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
#define x 5 //minus speed on the respective side when doing curved turns

// 7500 = Full duty cycle (HIGH, IN1 = 1, IN2 = 0 for Forward)
// 0 = No duty cycle (LOW, IN1 = 0, IN2 = 1 for Reverse)
#define PWM_MAX 7500
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
	PORTB->PCR[0] |= PORT_PCR_MUX(3); // enable ALT3 (TPM1_CH0) on PTB0 (Left Speed)
	
	PORTB->PCR[1] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[1] |= PORT_PCR_MUX(3); // enable ALT3 (TPM1_CH1) on PTB1 (Right Speed)
	
	PORTB->PCR[2] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[2] |= PORT_PCR_MUX(3); // enable ALT3 (TPM2_CH0) on PTB2 (Left Direction)
	
	PORTB->PCR[3] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[3] |= PORT_PCR_MUX(3); // enable ALT3 (TPM2_CH1) on PTB3 (Right Direction)
		
	// set PWM period
	TPM1->MOD = 7500;
	TPM2->MOD = 7500;
	
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
	
	// initialize PWM outputs to 0
	TPM1_C0V = 0;
	TPM1_C1V = 0;
	TPM2_C0V = 0;
	TPM2_C1V = 0;
	
	// 5. Enable TPM Counter (set the cmod bits to 1 to use internal clock)
	TPM1->SC |= TPM_SC_CMOD(1); 
  TPM2->SC |= TPM_SC_CMOD(1);
	
}

void motorControl(int state, int speed){
	
	// Setting motor directions using GPIO pins.
	// Adjusting motor speed using PWM.
	
	// Convert speed (0-100) to duty cycle
	int dutyCycle = (speed * PWM_MAX) / 100; // need to change this base on wuming's angle input

	
	switch(state){
		
		case forward:

			TPM1_C0V = dutyCycle; // Left Speed
			TPM1_C1V = dutyCycle; // Right Speed
			TPM2_C0V = CW; // Left Direction
			TPM2_C1V = CCW; // Right Direction
			break;
		
		case rightTurnOnSpot:

			TPM1_C0V = dutyCycle; // Left Speed
			TPM1_C1V = dutyCycle; // Right Speed
			TPM2_C0V = CW; // Left Direction
			TPM2_C1V = CW; // Right Direction
			break;
		
		case reverse:

			TPM1_C0V = dutyCycle; // Left Speed
			TPM1_C1V = dutyCycle; // Right Speed
			TPM2_C0V = CCW; // Left Direction
			TPM2_C1V = CW; // Right Direction
			break;
			
		case leftTurnOnSpot:

			TPM1_C0V = dutyCycle; // Left Speed
			TPM1_C1V = dutyCycle; // Right Speed
			TPM2_C0V = CCW; // Left Direction
			TPM2_C1V = CCW; // Right Direction
			break;
		
		case rightTurnForward:

			TPM1_C0V = dutyCycle; // Left Speed
			TPM1_C1V = dutyCycle - x; // Right Speed
			TPM2_C0V = CW; // Left Direction
			TPM2_C1V = CCW; // Right Direction
			break;
		
		case leftTurnForward:

			TPM1_C0V = dutyCycle - x; // Left Speed
			TPM1_C1V = dutyCycle; // Right Speed
			TPM2_C0V = CW; // Left Direction
			TPM2_C1V = CCW; // Right Direction
			break;
		
		case rightTurnReverse:

			TPM1_C0V = dutyCycle; // Left Speed
			TPM1_C1V = dutyCycle - x; // Right Speed
			TPM2_C0V = CCW; // Left Direction
			TPM2_C1V = CW; // Right Direction
			break;
			
		case leftTurnReverse:

			TPM1_C0V = dutyCycle - x; // Left Speed
			TPM1_C1V = dutyCycle; // Right Speed
			TPM2_C0V = CCW; // Left Direction
			TPM2_C1V = CW; // Right Direction
			break;
		
		case brake:

			TPM1_C0V = PWM_LOW; // Left Speed
			TPM1_C1V = PWM_LOW; // Right Speed
			TPM2_C0V = PWM_LOW; // Left Direction
			TPM2_C1V = PWM_LOW; // Right Direction
			break;
		
		default:

			TPM1_C0V = PWM_LOW; // Left Speed
			TPM1_C1V = PWM_LOW; // Right Speed
			TPM2_C0V = PWM_LOW; // Left Direction
			TPM2_C1V = PWM_LOW; // Right Direction
			break;
	}
}
