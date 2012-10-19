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

#include "sample.h"

void sample_init(sample * s) {
    s->size = 0;
}

void sample_push(sample *s, int16_t data_point) {
    int8_t i;
    int16_t previous;
    
    if (s->size < SAMPLE_SIZE) {
        s->size++;
    }
    // shift everything along
    
    for( i = SAMPLE_SIZE -1; i >= 0; i--) {
        if (i < SAMPLE_SIZE -1) {
            s->data[i + 1] = s->data[i];
        }
        //printf("i is: %d\n", i);
    } 
    //insert new value
    s->data[0] = data_point;
}

int32_t sample_average(sample *s) {
    int32_t total = 0;
    int8_t length;
    int8_t i;
    ldiv_t result;

    length = s->size;

    //printf("size: %d",length);

    if (length <= 0 ) {
        return 0;
    }

    //printf("total start: %d\n",total);
    for (i = 0; i < length ; i++) {
        total += s->data[i];
        //printf("total in loop: %d\n",total);
    }
    result = ldiv(total,length);
    return result.quot;
    //printf("total at end: %d\n",total);
    //return total;
}
