/*
* Doug S. Szumski <d.s.szumski@gmail.com> 19-10-2012
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


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

