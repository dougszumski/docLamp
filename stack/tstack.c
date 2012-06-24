/****************************************************************************/
/*   Sample C stack tester demonstrating the usage of mystack functions.    */
/*   Stack struct is declared and functions are prototyped in mystack.h.    */ 
/*   The ANSI C code for the functions is in mystack.c.                     */
/*                                                                          */
/*   Compile with:                                                          */
/*     gcc -o tstack tstack.c mystack.c                                     */
/*                                                                          */
/*   Usage:                                                                 */
/*     tstack                                                               */
/*                                                                          */
/****************************************************************************/
#include<stdio.h>
#include "mystack.h"


void main()
{
    Stack S;

    init(&S);

    MyStackPrint(&S);

    int i = 0;
    while (i < 30) {
        push(&S, i);
        if (full(&S) == 1) {
        printf("Popped value is: %g\n",pop(&S));
        }
        i++;
    }

    //push(&S, 2.31);
    //push(&S, 1.19);
   
    MyStackPrint(&S);

   //printf("Popped value is: %g\n",pop(&S));
    
    //push(&S, 6.7);

    //MyStackPrint(&S);

    //push(&S, pop(&S) + pop(&S)); 

    //MyStackPrint(&S);
}

