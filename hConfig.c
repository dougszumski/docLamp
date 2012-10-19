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

#include "hConfig.h"


void configPorts(void)
{
    //Set all pins to inputs
    DDRB = 0x00;
    DDRB |= ((1<<DDB0) | (1<<DDB1) | (1<<DDB2) | (1<<DDB3)); //Red LED, PWM channels
    DDRC = 0x00;
    DDRD = 0x00;

    //Pulls ups on all ports
    PORTB = 0x00;
    PORTB = ((1<<PB0) | (0<<PB1) | (0<<PB2) | (0<<PB3)); //Turn LED off
    PORTC = 0xFF;
    PORTD = 0x00; //D2 must be 0V, else applies 5V to INT pin on ADXL
    //PORTD = (1<<PD0) | (0<<PD1);
}

void init_OC1A_CTC (void) //16 bit CTC counter
{

	OCR1A = 30000; // 16 bit output compare register
	TCCR1A |= ((0 << COM1A1)|(0 << COM1A0));  // Configure timer 0 for PWM phase correct mode, inverting etc.. (p.85)
	TCCR1B |= ((1 << WGM12)|(1 << CS12));
	TIMSK |= (1 << OCIE1A); // Enable CTC interrupt (Clear on Timer Compare)

}

void init_OC1A (void)
{
    //Initialise OC1A on pin B1
	DDRB |= (1<<DDB1);
	OCR1A = 0; 
    //Set PWM, phase correct, 8-bit (p97)
	TCCR1A |= (1 << WGM10);
    //Clear OC1A on compare match (non-inverting mode)
    TCCR1A |= (1 << COM1A1);  	
    //Set prescaler to /64
	TCCR1B |= ((1 << CS10)|(1 << CS11)); 
}

void init_OC1B (void)
{
    //Initialise OC1B on pin B2
	DDRB |= (1<<DDB2);
	OCR1B = 0;
    //Set PWM, phase correct, 8-bit (p97)
	TCCR1A |= (1 << WGM10);
    //Clear OC1A on compare match (non-inverting mode)
    TCCR1A |= (1 << COM1B1);  	
    //Set prescaler to /64
	TCCR1B |= ((1 << CS10)|(1 << CS11)); 
}

void init_OC2 (void)
{
    //Initialise OC2 on pin B3
	DDRB |= (1<<DDB3);
	OCR2 = 0; 
    //Set PWM, phase correct
	TCCR2 |= (1 << WGM20);
    //Clear OC2 on compare match (non-inverting mode)
    TCCR2 |= (1 << COM21); 
    // Set prescaler to /64
	TCCR2 |= (1 << CS22); 
} 



void init_INT0 (void) //Pin D2
{
    DDRD |= (0<<DDRD);
	//PORTD |= (0<<PD2);
	MCUCR |= (1 << ISC01) | (1 << ISC00); //Level change for int. p66
	GICR |= (1 << INT0); //Enable INT0
	sei();
}

void init_INT1 (void) //Pin D2
{
    DDRD |= (0<<DDRD);
	PORTD |= (1<<PD3); //Set pull up for switch
	MCUCR |= (1 << ISC11) | (0 << ISC10); //Falling edge for int. p66
	GICR |= (1 << INT1); //Enable INT1
	sei();
}
