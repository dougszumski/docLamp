#include<stdio.h>
#include "sample.h"


void main()
{
    sample S;
    sample_init(&S);

    sample_push(1);
    
    printf("Average is: %g\n",sample_average(&S));

}

