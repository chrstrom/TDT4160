#include "o3.h"
#include "gpio.h"
#include "systick.h"

// MASKS
#define EXTIPSEL_Msk 0b0001
#define EXTIFALL_Msk 0b0001

// PORTS
// Alternatively you can use the port_pin_t struct for this
#define LED0_PORT GPIO_PORT_E
#define LED1_PORT GPIO_PORT_E

#define PB0_PORT GPIO_PORT_B
#define PB1_PORT GPIO_PORT_B

// PINS
#define LED0_PIN 2
#define LED1_PIN 3

#define PB0_PIN 9
#define PB1_PIN 10

// TIME
#define SEC_MAX 59
#define MIN_MAX 59
#define HR_MAX 99

#define S_IN_HR 3600
#define S_IN_MIN 60

// GLOBALS
gpio_t* GPIO = (gpio_t*) GPIO_BASE;					
systick_t* SYSTICK = (systick_t*) SYSTICK_BASE;

	// CLOCK
enum state{SEC, MIN, HR, COUNTDOWN, ALARM};
int clock_state = SEC;

	// TIME
int total_sec = 0;
char time_str[8];

/****************** UTILITY ******************/
// Convert a number between 0 and 99 to string
void int_to_string(char *timestamp, unsigned int offset, int i) {
    if (i > 99) {
        timestamp[offset]   = '9';
        timestamp[offset+1] = '9';
        return;
    }

    while (i > 0) {
	    if (i >= 10) {
		    i -= 10;
		    timestamp[offset]++;

	    } else {
		    timestamp[offset+1] = '0' + i;
		    i=0;
	    }
    }
}

// Convert three numbers to a timestamp. the timestamp argument is an array with at least 7 elements
void time_to_string(char *timestamp, int h, int m, int s) {
    timestamp[0] = '0';
    timestamp[1] = '0';
    timestamp[2] = '0';
    timestamp[3] = '0';
    timestamp[4] = '0';
    timestamp[5] = '0';
    timestamp[6] = '\0';

    int_to_string(timestamp, 0, h);
    int_to_string(timestamp, 2, m);
    int_to_string(timestamp, 4, s);
}

// set four bits in a selected memory location given by w
void set_bits(volatile word* w, int pin, int fl){
	pin *= 4;	// Mutliply by 4 to get the correct leftshift, because each "pin" is a 4 bit wide slot in memory
	*w &= ~ (0b1111 << pin);
	*w |= (fl << pin);
}

// Set single bit at pin location to s (s can either be 1 or 0)
void set_pin(volatile word* w, int pin, int s){
	if(s == 0 || s == 1){
		*w |= s << pin;
	}
}

// Convert seconds -> HH:MM:SS and display
void display_time(int sec){
	int h = sec / S_IN_HR;
	int m = (sec / S_IN_MIN) % 60;
	int s = sec % 60;

	time_to_string(time_str, h, m, s);
	lcd_write(time_str);
}

void start_clock(void){
	SYSTICK->CTRL |= SysTick_CTRL_ENABLE_Msk;		// Enable SysTick to start sending interrupts
}

void stop_clock(void){
	SYSTICK->CTRL &= ~SysTick_CTRL_ENABLE_Msk;		// Disable SysTick from sending interrupts
	clock_state = ALARM;
	set_pin(&GPIO->ports[LED0_PORT].DOUTTGL, LED0_PIN, 1);	// Turning on LED0 is our way of showing that the alarm went off
}

/************* INTERRUPT HANDLERS *************/
void GPIO_ODD_IRQHandler(void){
	switch(clock_state){
	case SEC:
		total_sec++;
		display_time(total_sec);
		break;
	case MIN:
		total_sec += S_IN_MIN;
		display_time(total_sec);
		break;
	case HR:
		total_sec += S_IN_HR;
		display_time(total_sec);
		break;
	case COUNTDOWN:
		break;
	case ALARM:
		break;
	default:
		set_pin(&GPIO->ports[LED1_PORT].DOUTTGL, LED1_PIN, 1);		// Turn on LED1 if something has gone wrong (we should never get to this point of the code)
	}

	set_pin(&GPIO->IFC, PB0_PIN, 1);			// Reset IFC (interrupt handled)
}

void GPIO_EVEN_IRQHandler(void){
	switch(clock_state){
	case SEC:
		clock_state = MIN;
		break;
	case MIN:
		clock_state = HR;
		break;
	case HR:
		clock_state = COUNTDOWN;
		start_clock();
		break;
	case COUNTDOWN:

		break;
	case ALARM:
		clock_state = SEC;
		set_pin(&GPIO->ports[LED0_PORT].DOUTCLR, LED0_PIN, 1);
		set_pin(&GPIO->ports[LED1_PORT].DOUTCLR, LED1_PIN, 1);
		break;
	default:
		set_pin(&GPIO->ports[LED1_PORT].DOUTTGL, LED1_PIN, 1);		// Turn on LED1 if something has gone wrong (we should never get to this point of the code)
	}
	
	set_pin(&GPIO->IFC, PB1_PIN, 1);			// Reset IFC (interrupt handled)
}

// Because SysTick sends an interrupt for every second, we can directly count down here
void SysTick_Handler(void){
	total_sec--;
	display_time(total_sec);
	if(!total_sec) { stop_clock(); }
}

/*********** SETUP FOR GPIO/SysTick **********/
void init_clock(void){
	// Set inputs and outputs
	// Subtract 8 from PB0 and PB1 because we are in MODEH (8-15)  pin # is relative to MODE (0-15)
	set_bits(&GPIO->ports[LED0_PORT].MODEL, LED0_PIN, GPIO_MODE_OUTPUT);
	set_bits(&GPIO->ports[LED1_PORT].MODEL, LED1_PIN, GPIO_MODE_OUTPUT);
	set_bits(&GPIO->ports[PB0_PORT].MODEH, PB0_PIN-8, GPIO_MODE_INPUT);
	set_bits(&GPIO->ports[PB1_PORT].MODEH, PB1_PIN-8, GPIO_MODE_INPUT);

	// Set pin09 and pin01 -> 0b0001 in EXTIPSELH
	// Same thing here, we are in EXTIPSHELH (8-15), while pin # is relative to EXTIPSEL (0-15)
	set_bits(&GPIO->EXTIPSHELH, PB0_PIN-8, EXTIPSEL_Msk);
	set_bits(&GPIO->EXTIPSHELH, PB1_PIN-8, EXTIPSEL_Msk);

	// Set EXTIFALL (External interrupt falling edge trigger)
	set_pin(&GPIO->EXTIFALL, PB0_PIN, 1);
	set_pin(&GPIO->EXTIFALL, PB1_PIN, 1);

	// Set IF (Interrupt flag) to 0 on startup
	set_pin(&GPIO->IF, PB0_PIN, 0);
	set_pin(&GPIO->IF, PB1_PIN, 0);

	// Set IEN (Interrupt enable)
	set_pin(&GPIO->IEN, PB0_PIN, 1);
	set_pin(&GPIO->IEN, PB1_PIN, 1);

	// Setup for SysTick
	SYSTICK->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;	//(TICKINT, CLKSOURCE,ENABLE) -> (1, 1, 0)
	SYSTICK->LOAD = FREQUENCY;						// The systick interrupt will now generate one signal per second
	SYSTICK->VAL = 0;							// VAL can either be 0 or LOAD

	// Show initial value for total_sec on startup. Should be 0
	display_time(total_sec);
}

/******************** MAIN ********************/
int main(void) {
    init();
    init_clock();

    while(1){}	// Wait for interrupts
    return 0;
}
