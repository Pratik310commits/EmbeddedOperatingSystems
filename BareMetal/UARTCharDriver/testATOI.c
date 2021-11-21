#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t ctoi(char c);

int main()
{
    char str[10] = "23";
    char *strptr = str;
    
    printf("%d\n", atoi(str));
    printf("%d\n", ctoi(strptr[0]));
    
}

uint8_t ctoi(char c)
{
    return atoi(&c);
}