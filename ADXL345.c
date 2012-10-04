#include <stdint.h>
#include "ADXL345.h"
#include "i2cmaster.h"

void ADXL345_init(void)
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

void ADXL345_updateVector(int16_t *vec)
{
    uint8_t xH, xL, yH, yL, zH, zL;

    // Read the acceleration registers sequentially 
    //i2c_start(ADXL345+I2C_WRITE); // Set device address and write mode
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

    //Data is Right justified
    vec[0] = (((xH << 8) | xL)) ;
    vec[1] = (((yH << 8) | yL)) ;
    vec[2] = (((zH << 8) | zL)) ;
}

void ADXL345_initDoubleTap(void)
{

    //Set tap threshold: 62.5mg/LSB
    i2c_start_wait(ADXL345+I2C_WRITE);
    i2c_write(THRESH_TAP); 
    i2c_write(0x18);         
    i2c_stop();

    //Write the DUR, LATENT and WINDOW bytes sequentially
    i2c_start_wait(ADXL345+I2C_WRITE);     
    i2c_write(DUR); 
    //Maximum time DURation above thereshold to be classed as a tap: 625us/LSB
    i2c_write(0x15);         
    //LATENT time from detection of first tap to start of time window: 1.25ms/LSB
    i2c_write(0x20);         
    //Time WINDOW after expiry of latent time in which a second tap can occur: 1.25ms/LSB
    i2c_write(0x50);         
    i2c_stop();

    //Tap axes: |0|0|0|0|suppress|tapxEn|tapyEn|tapzEn|
    i2c_start_wait(ADXL345+I2C_WRITE);    
    i2c_write(TAP_AXES); 
    i2c_write(0b00001111); //Supress, all axes         
    i2c_stop();

    //Interrupt enable for double tap
    i2c_start_wait(ADXL345+I2C_WRITE);     // set device address and write mode
    i2c_write(INT_ENABLE); 
    i2c_write(0b00100000);         
    i2c_stop();

    //Which pins the interrupts go to: p26.
    i2c_start_wait(ADXL345+I2C_WRITE);     // set device address and write mode
    i2c_write(INT_MAP); 
    i2c_write(0b00000000);         
    i2c_stop();
        
}

uint8_t ADXL345_devID(void)
{
    uint8_t deviceID;
     /* Read back the device ID */
    //i2c_start(ADXL345+I2C_WRITE); // Set device address and write mode
    i2c_start_wait(ADXL345+I2C_WRITE);    // Set device address and write mode
    i2c_write(ADXL345_IDREG); // Reading here
    i2c_rep_start(ADXL345+I2C_READ);  // Set device address and read mode               
    deviceID = i2c_readNak();  
    i2c_stop();

    return deviceID;
}

void ADXL345_clearInt(void)
{
    uint8_t dummy;
     /* Read the interupt source and do nothing with it (clears bit)*/
    //i2c_start(ADXL345+I2C_WRITE); // Set device address and write mode
    i2c_start_wait(ADXL345+I2C_WRITE);    // Set device address and write mode
    i2c_write(INT_SOURCE); // Reading here
    i2c_rep_start(ADXL345+I2C_READ);  // Set device address and read mode               
    dummy = i2c_readNak();  
    i2c_stop();

}

void ADXL345_setOffset(int8_t xOff, int8_t yOff, int8_t zOff)
{
    //Write the offset bytes sequentially
    //NOTE: Offsets are additive, 15.6mg/LSB which is 4 times _full_ res
    //sensitivty: 3.9mg/LSB
    i2c_start_wait(ADXL345+I2C_WRITE);     
    i2c_write(OFSX); 
    i2c_write(xOff);         
    i2c_write(yOff);         
    i2c_write(zOff);         
    i2c_stop();
}


