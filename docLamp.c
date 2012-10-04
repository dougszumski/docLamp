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


/* Macros */
#define len(x) (sizeof (x) / sizeof (*(x)))

int16_t vec[3];
sample X, Y, Z;
volatile uint8_t mode;
uint16_t theta;
uint8_t phi;

//#define _DEBUG 

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

//RGB fader
void rgb_fade(uint8_t red_target, uint8_t green_target, uint8_t blue_target, uint16_t delay) 
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
        _delay_loop_2(delay);
	}  
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

void updatePosition (void)
{
    ldiv_t dummy;
    uint32_t tempVar;
    uint16_t k;
    uint16_t quadrant = 0;

    #ifdef _DEBUG
    char str_out[64] = "(x,y,z): ";
    char buffer[32];
    uint8_t i;
    #endif

    // Update acceleration vector and concatenate into string
    ADXL345_updateVector(&vec[0]);
    //TODO do this directly
    // Average the vector
    sample_push(&X, vec[0]);
    sample_push(&Y, vec[1]);
    sample_push(&Z, vec[2]);
    vec[0] = sample_average(&X);
    vec[1] = sample_average(&Y);
    vec[2] = -1*sample_average(&Z);

    #ifdef _DEBUG
    for (i = 0; i < 3; i++){
        // Convert integer to char
        itoa(vec[i], buffer, 10);
        strcat(str_out, strcat(buffer, " "));
    }
    strcat(str_out, ", ");
    #endif 

    //Calculate phi assuming that r = g
    tempVar = abs(vec[2]);
    tempVar = tempVar << 11;
    dummy = ldiv(tempVar,224); //aprx g.
    phi = 0;
    while (pgm_read_word(&acosLut[phi]) >= (uint16_t)dummy.quot){
         phi++;  
    }
    //We went past the position so step back
    phi--;
    
    #ifdef _DEBUG
    itoa(phi, buffer, 10);
    uart_puts(buffer); 
    #endif

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

    #ifdef _DEBUG
    ultoa(dummy.quot, buffer, 10);
    strcat(str_out, buffer);
    strcat(str_out, ", ");
    #endif

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

    // Now look up the PWM values...
    k = 0;
    if ( (quadrant == 0) | (quadrant == 2) ){
        //Ratio is increasing throughout the quadrant so count up
        //Ensure that entry in the lut is >= to the quotient
        while ((uint16_t)dummy.quot > pgm_read_word(&atanLut[k])){
            k++;  
        }
    }
    else {
        //Ratio is decreasing so count down
        k = 382 ;//len(lut)-1;
        //Ensure that entry in the lut is >= to the quotient
        while (pgm_read_word(&atanLut[k]) > dummy.quot ){
            k--;
        }
        //Invert k
        k = 382 - k;
    }

    #ifdef _DEBUG
    utoa(quadrant, buffer, 10);
    strcat(str_out, buffer);
    strcat(str_out, ", ");
    itoa(k, buffer, 10);
    uart_puts(buffer);  
    #endif

    //FIXME should this be 383 or 382???
    //TODO ass k back in
    k += 383*quadrant; 
    theta = k;
    
    #ifdef _DEBUG
    strcat(str_out, "Theta: ");
    itoa(theta, buffer, 10);
    strcat(str_out, buffer);
    #endif

}

void pos2Colour (void)
{
    uint8_t red,green,blue;
    uint16_t i;
    
    #ifdef _DEBUG
    char str_out[64] = "(r,g,b): ";
    char buffer[32];
    #endif

    red = 255;
    green = 0;
    blue = 0;
    
    i = theta;
    while (green < 255 && i > 0){
        green++;
        i--;         
    }
    while (red > 0 && i > 0) {
        red--;
        i--;
    }
    while (blue < 255 && i > 0) {
        blue++;
        i--;
    }
    while (green > 0 && i > 0) {
        green--;
        i--;
    }
    while (red < 255 && i > 0) {
        red++;
        i--;
    }
    while (blue > 0 && i > 0) {
        blue--;
        i--;
    }
        
    rgb_fade(red, green, blue, 5000);

    #ifdef _DEBUG
    strcat(str_out, ", (");
    itoa(red, buffer, 10);
    strcat(str_out, strcat(buffer, ","));
    itoa(green, buffer, 10);
    strcat(str_out, strcat(buffer, ","));
    itoa(blue, buffer, 10);
    strcat(str_out, strcat(buffer, "), L:"));
    uart_puts(str_out);  
    str_out[9] = '\0';
    #endif
    
}

void pos2Brightness (void)
{
    uint8_t bri =0;
    uint16_t i;

    i = theta;
    while (bri < 255 && i > 0){
        bri++;
        i--;         
    }
    while (bri > 0 && i > 0) {
        bri--;
        i--;
    }
    while (bri < 255 && i > 0) {
        bri++;
        i--;
    }
    while (bri > 0 && i > 0) {
        bri--;
        i--;
    }
    while (bri < 255 && i > 0) {
        bri++;
        i--;
    }
    while (bri > 0 && i > 0) {
        bri--;
        i--;
    }
        
    rgb_fade(bri, bri, bri, 10000);

}

int main()
{
    #ifdef _DEBUG
    char buffer[32];
    #endif

    //Init queue
    sample_init(&X);
    sample_init(&Y);
    sample_init(&Z);

    uint16_t sum, delay;
    uint8_t beta, alpha, red,green,blue, i;
    
    // Init UART and enable globals ints for UART
    #ifdef _DEBUG
    uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 
    #endif
    sei();
    configPorts();
    init_OC1A();
    init_OC1B();
    init_OC2();
    //Double tap
    init_INT0();
    //Switch
    init_INT1();

    // Initialise I2C and the ADXL345
    i2c_init();
    ADXL345_init();
    ADXL345_initDoubleTap();

    #ifdef _DEBUG
    //Print device ID
    itoa(ADXL345_devID(), buffer, 10);
    uart_puts(buffer);  
    uart_puts("KXPS5 Initialised\n");
    #endif
   
    mode = 0;
    
    /*
    while(1){
        while (mode == 0){
            rgb_fade(0,0,0);
        }
        while (mode == 1){
            rgb_fade(255,0,0);
        }
        while (mode == 2){
            rgb_fade(255,255,0);
        }
        while (mode == 3){
            rgb_fade(0,255,0);
        }
        while (mode == 4){
            rgb_fade(0,255,255);
        }
        while (mode == 5){
            rgb_fade(0,0,255);
        }
    //    
    } 
    */
    
    //Mainloop
    while(1)
    {
        delay = 5000;
        while (mode == 0) {
            //Lamp off
            rgb_fade(0,0,0, delay);
        }
        while (mode == 1) {
            //White lamp TODO allow dimming
            rgb_fade(255,255,255, delay);	//yellow
            updatePosition();
            if (phi > 20 && phi < 85) {
                pos2Brightness();
            }
        }
        if (mode == 2) {
        //Indicator the for colour select mode
        rgb_fade(0,0,0, delay);
        delay = 0;
        rgb_fade(15,0,0, delay);
        rgb_fade(0,0,0, delay);
        rgb_fade(0,15,0, delay);
        rgb_fade(0,0,0, delay);
        rgb_fade(0,0,15, delay);
        rgb_fade(0,0,0, delay);
        }
        while (mode == 2) {
            updatePosition();
            //The zone of of the doughnut.
            if (phi > 20 && phi < 85) {
                pos2Colour();
            }
        }   
        delay = 5000;
        rgb_fade(0,0,0, delay);
        while (mode ==3) {
            //Candle simulation
            //Generate normally distributed pseudo random numbers
		    sum = 0;
		    delay = 0;
		    alpha = 8; //Delay distribution width 
		    beta = (rand() % 10 + 1); //Random distribution width for brightness

		    for(i = 0; i < alpha;i++){
			    delay += (rand() % 65535 + 1);
		    } 
            //Skew to higher levels to prevent too much flickering
            if (delay < 10000) delay += 5000; 
           
		    for(i = 0; i < beta;i++){
			    sum += (rand() % 255 + 1);
		    } 
		    sum = sum / beta ;
            //Skew to higher levels and prevent it going out
		    if (sum < 200) {
                sum += 50; 
            }
		    //Candle colour
		    red = sum;
		    green = (sum*205) >> 8; //Equiv: *0.8
		    blue = (sum*38) >> 8; //Equiv: *0.15
		
		    rgb_fade(red,green,blue, delay); 
        }
        delay = 5000;
        rgb_fade(0,0,0, delay);
        delay = 65000;
        while (mode == 4) {
            rgb_fade(255,0,0, delay); 	//red
            rgb_fade(255,255,0, delay);	//yellow
	        rgb_fade(0,255,0, delay); 	//green
	        rgb_fade(0,255,255, delay);	//purple
	        rgb_fade(0,0,255, delay);  	//blue
	        rgb_fade(255,0,255, delay);	//pink
	        rgb_fade(255,0,0, delay); 	//red
      	
        }
             
    }	


}



ISR(TIMER1_COMPA_vect) //16bit one
{
	//PORTB = ~PORTB;// Invert port A
} 

ISR(INT0_vect)
{
    //Double Tap
    //mode++;
    //if (mode > 3) {
    //    mode = 0;
    //}
    PORTB = ~PORTB;
    //ADXL345_clearInt();
}

ISR(INT1_vect)
{
    uint8_t count = 0;
	//rgb_fade(0,0,0);
	while bit_is_clear(PIND, PD3) {
		_delay_ms(5);
        if (count < 254) {
		    count++;
        }
		if (count == 10){
            mode++;
            if (mode > 4) {
                mode = 0;
            }
		}
        if (count == 200){
            mode = 0;
            //rgb_fade(0,0,0, 10000);
        }
    }
}



