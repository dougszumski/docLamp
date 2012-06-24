#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>

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
   USARTInit(38);   //Baud rate = 19200bps @12MHz Fosc
   while(1)
   {
      //Read data
      data=USARTReadChar();
      USARTWriteChar(data);
      _delay_ms(100);
   }
}
