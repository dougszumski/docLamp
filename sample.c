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
