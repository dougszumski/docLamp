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

