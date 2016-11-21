/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "print.h"
#include <project.h>

void print_hex(int val) {
    UART_1_UartPutString("0x");
    int temp;
    int i;
    for (i = 7; i >= 0; i++) {
        temp = (val >> (4*i)) & 0xF;
        if (temp < 10) {
            UART_1_UartPutChar('0' + temp);
        }
        else {
            UART_1_UartPutChar('A' + temp - 10);
        }
    }
}

void print_int (int val) {
  int temp = val;
  int i = 0;
  int j, div;
  int num_count = 1;
  int dig_count;

  while (num_count != 0) {

    if (i == 1) {
      div = 1;
      for ( j = num_count; j > 0; j-- ) {
        div *= 10;
      }
      temp = val % div;
      num_count--;
    }
    else {
      num_count = 0;
    }

    dig_count = 0;
    
    while (temp/10 != 0) {
      temp = temp/10;
      dig_count++;
      if ( i == 0 ) {
        num_count++;
      }
    }
    
    if (dig_count < num_count) {
      temp = 0;
    }
    
    UART_1_UartPutChar('0' + temp);

    i = 1;
  }
}
/* [] END OF FILE */
