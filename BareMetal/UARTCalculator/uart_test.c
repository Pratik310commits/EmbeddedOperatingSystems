// #include <stdint.h>
// #include <stdbool.h>
#include <string.h>
// #include <stdlib.h>
#include "uart_pl011.h"

static void executeArithCmd();
static void sanitizeArithCmd(uint8_t *firstOperand, uint8_t *secondOperand, char *arithOperator);
uint8_t ctoi(char c);
void clearBuffers();

char buf[64];
char arith_buf[8];

bool calcRequested = false;

static void parse_cmd(void) {
    if (!strncmp("help\r", buf, strlen("help\r"))) {
        uart_write("Just type and see what happens!\n");
    } else if (!strncmp("uname\r", buf, strlen("uname\r"))) {
        uart_write("bare-metal arm 06_uart\n");
    } else if (!strncmp("calc ", buf, strlen("calc "))) {
        // uart_write("Calculator requested: Flag set\n");
        calcRequested = true;
    }
}

int main() {
    uart_config config = {
        .data_bits = 8,
        .stop_bits = 1,
        .parity = false,
        .baudrate = 9600
    };
    uart_configure(&config);

    char c;
    uint8_t buf_idx = 0u;
    uint8_t arith_buf_idx = 0u;
    uint8_t opCount = 0;
    bool erroneousCmd = false;
    while (1)
    {        
        if (uart_getchar(&c) == UART_OK)
        {
            uart_putchar(c);
            if(calcRequested)
            {
                if((c >= '0' && c <= '9') || c == '\r' || c ==' ')
                {
                    if(c != ' ')
                    {
                        arith_buf[arith_buf_idx % 8] = c;
                        arith_buf_idx++;
                    }
                }
                else if(c == '+' || c == '-' || c == '*' || c == '/')
                {
                    opCount++;
                    if(opCount > 1)
                    {
                        uart_write("\n**Multiple '");
                        uart_putchar(c);
                        uart_write("' used.");
                        erroneousCmd = true;
                    }
                    else
                    {
                        arith_buf[arith_buf_idx % 8] = c;
                        arith_buf_idx++;
                    }
                }
                else
                {
                    uart_write("\n**Invalid symbol '");
                    uart_putchar(c);
                    uart_write("' used.");
                    erroneousCmd = true;
                }

                if(erroneousCmd)
                {
                    uart_write(" Only allowed to use integers and 4 basic arithmetic operations. Try again.**\n");
                    arith_buf_idx = 0;
                    buf_idx = 0;
                    calcRequested = false;
                    erroneousCmd = false;
                    clearBuffers();
                    continue;
                }
            }
            
            buf[buf_idx % 64] = c;
            buf_idx++;
            parse_cmd();
            if (c == '\r')
            {
                uart_write("\n");
                buf_idx = 0u;
                
                // uart_write((char *)(&buf));
                // parse_cmd();
                if(calcRequested)
                {
                    uart_write("Calc requested\n");
                    // arith_buf[arith_buf_idx % 8] = c;
                    // arith_buf_idx = 0u;
                    executeArithCmd();
                    opCount = 0u;
                    calcRequested = false;
                    arith_buf_idx = 0u;
                    buf_idx = 0u;
                }
                clearBuffers();
            }
        }
    }
    return 0;
}

void executeArithCmd()
{
    uint8_t value = 0u;
    char arithOperator = '-';
    uint8_t firstOperand = 0u;
    uint8_t secondOperand = 0u;
    sanitizeArithCmd(&firstOperand, &secondOperand, &arithOperator);
    
    if(arithOperator == '+')
        value = firstOperand + secondOperand;
    else if(arithOperator == '-')
        value = firstOperand - secondOperand;
    else if(arithOperator == '/')
    {
        if(secondOperand == 0)
        {
            uart_write("\n**Divide by zero operation not allowed.**\n");
            return;
        }
        else
            value = (firstOperand / secondOperand);
    }
    else if(arithOperator == '*')
        value = (firstOperand * secondOperand);
    
    uart_write("answer: ");
    for(int i = 100; i > 0; i/=10)
    {
        // if(value >= i)
        if((value/i) > 0)
        {
            uart_putchar((value/i) + 48);
            value %= i;
        }
        else
            uart_putchar('0');        
    }
    uart_putchar('\n');
}

void sanitizeArithCmd(uint8_t *firstOperand, uint8_t *secondOperand, char *arithOperator)
{
    *firstOperand = 0;
    *secondOperand = 0;
    char *arith_buf_temp = arith_buf;

    *firstOperand = ctoi(*arith_buf_temp);
    arith_buf_temp++;

    while(*arith_buf_temp != '+' && *arith_buf_temp != '-' && *arith_buf_temp != '*' && *arith_buf_temp != '/')
    {
        *firstOperand = (*firstOperand)*10 +  ctoi(*arith_buf_temp);
        arith_buf_temp++;
    }

    *arithOperator = *arith_buf_temp;
    // uart_write("Op: ");
    // uart_putchar(*arith_buf_temp);
    // uart_putchar('\n');
    if(*arithOperator == '\r')
    {
        // uart_write("Not Skipped\n");
        *arithOperator = '+';
        return;
    }
    // uart_write("Skipped\n");
    arith_buf_temp++;

    *secondOperand = ctoi(*arith_buf_temp);
    arith_buf_temp++;
    while(*arith_buf_temp != '\r')
    {
        *secondOperand = (*secondOperand)*10 +  ctoi(*arith_buf_temp);
        arith_buf_temp++;
    }

}

void clearBuffers()
{
    memset(buf, '0', 64);
    memset(arith_buf, '0', 8);
}

uint8_t ctoi(char c)
{
    return atoi(&c);
}

/*

arm-none-eabi-as -mcpu=cortex-a8 -o startup.o startup.s
arm-none-eabi-gcc -c -mcpu=cortex-a8 -o uart_pl011.o uart_pl011.c 
arm-none-eabi-gcc -c -mcpu=cortex-a8 -o uart_test.o uart_test.c 
arm-none-eabi-gcc -nostartfiles -T uart_test.ld startup.o uart_pl011.o uart_test.o -o uart
arm-none-eabi-objcopy -O binary uart uart.bin

qemu-system-arm -M realview-pb-a8 -nographic -kernel uart.bin
*/