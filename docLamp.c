#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <math.h> 
#include "i2cmaster.h"
#include "uart.h"
#include "sample.h"
#include "lut.c"
#include "ADXL345.h"

/* CPU frequency */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* UART baud */
#define UART_BAUD_RATE      57600      

/* Number of ms between PWM steps*/
#define DELAY 1

/* Macros */
#define len(x) (sizeof (x) / sizeof (*(x)))

int16_t vec[3];
//sample X, Y, Z;

void configPorts(void)
{
    //Set all pins to inputs
    DDRB = 0x00;
    DDRB |= ((1<<DDB0) | (1<<DDB1) | (1<<DDB2) | (1<<DDB3)); //Red LED is only output
    DDRC = 0x00;
    DDRD = 0x00;

    //Pulls ups on all ports
    PORTB = 0x00;
    PORTB = ((1<<PB0) | (0<<PB1) | (0<<PB2) | (0<<PB3)); //Turn LED off
    PORTC = 0xFF;
    PORTD = 0x00; //only d3 needsa to be 0
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

//RGB fader
void rgb_fade(uint8_t red_target, uint8_t green_target, uint8_t blue_target) 
{	
	//Adjust brightness level to targets	
	while ( (OCR1B != red_target) | (OCR2 != green_target) | (OCR1A != blue_target)) 
	{
		if (OCR1B < red_target) OCR1B++;
		if (OCR1B > red_target) OCR1B--;
		if (OCR2 < green_target) OCR2++;
		if (OCR2 > green_target) OCR2--;
		if (OCR1A < blue_target) OCR1A++;
		if (OCR1A > blue_target) OCR1A--;
		_delay_ms(DELAY);
	}  
} 

void init_INT0 (void) //Pin D2
{
    DDRD |= (0<<DDRD);
	//PORTD |= (0<<PD3);
	MCUCR |= (0 << ISC01) | (1 << ISC00); //Level change for int. p66
	GICR |= (1 << INT0); //Enable INT0
	sei();
}



int main()
{
    char str_out[64] = "(x,y,z): ";
    char buffer[32];
    ldiv_t dummy;
    uint32_t tempVar;
    int i, j;
    uint16_t k;
    uint16_t quadrant = 0;
    uint8_t  red,green,blue;
    uint16_t r;
    
    // Init UART and enable globals ints for UART
    uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 
    sei();
    configPorts();
    init_OC1A();
    init_OC1B();
    init_OC2();

    init_INT0();

    // Initialise I2C and the ADXL345
    i2c_init();
    ADXL345_init();
    ADXL345_initDoubleTap();
    //while(1){ADXL345_clearInt();
    //_delay_ms(2000);
    //};
   
    // Lamp test 
    /*
    rgb_fade(255,0,0); 	//red
    while(1){
        rgb_fade(255,255,0);	//yellow
	    rgb_fade(0,255,0); 	//green
	    rgb_fade(0,255,255);	//purple
	    rgb_fade(0,0,255);  	//blue
	    rgb_fade(255,0,255);	//pink
        rgb_fade(255,255,255);
        rgb_fade(0,0,0);
        _delay_ms(500);
	    rgb_fade(255,0,0); 	//red
    }} */

    /*  //Timer test 
    while(1) {
        tempVar = 65535;
        while (tempVar > 0) {
            tempVar--;}

        PORTB = ~PORTB;
    } */


    //Init queue
    //sample_init(&X);
    //sample_init(&Y);
    //sample_init(&Z);

    
    //init_OC1A_CTC();

    //Print device ID
    itoa(ADXL345_devID(), buffer, 10);
    uart_puts(buffer);  

    uart_puts("KXPS5 Initialised\n");
    j = 0;
    while(1){

        // Update acceleration vector and concatenate into string
        ADXL345_updateVector(vec);
        _delay_ms(1);

        
        if (j == 255){
        
            PORTB = ~PORTB;

            for (i = 0; i < 3; i++){
                // Convert integer to char
                itoa(vec[i], buffer, 10);
                strcat(str_out, strcat(buffer, " "));
            }
            strcat(str_out, ", ");
       
            // Calculate y/x using integer division
            tempVar = abs(vec[1]);
            tempVar = tempVar << 8;
            //Avoid divide by zero
            if (vec[0] != 0) {
                dummy = ldiv(tempVar,abs(vec[0]));
            }
            else {
                dummy.quot = 65535;
            }
            //Clamp the quotient at size of uint16
            if (dummy.quot > 65535) {
                dummy.quot = 65535;
            }
        
            ultoa(dummy.quot, buffer, 10);
            strcat(str_out, buffer);
            strcat(str_out, ", ");

            // Work out which quadrant we're in
            if (vec[0] >= 0) {
                if (vec[1] >= 0) {
                    quadrant = 0;
                }
                else {
                    quadrant = 3;
                }
            }
            else if (vec[0] < 0) {
                if (vec[1] >= 0) {
                    quadrant = 1;
                }
                else {
                    quadrant = 2;
                }
            }
            //utoa(quadrant, buffer, 10);
            //strcat(str_out, buffer);
            //strcat(str_out, ", ");

            // Now look up the PWM values...
            //TODO move this into a function
            
            k = 0;
            if ( (quadrant == 0) | (quadrant == 2) ){
                //Ratio is increasing throughout the quadrant so count up
                //Ensure that entry in the lut is >= to the quotient
                while ((uint16_t)dummy.quot > pgm_read_word(&lut[k])){
                    k++;  
                }
            }
            else {
                //Ratio is decreasing so count down
                k = 382 ;//len(lut)-1;
                //Ensure that entry in the lut is >= to the quotient
                while (pgm_read_word(&lut[k]) > dummy.quot ){
                    k--;
                }
                //Invert k
                k = 382 - k;
            }

            utoa(quadrant, buffer, 10);
            strcat(str_out, buffer);
            strcat(str_out, ", ");
            
            itoa(k, buffer, 10);
            uart_puts(buffer);  
            //Now calculate the colour: 
            for (int f=0; f < quadrant; f++) {
                k += 383; 
            }

            red = 255;
            green = 0;
            blue = 0;
            
            strcat(str_out, "k: ");
            itoa(k, buffer, 10);
            strcat(str_out, buffer);
            
            while (green < 255 && k > 0){
                green++;
                k--;         
            }
            while (red > 0 && k > 0) {
                red--;
                k--;
            }
            while (blue < 255 && k > 0) {
                blue++;
                k--;
            }
            while (green > 0 && k > 0) {
                green--;
                k--;
            }
            while (red < 255 && k > 0) {
                red++;
                k--;
            }
            while (blue > 0 && k > 0) {
                blue--;
                k--;
            }
         
            strcat(str_out, ", (");
            itoa(red, buffer, 10);
            strcat(str_out, strcat(buffer, ","));
            itoa(green, buffer, 10);
            strcat(str_out, strcat(buffer, ","));
            itoa(blue, buffer, 10);
            strcat(str_out, strcat(buffer, "), L:"));
            
            rgb_fade(red, green, blue);

            //r = sqrt(square(vec[0]) + square(vec[1]) + square(vec[2]));

            //ultoa(r, buffer, 10);
            //strcat(str_out, strcat(buffer, ""));

            //utoa(k, buffer, 10);
            //strcat(str_out, buffer);

            // Transmit to UART
            uart_puts(str_out);  
            str_out[9] = '\0';

            j = 0;

        }
        j++;
        
    }	
}

ISR(TIMER1_COMPA_vect) //16bit one
{
	//PORTB = ~PORTB;	// Invert port A
} 

ISR(INT0_vect)
{
    
    PORTB = ~PORTB;
}


 //strcat(str_out, strcat(buffer, " "));
        //itoa(var2, buffer, 10);
        //strcat(str_out, buffer);
        
        //Calculate r
        /*r = sqrt(square(vec[0]) + square(vec[1]) + square(vec[2]));
        dtostrf(r, 8, 4, r_str);
        strcat(str_out, r_str);
        strcat(str_out, ", ");

        //Calculate theta
        theta = atan2(vec[1], vec[0]);
        if (theta < 0.0) { 
            theta += M_PI*2;
        }
        dtostrf(theta, 6, 4, theta_str);
        strcat(str_out, theta_str);
        strcat(str_out, ", ");

        //Calculate phi
        phi = acos(vec[2]/r);
        dtostrf(phi, 8, 4, phi_str);
        strcat(str_out, phi_str);
        */


