/* En rask m책te 책 unng책 header recursion p책 er 책 sjekke om verdi, f.eks. 'O3_H',
   er definert. Hvis ikke, definer 'O3_H' og deretter innholdet av headeren 
   (merk endif p책 bunnen). N책 kan headeren inkluderes s책 mange ganger vi vil 
   uten at det blir noen problemer. */
#ifndef O3_H
#define O3_H

// Type-definisjoner fra std-bibliotekene
#include <stdint.h>
#include <stdbool.h>

// Type-aliaser
typedef uint32_t word;
typedef uint8_t  byte;

// Prototyper for bibliotekfunksjoner
void init(void);
void lcd_write(char* string);
void int_to_string(char *timestamp, unsigned int offset, int i);
void time_to_string(char *timestamp, int h, int m, int s);

// Prototyper
// legg prototyper for dine funksjoner her�
void init_clock(void);
void display_time(int seconds);
void decrement_time(void);
void start_clock(void);
void stop_clock(void);


void set_bits(volatile word* w, int pin, int fl);
void set_pin(volatile word* w, int pin, int s);

void GPIO_ODD_IRQHandler(void);
void GPIO_EVEN_IRQHandler(void);
void SysTick_Handler(void);

#endif
