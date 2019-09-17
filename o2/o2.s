.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO
.include "sys-tick_constants.s" // Register-adresser og konstanter for SysTick

.text
	.global Start

Start:
	// To set up the interrupt for PB0, we need to configure EXTIPSEL, EXTIFALL, IF and IEN

	// Setting EXTIPSEL (External interrupt port select) to set interrupt location
	LDR R0, =GPIO_BASE  + GPIO_EXTIPSELH					// MEMLOC: GPIO->EXTIPSEL->H
	MOV R1, #0b1111 << 4
	MVN R1, R1												// Loading 1's then inverting ensures that we are only nulling the group of interest
	LDR R2, [R0]
	AND R3, R1, R2											// Mask AND Reading from EXTIPSELH (resetting group for pin9)
	MOV R4, #PORT_B << 4									// Port B is 0b0001, and shifted into position sets up ORR
	ORR R4, R3, R4											// Reset EXTIPSELH  ORR  0b0001 sets 0b0001 correctly
	STR R4, [R0]											// Write the value to EXTIPSELH

	// Setting EXTIFALL (External interrupt falling edge trigger)
	// We want the interrupt to send on PB0 press, and since it is active high, we need to
	// send the interrupt at falling edge
	LDR R0, =GPIO_BASE + GPIO_EXTIFALL						// MEMLOC: GPIO->EXTIFALL
	MOV R1, #1 << BUTTON_PIN								// Mask to set PB0 pin to 1
	LDR R2, [R0]											// Get current values from EXTIFALL
	ORR R1, R1, R2											// Inserting 1 in PB0-bit of EXTIFALL
	STR R1, [R0]											// Insert back ito EXTIFALL

	// Setting IF (Interrupt flag) to 0 on startup
	LDR R0, =GPIO_BASE + GPIO_IFC							// MEMLOC: GPIO->IF->C
	MOV R1, #0 << BUTTON_PIN								// Mask to set PB0 pin to 1
	LDR R2, [R0]											// Get current values from IFC
	ORR R1, R1, R2											// Inserting 1 in PB0-bit of IFC
	STR R1, [R0]											// Insert back ito IFC

	// Setting IEN (Interrupt enable)
	LDR R0, =GPIO_BASE + GPIO_IEN
	MOV R1, #1 << BUTTON_PIN								// Mask to set PB0 pin to 1
	LDR R2, [R0]											// Get current values from IEN
	ORR R1, R1, R2											// Inserting 1 in PB0-bit of IEN
	STR R1, [R0]											// Insert back into IEN

	// Setup for SysTick
	// Setting up CTRL (Control and status register)
	LDR R0, =SYSTICK_BASE + SYSTICK_CTRL					// MEMLOC: SYSTICK->CTRL
	LDR R1, =SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk 		// (TICKINT, CLKSOURCE,ENABLE) -> (1, 1, 0)
	STR R1, [R0]

	// Setting up LOAD (Reload value register)
	LDR R0, =SYSTICK_BASE + SYSTICK_LOAD
	LDR R1, =FREQUENCY / 10									// This gives us 10 interrupt signals per second
	STR R1, [R0]

	// Setting up VAL (Current value register)
	LDR R0, =SYSTICK_BASE + SYSTICK_VAL
	MOV R1, #0
	STR R1, [R0]

Loop:
	WFI 													// Wait for interrupt
	B Loop

.global SysTick_Handler										// Overriding the base interrupt vector for SysTick
.thumb_func
SysTick_Handler:
	// Update tenths										// The logic flow described below is similar for seconds/minutes too
	LDR R0, =tenths
	LDR R1, [R0]											// Load current tenths
	ADD R1, #1												// Increment by 1
	CMP R1, #10
	BNE Return												// Branch if tenths != 10
		MOV R1, #0											// If we don't branch, we roll tenths back over to 0

		// We reach this block of code every second, so we can toggle LED0 here
		LDR R6, =GPIO_BASE + (LED_PORT * PORT_SIZE) + GPIO_PORT_DOUTTGL
		MOV R7, #1<<LED_PIN
		STR R7, [R6]

		// Update seconds
		LDR R2, =seconds
		LDR R3, [R2]
		ADD R3, #1
		CMP R3, #60
		BNE Return
			MOV R3, #0

			// Update minutes
			LDR R4, =minutes
			LDR R5, [R4]
			ADD R5, #1
			CMP R5, #60
			BNE Return
				MOV R5, #0

	Return:
		// Set values for tenths/seconds/minutes in their respective registers before returning from interrupt
		STR R1, [R0]
		STR R3, [R2]
		STR R5, [R4]
		BX LR

.global GPIO_ODD_IRQHandler									// Since PB0 is bit 9 (odd) of GPIO_PORT_B_DIN, we need to use the ODD IRQHandler
.thumb_func
GPIO_ODD_IRQHandler:
	// Reset IFC
	LDR R0, =GPIO_BASE + GPIO_IFC
	MOV R1, #1 << BUTTON_PIN
	STR R1, [R0]

	// Turn clock on/off
	LDR R0, =SYSTICK_BASE + SYSTICK_CTRL
	LDR R1, [R0]
	EOR R1, #SysTick_CTRL_ENABLE_Msk						// Flips the ENABLE bit but keeps everything else the same
	STR R1, [R0]

	BX LR													// Return from interrupt

NOP // Behold denne på bunnen av fila
