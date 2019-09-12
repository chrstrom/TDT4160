.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO

.text
	.global Start

Start:
    	LDR R0, =GPIO_BASE + (PORT_SIZE * BUTTON_PORT) + GPIO_PORT_DIN				// MEMLOC: GPIO->PORT_B->DIN
	LDR R1, =GPIO_BASE + (PORT_SIZE * LED_PORT)    + GPIO_PORT_DOUTCLR			// MEMLOC: GPIO->PORT_E->DOUTCLR
	LDR R2, =GPIO_BASE + (PORT_SIZE * LED_PORT)    + GPIO_PORT_DOUTSET			// MEMLOC: GPIO->PORT_E->DOUTSET

Loop:
	LDR R3, [R0] 				// Polling PB0
	LSR R3, R3, #7				// BUTTON_PIN - LED_PIN = 7
	AND R3, R3, #4				// Mask to clean up R3
	STR R3, [R1]				// Set DOUTCLR
	EOR R3, R3, #4				// Flip LED_PIN bit
	STR R3, [R2]				// Set DOUTSET
	B Loop					// Loop without breakout condition
NOP
