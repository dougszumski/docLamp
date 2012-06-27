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

/* CPU frequency */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* UART baud */
#define UART_BAUD_RATE      57600      

/* ADXL345 I2C Address (Alt. addr. pin grounded) */
#define ADXL345 0xA6
#define ADXL345_ID 0xE5
#define ADXL345_IDREG 0x00

#define POWER_CTL 0x2D
#define POWER_CTL_SET 0b00101000

#define DATA_FORMAT 0x31
#define DATA_FORMAT_SET 0b00001000

#define DATAX0 0x32

/* KXPS5 register locations */
#define KXPS5_CREGB		0x0D
#define KXPS5_CREGC		0x0C
#define KXPS5_XHIGH     0x00

/* KXPS5 chip register settings */
#define KXPS5_CREGB_SET		0x42
#define KXPS5_CREGC_SET		0x00

/* Number of ms between PWM steps*/
#define DELAY 10

/* Macros */
#define len(x) (sizeof (x) / sizeof (*(x)))


int16_t vec[3];

sample X, Y, Z;


void updateVector(void)
{
    uint16_t xH, xL, yH, yL, zH, zL;

    // Read the acceleration registers sequentially 
    i2c_start(ADXL345+I2C_WRITE); // Set device address and write mode
    i2c_start_wait(ADXL345+I2C_WRITE);    // Set device address and write mode
    i2c_write(DATAX0); // Start reading at xH, auto increments to zL
    i2c_rep_start(ADXL345+I2C_READ);  // Set device address and read mode
    xL = i2c_readAck();                    
    xH = i2c_readAck();  
    yL = i2c_readAck();                    
    yH = i2c_readAck();  
    zL = i2c_readAck();                    
    zH = i2c_readNak();  
    i2c_stop();

    // Update the acceleration vector -- assumes data is right justified
    sample_push(&X, (xH << 8) | xL);
    sample_push(&Y, (yH << 8) | yL);
    sample_push(&Z, (zH << 8) | zL);

    //vec[0] = sample_average(&X);
    //vec[1] = sample_average(&Y);
    //vec[2] = sample_average(&Z);
    
    //Data is Right justified
    vec[0] = (((xH << 8) | xL)) ;
    vec[1] = (((yH << 8) | yL)) ;
    vec[2] = (((zH << 8) | zL)) ;

}
/*
void setupKXPS5(void)
{
    //Configure the KXPS5 setup registers
    i2c_start_wait(KXPS5+I2C_WRITE);     // set device address and write mode
    i2c_write(KXPS5_CREGB); 
    i2c_write(KXPS5_CREGB_SET);         
    i2c_stop();
    i2c_start_wait(KXPS5+I2C_WRITE);     // set device address and write mode
    i2c_write(KXPS5_CREGC);
    i2c_write(KXPS5_CREGC_SET);
    i2c_stop();
}

*/

uint8_t devidADXL345(void)
{
    uint8_t deviceID;
     /* Read back the device ID */
    i2c_start(ADXL345+I2C_WRITE); // Set device address and write mode
    i2c_start_wait(ADXL345+I2C_WRITE);    // Set device address and write mode
    i2c_write(ADXL345_IDREG); // Reading here
    i2c_rep_start(ADXL345+I2C_READ);  // Set device address and read mode               
    deviceID = i2c_readNak();  
    i2c_stop();

    return deviceID;
}

void initADXL345(void)
{
     //Configure the KXPS5 setup registers
    i2c_start_wait(ADXL345+I2C_WRITE);     // set device address and write mode
    i2c_write(POWER_CTL); 
    i2c_write(POWER_CTL_SET);         
    i2c_stop();

    i2c_start_wait(ADXL345+I2C_WRITE);     // set device address and write mode
    i2c_write(DATA_FORMAT); 
    i2c_write(DATA_FORMAT_SET);         
    i2c_stop();
}

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
    PORTD = 0xFF;
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

/*
void rgb_fade_d(uint16_t red_target, uint16_t green_target, uint16_t blue_target, uint16_t delay) 
{	
    //FIXME Pulls in fixed point algo -> 4kb bloat!
	//Adjust brightness level to targets	
    while ( (OCR1B != red_target) | (OCR2 != green_target) | (OCR1A != blue_target)) 
	{
		if (OCR1B < red_target) OCR1B++;
		if (OCR1B > red_target) OCR1B--;
		if (OCR2 < green_target) OCR2++;
		if (OCR2 > green_target) OCR2--;
		if (OCR1A < blue_target) OCR1A++;
		if (OCR1A > blue_target) OCR1A--;
		_delay_us(delay);
	}

}
*/

int main()
{
    char str_out[32] = "(x,y,z): ";
    char buffer[32];
    ldiv_t dummy;
    uint32_t tempVar;
    int i, j, l, up;
    uint16_t k, delay;
    uint8_t quadrant = 0;
    uint8_t beta, alpha, red,green,blue;
    uint16_t sum;
    
    // Init UART and enable globals ints for UART
    uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 
    sei();
    configPorts();
    init_OC1A();
    init_OC1B();
    init_OC2();
   
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


    //Init queue
    //sample_init(&X);
    //sample_init(&Y);
    //sample_init(&Z);

    // Initialise I2C and the ADXL345
    i2c_init();
    initADXL345();
    //init_OC1A_CTC();

    //Print device ID
    itoa(devidADXL345(), buffer, 10);
    uart_puts(buffer);  


    uart_puts("KXPS5 Initialised\n");
    j = 0;
    while(1){

        // Update acceleration vector and concatenate into string
        updateVector();
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
            tempVar = tempVar << 9;
            //Avoid divide by zero
            if (vec[0] != 0) {
                dummy = ldiv(tempVar,abs(vec[0]));
            }
            else {
                dummy.quot = 50000;
            }
            ultoa(dummy.quot, buffer, 10);
            strcat(str_out, buffer);
            strcat(str_out, ", ");

            // Work out which quadrant we're in
            if (vec[0] >= 0) {
                if (vec[1] >= 0) {
                    quadrant = 1;
                }
                else {
                    quadrant = 4;
                }
            }
            else if (vec[0] < 0) {
                if (vec[1] >= 0) {
                    quadrant = 2;
                }
                else {
                    quadrant = 3;
                }
            }
            utoa(quadrant, buffer, 10);
            strcat(str_out, buffer);
            strcat(str_out, ", ");

            // Now look up the PWM values...
            //TODO move this into a function
            
            if (quadrant == 1){
                k = 0;
                for (l = 0; l < len(lutQ1)-1; l++){
                    if (pgm_read_word(&lutQ1[l][0]) < dummy.quot) {
                       k++;
                    }
                    else {
                        break;
                    }
                }
                for (i = 1; i < 4; i++){
                    // Convert integer to char
                    if (i = 1) { 
                    red = (uint8_t)pgm_read_word(&lutQ1[k][i]); }
                    itoa(pgm_read_word(&lutQ1[k][i]), buffer, 10);
                    strcat(str_out, strcat(buffer, " "));
                }
            }
            else if (quadrant == 2){
                k = 0;
                for (l = 0; l < len(lutQ2)-1; l++){
                    if (pgm_read_word(&lutQ2[l][0]) < dummy.quot) {
                       k++;
                    }
                    else {
                        break;
                    }
                }
                for (i = 1; i < 4; i++){
                    // Convert integer to char
                    itoa(pgm_read_word(&lutQ2[k][i]), buffer, 10);
                    strcat(str_out, strcat(buffer, " "));
                }
            }
            else if (quadrant == 3){
                k = 0;
                for (l = 0; l < len(lutQ3)-1; l++){
                    if (pgm_read_word(&lutQ3[l][0]) < dummy.quot) {
                       k++;
                    }
                    else {
                        break;
                    }
                }
                for (i = 1; i < 4; i++){
                    // Convert integer to char
                    itoa(pgm_read_word(&lutQ3[k][i]), buffer, 10);
                    strcat(str_out, strcat(buffer, " "));
                }
            }
            else {
                k = 0;
                for (l = 0; l < len(lutQ4)-1; l++){
                    if (pgm_read_word(&lutQ4[l][0]) < dummy.quot) {
                       k++;
                    }
                    else {
                        break;
                    }
                }
                for (i = 1; i < 4; i++){
                    // Convert integer to char
                    itoa(pgm_read_word(&lutQ4[k][i]), buffer, 10);
                    strcat(str_out, strcat(buffer, " "));
                }
            }
            
            //rgb_fade(

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


