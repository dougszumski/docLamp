#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <math.h> 
#include "i2cmaster.h"
#include "uart.h"
#include "sample.h"
#include "lut.c"
#include "ADXL345.h"
#include "hConfig.h"

/* CPU frequency */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//#define _DEBUG 

/* Macros */
#define len(x) (sizeof (x) / sizeof (*(x)))

/* Update position */
void updatePosition (void);
/* Set colour form position*/ 
void pos2Colour (void);
/* Set colour temp from position*/
void pos2Temp(uint8_t* green, uint8_t* blue);
/* Fade to colour */
void rgb_fade(uint8_t red_target, uint8_t green_target, uint8_t blue_target, uint16_t delay);

int16_t vec[3];
sample X, Y, Z;
volatile uint8_t mode;
uint16_t theta;
uint8_t phi;

/*Stored colour for white lamp mode */
uint8_t storedGreen EEMEM = 255;
uint8_t storedBlue EEMEM = 255;

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
    uint8_t beta, alpha, red, green, blue, i;
    
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
    
    //Mainloop
    
    while(1)
    {
        delay = 5000;

        while (mode == 0) {
            //Lamp off
            rgb_fade(0,0,0, delay);
        }
        green = eeprom_read_byte(&storedGreen);
        blue = eeprom_read_byte(&storedBlue);
        rgb_fade(255, green, blue, 5000);
        while (mode == 1) {
            //White lamp with adjustable colour temp
            
            updatePosition();
            if (phi > 20 && phi < 85) {
                pos2Temp(&green, &blue);
                rgb_fade(255, green, blue, 5000);
            }
            
        }
        //Save the colour temp
        eeprom_write_byte(&storedGreen, green);
        eeprom_write_byte(&storedBlue, blue);
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
            if (mode != 4) break;
            rgb_fade(255,255,0, delay);	//yellow
            if (mode != 4) break;
	        rgb_fade(0,255,0, delay); 	//green
            if (mode != 4) break;
	        rgb_fade(0,255,255, delay);	//purple
            if (mode != 4) break;
	        rgb_fade(0,0,255, delay);  	//blue
            if (mode != 4) break;
	        rgb_fade(255,0,255, delay);	//pink
            if (mode != 4) break;
	        rgb_fade(255,0,0, delay); 	//red
        }      
        delay = 5000;
        rgb_fade(0,0,0, delay);
        delay = 65000;
        while (mode == 5) {
            red = rand() % 255;
            blue = rand() % 255;
            green = rand() % 255;
            rgb_fade(red, blue, green, delay);
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
            if (mode > 5) {
                mode = 0;
            }
		}
        if (count == 200){
            mode = 0;
            //rgb_fade(0,0,0, 10000);
        }
    }
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

void pos2Temp (uint8_t* green, uint8_t* blue)
{
    uint32_t pos;
    ldiv_t dummy;
  
    pos = theta;
    if (theta > 765 ) {
        pos = 1530 - theta;
    }
    dummy = ldiv((pos << 8), 768);
    *green = 255 - (dummy.quot*90 >> 8); 
    *blue = 255- (dummy.quot*255 >> 8);  
}
