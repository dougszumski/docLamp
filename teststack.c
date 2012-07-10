#include<stdio.h>
#include "sample.h"


void main()
{
    sample S;
    int i = 0;

    sample_init(&S);
    
    while (i < 10) {
        sample_push(&S, i);
        i++;
    }
    
    for(i = 0; i < SAMPLE_SIZE; i++) {
        printf("%d\n",S.data[i]);
    }
    printf("Average is: %d\n", sample_average(&S));

}

