#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {

char buf[5];
int k;

static const int lookup[7][4] = {
    { 2064 , 5 , 0 , 0 },
    { 3966 , 5 , 1 , 0 },
    { 5609 , 5 , 2 , 0 },
    { 6973 , 5 , 3 , 0 },
    { 8084 , 5 , 4 , 0 },
    { 8986 , 5 , 5 , 0 },
    { 9721 , 4 , 5 , 0 },
};

    //itoa(, buf, 10);

    sprintf(buf, "%d", lookup[0][0] );
    printf("dp[1][1] == %s\n", buf );

    k = 7;
    while (k > 0){  
            k--;
            printf("test%d\n", k);
            sprintf(buf, "%d", lookup[k][0] );
            printf("dp[%d][1] == %s\n",k, buf );
            } 

    return;
}



// convert 123 to string [buf]


   // int i = 42; // number to convert
  //  char buffer[3]; // 3 digit buffer
   // char* output; //output string (already filled with something)
   // sprintf(buffer, "%d", i);
   /// strcat(output, buffer);
