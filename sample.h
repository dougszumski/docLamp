/*
* Doug S. Szumski <d.s.szumski@gmail.com> and 
* Will J. Szumski <averylongnamewhichnobodyhas@gmail.com> 19-10-2012
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


#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#define SAMPLE_SIZE 60

typedef struct {
    uint8_t size;
    int16_t data[SAMPLE_SIZE]; 
} sample ;

void sample_init(sample * s);
void sample_push(sample *s, int16_t data_point);
int32_t sample_average(sample *s);

