#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdint.h>
#include <util/twi.h>
//#include <compat/deprecated.h>
//#include <avr/interrupt.h>

#define TempRegAddr 0x00   // Temp register address of Temperature Sensor
#define ReadLM75Addr 0x91  
#define WriteLM75Addr 0x90 //Address when 3 lines pulled low on PCB
#define Twi_Stop() TWCR=_BV(TWINT)|_BV(TWSTO)|_BV(TWEN)                                                           
#define Twi_Start() TWCR=_BV(TWINT)|_BV(TWSTA)|_BV(TWEN)                                                                                            
#define check_TWINT() while(!(TWCR & (1<<TWINT))) 

#define uchar unsigned char
#define uint unsigned int

uchar TempHigh; 
uchar TempLow; 
uchar TempSign,TempData; 
uchar TwiStatus; 

void InitTwi(void);
void TWITempRead(uchar ReadDeviceAddr,uchar WriteDeviceAddr,uchar RegAddr) ;

void USARTInit(uint16_t ubrr_value)
{
   //Set Baud rate
   UBRRL = ubrr_value;
   UBRRH = (ubrr_value>>8);
   //Set frame format
   UCSRC=(1<<URSEL)|(3<<UCSZ0);
   //Enable The receiver and transmitter
   UCSRB=(1<<RXEN)|(1<<TXEN);
}

char USARTReadChar()
{
   //Wait untill a data is available
   while(!(UCSRA & (1<<RXC))) { }
   return UDR;
}

void USARTWriteChar(char data)
{
   //Wait until the transmitter is ready
   while(!(UCSRA & (1<<UDRE))) { }
   UDR=data;
}

void main()
{
   char data;
   uint  temp_H,temp_L;
   USARTInit(38);   //Baud rate = 19200bps @12MHz Fosc

    //data=USARTReadChar();
    //USARTWriteChar(data);
    //_delay_ms(100);
    InitTwi();

    while(1){
    _delay_ms(100);
   TWITempRead(ReadLM75Addr,WriteLM75Addr,TempRegAddr); 
   temp_H=TempHigh;                  //High bits
   temp_L=TempLow;                  //Low bits
   USARTWriteChar(temp_H);
   _delay_ms(100);
   USARTWriteChar(temp_L);
   _delay_ms(100); }

   /*while(1)
   {
      //Read data
      data=USARTReadChar();
      USARTWriteChar(data);
      _delay_ms(100);
   }*/
}


void InitTwi(void) 
{ 
    TWCR= 0X00; //disable twi 
    TWBR= 0x12; //set bit rate 
    TWSR= 0x01; //set prescale 
    TWCR= 0x04; //enable twi 
}

void TWITempRead(uchar ReadDeviceAddr,uchar WriteDeviceAddr,uchar RegAddr)  
{  
    _delay_ms(20);
    while (TwiStatus != 0x08)   //0b00001000  
    { 
         Twi_Start();           
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8; //0b11111000 TWSR BITS 7-3 are TWI status
   }  
                                                    
    while (TwiStatus != 0x18)        //0b00011000
    { 
         TWDR = WriteDeviceAddr;  //0x90
         TWCR=0x84;             //0b10000100   START CONDITION: Start TWI and clear TWINT flag
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8;   
   }                              

    //Selects temp reg              
   while (TwiStatus != 0x28)          //0b00101000                    
   { 
         TWDR = RegAddr;                     //0x00
         TWCR=0x84;   
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8;  
   } 
            
   while (TwiStatus != 0x10)        //0b00010000
   { 
         Twi_Start();         
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8;  
   } 
    
   while (TwiStatus != 0x40)         //0b01000000
   { 
         TWDR =ReadDeviceAddr;      //Read at 0x91 current pointer command   
         TWCR=0x84;   
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8;  
   } 
   TwiStatus=0x00;  

    //Get the temperature
   while (TwiStatus != 0x50)         
   { 
         TWCR=0xc4;                  
         _delay_ms(20);
         TempHigh=TWDR;         
         TwiStatus=TWSR & 0xF8;  
   } 
   while(TwiStatus != 0x58)                
   { 
         TwiStatus=TWSR & 0xF8;  
         TWCR=0x84;                 
         _delay_ms(10);
         TempLow=TWDR; 
   } 
   Twi_Stop();                                                      
}  

