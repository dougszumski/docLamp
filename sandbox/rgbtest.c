#include <stdio.h>
#include <string.h>

void rgbSweep (int quadrant, int k)
{
    int red, green, blue;
    
    k += quadrant*383; 
    red = 255;
    green = 0;
    blue = 0;

    //printf("K is:%d\n", k);
    while (green < 255 && k > 0){
        green++;
        k--;         
    }
    while (red > 0 && k > 0) {
        red--;
        k--;
    }
    while (blue < 255 && k > 0) {
        blue++;
        k--;
    }
    while (green > 0 && k > 0) {
        green--;
        k--;
    }
    while (red < 255 && k > 0) {
        red++;
        k--;
    }
    while (blue > 0 && k > 0) {
        blue--;
        k--;
    }

    printf(" red: %d, green %d, blue %d\n", red, green, blue );
} 

int main(int argc, char **argv) {

    /*
    int i,j;
    for (j = 0; j<4; j++)
    {
        for (i = 0; i<383; i++) {
            rgbSweep(j,i);
        }
    }
    */
    int x = 0;
    int y = 10;
    while (++x < y){
        printf("x:%d\n", x);
        y--;
    }

 
}





// convert 123 to string [buf]


   // int i = 42; // number to convert
  //  char buffer[3]; // 3 digit buffer
   // char* output; //output string (already filled with something)
   // sprintf(buffer, "%d", i);
   /// strcat(output, buffer);
