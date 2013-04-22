#ifndef MSP_SUPPORT_TOOLS_H
#define MSP_SUPPORT_TOOLS_H

#include <stdlib.h>

uint16_t read_int(void);
char* read_string(void);
void print_data(const unsigned char* data, size_t size);
int __attribute__((format(printf, 1, 2))) uart_printf(const char* str, ...);

#endif

