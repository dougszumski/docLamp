#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdint.h>
#include <util/twi.h>
//#include <compat/deprecated.h>
//#include <avr/interrupt.h>

//SLAVE address is |1|0|0|1|A2|A1|A0|R/W| -- From the maxim datasheet -- R/W byte is generic!!
//A2=A1=A0 = 0 therefore read address in hex is 0x91, write address is 0x90

#define TempRegAddr 0x00   //Temp register address of Temperature Sensor
#define ReadLM75Addr 0x19   
#define WriteLM75Addr 0x18 //Address is 0b001100X where X is one 0 on the PCB
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
void TWIWriteByte(uchar WriteDeviceAddr, uchar RegAddr, uchar RegContents) ;


void i2c_transmit(char address, char reg, char data);
unsigned char i2cRead(char address, char reg);

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
    _delay_ms(100);
    TWIWriteByte(0x24, 0x0D, 0x42);

    while(1){
    TWIWriteByte(0x24, 0x0D, 0x42);
   //TWITempRead(0x91,0x90,0x00); 
   //TWITempRead(0b10010001,0b10010000,0b00000000); 
   
   TWITempRead(0x25,0x24,0x00); //KXPS5
   //KXPS5_Init(ReadLM75Addr,WriteLM75Addr,TempRegAddr); 
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
    // Should be 75kHz if @ 12MHz
    // KXPS5 can go to 400KHz
    TWCR= 0X00; //disable twi 
    TWBR= 0x12; //set bit rate 
    TWSR= 0x01; //set prescale 
    TWCR= 0x04; //enable twi 
}

void TWITempRead(uchar ReadDeviceAddr,uchar WriteDeviceAddr,uchar RegAddr)  
{  

    //** LM75 pointer set, followed by immediate read: P8 Maxim LM75 datasheet
    //** Start by Master

    _delay_ms(20);
    while (TwiStatus != 0x08)   //0b00001000  WAITS for TWI to confirm it sent start to bus
    { 
         Twi_Start();           
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8; //0b11111000 TWSR BITS 7-3 are TWI status
   }  
                               
    //** Write address byte
                     
    while (TwiStatus != 0x18)        //0b00011000
    { 
         TWDR = WriteDeviceAddr;  //0x90 -- write address of LM75
         TWCR=0x84;             //0b10000100   START CONDITION: Start TWI and clear TWINT flag
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8;   
   }                              

    //NOTE in Transmit mode TWDR contains next byte to transmit, or last received byte in write /read  mode.


    //** Set pointer
    //Selects temp reg              
   while (TwiStatus != 0x28)          //0b00101000                    
   { 
         TWDR = RegAddr;                     //0x00 --this points to temp reg   
         TWCR=0x84;   //Enter master transmit mode? p187 datasheet
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8;  
   } 
            

   //** Repeat start by master
   while (TwiStatus != 0x10)        //0b00010000
   { 
         Twi_Start();         
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8;  
   } 
    
    //** Read Address byte
   while (TwiStatus != 0x40)         //0b01000000
   { 
         TWDR =ReadDeviceAddr;      //Read at 0x91 current pointer command   
         TWCR=0x84;   
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8;  
   } 
   TwiStatus=0x00;  

    //**Get data bytes
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
    //** Stop by master
   Twi_Stop();                                                      
}  


void TWIWriteByte(uchar WriteDeviceAddr, uchar RegAddr, uchar RegContents)  
{  

    //** S
    _delay_ms(20);
    while (TwiStatus != 0x08)   //0b00001000  
    { 
         Twi_Start();           
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8; //0b11111000 TWSR BITS 7-3 are TWI status
    }  
                               
    //** SAD+W 
    while (TwiStatus != 0x18)        //0b00011000
    { 
         TWDR = WriteDeviceAddr;  //0x90 -- write address of LM75
         TWCR=0x84;             //0b10000100   START CONDITION: Start TWI and clear TWINT flag
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8;   
   }                              

   //** RA          
   while (TwiStatus != 0x28)          //0b00101000                    
   { 
         TWDR = RegAddr;                     //0x00 --this points to temp reg   
         TWCR=0x84;   //Enter master transmit mode? p187 datasheet
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8;  
   } 
            
   //**Write data bytes
   while (TwiStatus != 0x28)         
   { 
         TWDR = RegContents;  
         TWCR=0x84;                  
         _delay_ms(1);
         TwiStatus=TWSR & 0xF8;  
   } 
   /*while(TwiStatus != 0x58)                
   { 
         TwiStatus=TWSR & 0xF8;  
         TWCR=0x84;                 
         _delay_ms(10);
         TempLow=TWDR; 
   } */
    
   //** Stop by master
   Twi_Stop(); 
                                                 
}  


void i2c_transmit(char address, char reg, char data)
{
	TWCR = 0xA4;                                                  // send a start bit on i2c bus
	while(!(TWCR & 0x80));                                        // wait for confirmation of transmit 
	TWDR = address;                                               // load address of i2c device
	TWCR = 0x84;                                                  // transmit
	while(!(TWCR & 0x80));                                        // wait for confirmation of transmit
	TWDR = reg;
	TWCR = 0x84;                                                  // transmit
	while(!(TWCR & 0x80));                                        // wait for confirmation of transmit
	TWDR = data;
	TWCR = 0x84;                                                  // transmit
	while(!(TWCR & 0x80));                                        // wait for confirmation of transmit
	TWCR = 0x94;                                                  // stop bit
}

unsigned char i2cRead(char address, char reg)
{
char read_data = 0;

   TWCR = 0xA4;                                                  // send a start bit on i2c bus
   while(!(TWCR & 0x80));                                        // wait for confirmation of transmit  
   TWDR = address;                                               // load address of i2c device
   TWCR = 0x84;                                                  // transmit 
   while(!(TWCR & 0x80));                                        // wait for confirmation of transmit
   TWDR = reg;                                                   // send register number to read from
   TWCR = 0x84;                                                  // transmit
   while(!(TWCR & 0x80));                                        // wait for confirmation of transmit

   TWCR = 0xA4;                                                  // send repeated start bit
   while(!(TWCR & 0x80));                                        // wait for confirmation of transmit 
   TWDR = address+1;                                             // transmit address of i2c device with readbit set
   TWCR = 0xC4;                                                  // clear transmit interupt flag
   while(!(TWCR & 0x80));                                        // wait for confirmation of transmit
   TWCR = 0x84;                                                  // transmit, nack (last byte request)
   while(!(TWCR & 0x80));                                        // wait for confirmation of transmit 
   read_data = TWDR;                                             // and grab the target data
   TWCR = 0x94;                                                  // send a stop bit on i2c bus
   return read_data;

}





