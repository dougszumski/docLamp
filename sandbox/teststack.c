#include<stdio.h>
#include "sample.h"


void main()
{
    sample S;
    sample_init(&S);

    int i = 0;
    while (i< 10) {
        sample_push(&S, i);
        i++;
    }
    

    //int i;
    for(i =0 ; i< SAMPLE_SIZE ; i++) {
        printf("%d\n",S.data[i]);
    }
    printf("Average is: %d\n", sample_average(&S));


}

