#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/io.h>

/* CPU frequency */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* UART baud */
#define UART_BAUD_RATE      57600      

//Setup ports 
void configPorts(void);

//16 bit CTC counter
void init_OC1A_CTC(void) ;

//Config PWM 
void init_OC1A(void);
void init_OC1B(void);
void init_OC2(void);

//Configure pin change interrupts
void init_INT0(void); //Pin D2
void init_INT1(void); //Pin D2

