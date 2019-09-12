.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO

.text
	.global Start

Start:
	LDR R0, =GPIO_BASE + (PORT_SIZE * LED_PORT)    + GPIO_PORT_DOUTSET			// MEMLOC: GPIO->PORT_E->DOUTSET
	LDR R1, =GPIO_BASE + (PORT_SIZE * LED_PORT)    + GPIO_PORT_DOUTCLR			// MEMLOC: GPIO->PORT_E->DOUTCLR
	LDR R2, =GPIO_BASE + (PORT_SIZE * BUTTON_PORT) + GPIO_PORT_DIN				// MEMLOC: GPIO->PORT_B->DIN

	MOV R3, #1 << LED_PIN			// Set 1 at the LED_PIN location. This gets set in either DOUTSET or DOUTCLR
	MOV R4, #1 << BUTTON_PIN		// Set 1 at the BUTTON_PIN location. Used as a mask to filter PB0 input
Loop:
	LDR R5, [R2]		// Poll PB0
	AND R5, R5, R4		// Filters out any input that is not pin 9 (PB0 state)
	CMP R5, R4			// R5 contains the data from PB0, compare it to R4
	BEQ On				// If equal (PB0 is pressed) we jump to turn on the lamp, otherwise, turn off
Off:
	STR R3, [R0]		// [R0] is the memory address for GPIO->PORT_E->DOUTSET
	B Loop
On:						// To turn the lamp on, we set
	STR R3, [R1]		// [R1] is the memory address for GPIO->PORT_E->DOUTCLR
	B Loop
NOP // Keep at the end of the file
