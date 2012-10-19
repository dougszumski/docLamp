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

#ifndef ADXL345_H
#define ADXL345_H

/* ADXL345 I2C Address (Alt. addr. pin grounded) */
#define ADXL345 0xA6
#define ADXL345_ID 0xE5
#define ADXL345_IDREG 0x00
#define POWER_CTL 0x2D
#define POWER_CTL_SET 0b00101000
#define DATA_FORMAT 0x31
#define DATA_FORMAT_SET 0b00001000
#define DATAX0 0x32

/* Double Tap */
#define THRESH_TAP 0x1D
#define DUR 0x21
#define LATENT 0x22
#define WINDOW 0x23
#define TAP_AXES 0x2A
#define ACT_TAP_STATUS 0x2B

/* Interrupts */
#define INT_ENABLE 0x2E
#define INT_MAP 0x2F
#define INT_SOURCE 0x30

/* Offset calibration */
#define OFSX 0x1E
#define OFSY 0x1F
#define OFSZ 0x20

/* Initialise the ADXL345 with predefined registers */
void ADXL345_init(void);

/* Update the acceleration register */
void ADXL345_updateVector(int16_t *vec);

/* Configure double tap interrupt */
void ADXL345_initDoubleTap(void);

/* Return the device ID */
uint8_t ADXL345_devID(void);

/* Clear the interrupt register */
void ADXL345_clearInt(void);

/* Add offset to acceleration vector */
void ADXL345_setOffset(int8_t xOff, int8_t yOff, int8_t zOff);

#endif //ADXL345_H
