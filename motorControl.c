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

// steps to initialize PWM on KL25Z
// 1. Enable clock for TPM (Timer/PWM Module)
// 2. Configure PWM Pins
// 3. Set TPM to Edge-Alighned PWM Mode
// 4. Configure PWM Duty Cycle
// 5. Enable TPM Counter

// PTB0 -> TPM1 Channel 0
// PTB1 -> TPM1 Channel 1
// PTB2 -> TPM2 Channel 0
// PTB3 -> TPM2 Channel 1

void initMotor(){
	
	// 1. Enable clock gating for TPM (Timer/PWM Module)
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK; // TPM1
	SIM->SCGC6 |= SIM_SCGC6_TPM2_MASK; // TPM2
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK; // Port B
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK; // Port D
	
	// 2. Configure PWM Pins
	PORTB->PCR[0] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[0] |= PORT_PCR_MUX(3); // enable ALT3 (TPM1_CH0) on PTB0 (FR motor speed)
	
	PORTB->PCR[1] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[1] |= PORT_PCR_MUX(3); // enable ALT3 (TPM1_CH1) on PTB1 (FL motor speed)
	
	PORTB->PCR[2] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[2] |= PORT_PCR_MUX(3); // enable ALT3 (TPM2_CH0) on PTB2 (BR motor speed)
	
	PORTB->PCR[3] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[3] |= PORT_PCR_MUX(3); // enable ALT3 (TPM2_CH1) on PTB3 (BL Motor Speed)
	
	// configure GPIO pins
	PORTD->PCR[0] = PORT_PCR_MUX(1); // PTD0 - FR Motor DIR1
	PORTD->PCR[1] = PORT_PCR_MUX(1); // PTD1 - FR Motor DIR2
	PORTD->PCR[2] = PORT_PCR_MUX(1); // PTD2 - FL Motor DIR1
	PORTD->PCR[3] = PORT_PCR_MUX(1); // PTD3 - FL Motor DIR2
	PORTD->PCR[4] = PORT_PCR_MUX(1); // PTD4 - BR Motor DIR1
	PORTD->PCR[5] = PORT_PCR_MUX(1); // PTD5 - BR Motor DIR2
	PORTD->PCR[6] = PORT_PCR_MUX(1); // PTD6 - BL Motor DIR1
	PORTD->PCR[7] = PORT_PCR_MUX(1); // PTD7 - BL Motor DIR2
	
	// Set GPIO Direction Pins as OUTPUTS
	PTD->PDDR |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);
		
	// set PWM period
	TPM1->MOD = 7500;
	TPM2->MOD = 7500;
		
	// clear the CMOD bits to diable the timer, and clear the prescalar bits
	//TPM1->SC &= ~TPM_SC_CMOD_MASK;
	TPM1->SC &= ~TPM_SC_PS_MASK;
	
	//TPM2->SC &= ~TPM_SC_CMOD_MASK;
	TPM2->SC &= ~TPM_SC_PS_MASK;
	
	// set prescalar to 32
	TPM1->SC |=  TPM_SC_PS(5);
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
	
	// If IN1 = HIGH, IN2 = LOW --> Motor moves forward.
	// If IN1 = LOW, IN2 = HIGH --> Motor moves backward.
	
	// PSOR -> set to high aka 1
	// PCOR -> set to low aka 0
	
	
	// PTD0 - FR Motor DIR1
	// PTD1 - FR Motor DIR2 
	// PTD2 - FL Motor DIR1 
	// PTD3 - FL Motor DIR2
	// PTD4 - BR Motor DIR1
	// PTD5 - BR Motor DIR2 
	// PTD6 - BL Motor DIR1 
	// PTD7 - BL Motor DIR2
	
	// DIR1 == 1 for forward, DIR2 == 1 for reverse
	
	// Convert speed (0-100) to duty cycle
	int dutyCycle = (speed * 7500) / 100;
	
	switch(state){
		case forward:
			// Set direction to forward (DIR1 = HIGH, DIR2 = LOW)
			PTD->PSOR = (1 << 0) | (1 << 2) | (1 << 4) | (1 << 6); // DIR1 HIGH
			PTD->PCOR = (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7); // DIR2 LOW
			// Apply speed (PWM)
			TPM1_C0V = dutyCycle; // FR Motor
			TPM1_C1V = dutyCycle; // FL Motor
			TPM2_C0V = dutyCycle; // BR Motor
			TPM2_C1V = dutyCycle; // BL Motor
			break;
		case rightTurnOnSpot:
			
			// right motors shld be in reverse aka DIR2, left motors shld be in forward aka DIR1
			PTD->PSOR = (1 << 1) | (1 << 2) | (1 << 5) | (1 << 6);
			PTD->PCOR = (1 << 0) | (1 << 3) | (1 << 4) | (1 << 7); 
			// Apply speed (PWM)
			TPM1_C0V = dutyCycle; // FR Motor
			TPM1_C1V = dutyCycle; // FL Motor
			TPM2_C0V = dutyCycle; // BR Motor
			TPM2_C1V = dutyCycle; // BL Motor
			break;
		case reverse:
			// Set direction to reverse (DIR1 = LOW, DIR2 = HIGH)
			PTD->PCOR = (1 << 0) | (1 << 2) | (1 << 4) | (1 << 6); // DIR1 LOW
			PTD->PSOR = (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7); // DIR2 HIGH

			// Apply speed (PWM)
			TPM1_C0V = dutyCycle; // FR Motor
			TPM1_C1V = dutyCycle; // FL Motor
			TPM2_C0V = dutyCycle; // BR Motor
			TPM2_C1V = dutyCycle; // BL Motor
			break;
			
		case leftTurnOnSpot:
			// left motors shld be in reverse aka DIR2, right motors shld be in forward aka DIR1
			PTD->PSOR = (1 << 0) | (1 << 3) | (1 << 4) | (1 << 7);
			PTD->PCOR = (1 << 1) | (1 << 2) | (1 << 5) | (1 << 6); 
			// Apply speed (PWM)
			TPM1_C0V = dutyCycle; // FR Motor
			TPM1_C1V = dutyCycle; // FL Motor
			TPM2_C0V = dutyCycle; // BR Motor
			TPM2_C1V = dutyCycle; // BL Motor
			break;
		case rightTurnForward:
			// since still moving forward GPIO shld be same as forward
			PTD->PSOR = (1 << 0) | (1 << 2) | (1 << 4) | (1 << 6); // DIR1 HIGH
			PTD->PCOR = (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7); // DIR2 LOW
			// Apply speed (PWM)
			TPM1_C0V = dutyCycle - x; // FR Motor
			TPM1_C1V = dutyCycle; // FL Motor
			TPM2_C0V = dutyCycle - x; // BR Motor
			TPM2_C1V = dutyCycle; // BL Motor
			break;
		case leftTurnForward:
			// since still moving forward GPIO shld be same as forward
			PTD->PSOR = (1 << 0) | (1 << 2) | (1 << 4) | (1 << 6); // DIR1 HIGH
			PTD->PCOR = (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7); // DIR2 LOW
			// Apply speed (PWM)
			TPM1_C0V = dutyCycle; // FR Motor
			TPM1_C1V = dutyCycle - x; // FL Motor
			TPM2_C0V = dutyCycle; // BR Motor
			TPM2_C1V = dutyCycle - x; // BL Motor
			break;
		case rightTurnReverse:
			// since still moving in reverse direction, GPIO shld be same as reverse
			PTD->PCOR = (1 << 0) | (1 << 2) | (1 << 4) | (1 << 6); // DIR1 LOW
			PTD->PSOR = (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7); // DIR2 HIGH
			// Apply speed (PWM)
			TPM1_C0V = dutyCycle - x; // FR Motor
			TPM1_C1V = dutyCycle; // FL Motor
			TPM2_C0V = dutyCycle - x; // BR Motor
			TPM2_C1V = dutyCycle; // BL Motor
			break;
			
		case leftTurnReverse:
			// since still moving in reverse direction, GPIO shld be same as reverse
			PTD->PCOR = (1 << 0) | (1 << 2) | (1 << 4) | (1 << 6); // DIR1 LOW
			PTD->PSOR = (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7); // DIR2 HIGH
			// Apply speed (PWM)
			TPM1_C0V = dutyCycle; // FR Motor
			TPM1_C1V = dutyCycle - x; // FL Motor
			TPM2_C0V = dutyCycle; // BR Motor
			TPM2_C1V = dutyCycle - x; // BL Motor
			break;
		case brake:
			// everything set to 0
			PTD->PCOR = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) |
									(1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);
			// Apply speed (PWM)
			TPM1_C0V = 0; // FR Motor
			TPM1_C1V = 0; // FL Motor
			TPM2_C0V = 0; // BR Motor
			TPM2_C1V = 0; // BL Motor
			break;
		default:
			// everything set to 0
			PTD->PCOR = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) |
									(1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);
			// Apply speed (PWM)
			TPM1_C0V = 0; // FR Motor
			TPM1_C1V = 0; // FL Motor
			TPM2_C0V = 0; // BR Motor
			TPM2_C1V = 0; // BL Motor
			break;
			
	}
}
