#include <MKL25Z4.h>
#include "circularQueue.h"

#define MASK(x) (1 << (x))

#define PACKET_SIZE 1  // Each packet is 8 bits (1 byte)

volatile Queue_t RxQ;
volatile uint8_t received_byte = 0;
volatile int received = 0;
volatile int triggered = 0;

void Init_UART2(uint32_t baud_rate) {
    uint32_t divisor;

    // Enable clock to UART and Port E
    SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

    // Connect UART to pins for PTE23
		PORTE->PCR[23] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[23] |= PORT_PCR_MUX(4);
	
		PORTE->PCR[22] = PORT_PCR_MUX(4);
		UART2->C2 |= UART_C2_TE_MASK; // For TX



    // Ensure tx and rx are disabled before configuration
    UART2->C2 &=  ~(UARTLP_C2_TE_MASK | UARTLP_C2_RE_MASK);
		
		uint32_t bus_clock = DEFAULT_SYSTEM_CLOCK / 2;
    divisor = bus_clock/(baud_rate*16);
    UART2->BDH = UART_BDH_SBR(divisor>>8);
    UART2->BDL = UART_BDL_SBR(divisor);

    // 8N1 data frame config
    UART2->C1 = 0;
		UART2->S2 = 0;
		UART2->C3 = 0;

    // Enable receiver
    UART2->C2 |= UART_C2_RE_MASK;


    NVIC_SetPriority(UART2_IRQn, 128); // 0 (0), 64 (1), 128 (2)...
    NVIC_ClearPendingIRQ(UART2_IRQn); 
    NVIC_EnableIRQ(UART2_IRQn);

    UART2->C2 |= UART_C2_RIE_MASK; // Enables the receive interrupt.
    Q_Init(&RxQ); // Receive Queue
 }


void UART2_IRQHandler(void) {
	NVIC_ClearPendingIRQ(UART2_IRQn);
	if (UART2->S1 & UART_S1_RDRF_MASK) {
		received = 1;
		received_byte = UART2->D;
	}
}
		
