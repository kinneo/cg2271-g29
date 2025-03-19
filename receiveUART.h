#include "MKL25Z4.h" 
#include "circularQueue.h"

#define MASK(x) (1 << (x))

#define BUS_CLOCK 32000000
#define PACKET_SIZE 1  // Each packet is 8 bits (1 byte)

volatile uint8_t packet_buffer[PACKET_SIZE];  // Buffer to store received data
volatile uint8_t packet_index = 0;  // Tracks the current byte received in the packet
volatile Queue_t RxQ;

void Init_UART2(uint32_t baud_rate) {
    uint32_t divisor;

    // Enable clock to UART and Port E
    SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

    // Connect UART to pins for PTE22, PTE23
    PORTE->PCR[22] = PORT_PCR_MUX(4);
    PORTE->PCR[23] = PORT_PCR_MUX(4);

    // Ensure tx and rx are disabled before configuration
    UART2->C2 &=  ~(UARTLP_C2_TE_MASK | UARTLP_C2_RE_MASK);

    // Set baud rate to 4800 baud
    divisor = BUS_CLOCK/(baud_rate*16);
    UART2->BDH = UART_BDH_SBR(divisor>>8);
    UART2->BDL = UART_BDL_SBR(divisor);

    // 8N1 data frame config
    UART2->C1 = UART2->S2 = UART2->C3 = 0;

    // Enable receiver
    UART2->C2 = UART_C2_RE_MASK; 

    NVIC_SetPriority(UART2_IRQn, 128); // 0 (0), 64 (1), 128 (2)...
    NVIC_ClearPendingIRQ(UART2_IRQn); 
    NVIC_EnableIRQ(UART2_IRQn);

    UART2->C2 |= UART_C2_RIE_MASK; // Enables the receive interrupt.

    Q_Init(&RxQ); // Receive Queue
 }


 void UART2_IRQHandler(void) {
    NVIC_ClearPendingIRQ(UART2_IRQn);

    if (UART2->S1 & UART_S1_RDRF_MASK) {  // If received data register is full
        uint8_t received_byte = UART2->D;  // Read received byte

        // Store byte in packet buffer
        packet_buffer[packet_index++] = received_byte;

        // If the packet is fully received (8 bits in this case)
        if (packet_index >= PACKET_SIZE) {
            if (!Q_Full(&RxQ)) {  // Check if the queue has space
                Q_Enqueue(&RxQ, packet_buffer[0]);  // Store the received byte
                packet_index = 0;  // Reset packet buffer index for the next packet
            } else {
                // Error handling - queue full, drop data or take necessary action
                packet_index = 0;  // Reset to avoid buffer overflows
            }
        }
    }
}
