#ifndef GPIO_H
#define GPIO_H

// Pin-datatype
typedef struct{
	unsigned int   port;
	unsigned int   pin;
} port_pin_t;

// GPIO pin modes
#define GPIO_MODE_INPUT  0b0001
#define GPIO_MODE_OUTPUT 0b0100

// GPIO-adresse
#define GPIO_BASE 0x40006000

// GPIO port-nummere
#define GPIO_PORT_A 0
#define GPIO_PORT_B 1
#define GPIO_PORT_C 2
#define GPIO_PORT_D 3
#define GPIO_PORT_E 4
#define GPIO_PORT_F 5

typedef volatile struct {
	word CTRL;
	word MODEL;
	word MODEH;
	word DOUT;
	word DOUTSET;
	word DOUTCLR;
	word DOUTTGL;
	word DIN;
	word PINLOCKN;
} gpio_port_t;

typedef volatile struct {
	gpio_port_t ports[6];
	word unused[10];
	word EXTIPSHELL;
	word EXTIPSHELH;
	word EXTIRISE;
	word EXTIFALL;
	word IEN;
	word IF;
	word IFS;
	word IFC;
	word ROUTE;
	word INSENSE;
	word LOCK;
	word CTRL;
	word CMD;
	word EM4WUEN;
	word EM4WUPOL;
	word EM4WUCAUSE;
} gpio_t;

// If the pointer to gpio_t is pointing as the address GPIO_BASE,
// every subsequent value in gpio_port_t and gpio_t will line up
// with the EFM32GG addresses

#endif
