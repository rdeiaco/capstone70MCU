/*******************************************************************************
* File Name: main.c
*
* Version: 3.00
*
* Description:
*  This is the source code for the datasheet example of the TCPWM (Timer / 
*  Counter mode) component.
*
********************************************************************************
* Copyright 2013-2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#include <project.h>
#include <stdlib.h>

#include "queue.h"

/* LED control defines (active low)*/
#define LIGHT_OFF                       (1u)
#define LIGHT_ON                        (0u)
#define SIZE 2000U

/* Selects the active blinking LED */
uint8 activeLed;
// Flag to indicate when to sample digital lines.
uint8 sample_lines;


/*******************************************************************************
* Defines the interrupt service routine and allocates a vector to the interrupt.
* We use one handler for both the Capture and Terminal Count interrupts
* We toggle the active LED upon each Terminal Count interrupt
* We toggle the color (active LED) between blue and green upon each Capture 
* interrupt
********************************************************************************/

CY_ISR(InterruptHandler)
{
    
    if (Timer_INTR_MASK_TC == Timer_GetInterruptSource()) {
        // Timer has overflowed, 1ms has elapsed.
        // Clear interrupt, then set flag to execute finite state control loop.
        Timer_ClearInterrupt(Timer_INTR_MASK_TC);
        sample_lines = 1;
        
    }
}

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

    if ( i == 1) {
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

    i=1;
  }
}

int main()
{   
    #if (CY_PSOC4_4000)
        CySysWdtDisable();
    #endif /* (CY_PSOC4_4000) */
       
    /* Enable global interrupt */
    CyGlobalIntEnable;
    
    /* Enable interrupt component connected to interrupt */
    TC_CC_ISR_StartEx(InterruptHandler);

    /* Start components */
    Timer_Start();
    
    // The digital values from the low, medium and high filters, respectively.
    
    uint8 *low_val;
    uint8 *med_val;
    uint8 *high_val;
    // The state of the FSM.
    uint8 state;
    uint32 i = 0;
    
    UART_1_Start();

    UART_1_UartPutString("\r\nTest Line\r\n");
    print_int(123);
    UART_1_UartPutString("\r\n");
    print_int(12);
    UART_1_UartPutString("\r\n");
    print_int(100);
    UART_1_UartPutString("\r\n");
    print_int(1901);
    UART_1_UartPutString("\r\n");
    print_int(0);
    UART_1_UartPutString("\r\n");

    low_val = (uint8 *) malloc(SIZE*sizeof(uint8));
    med_val = (uint8 *) malloc(SIZE*sizeof(uint8));
    high_val = (uint8 *) malloc(SIZE*sizeof(uint8));
    
    queue_t low_prev;
    queue_t med_prev;
    queue_t high_prev;
    int temp;
    uint8 reading;
    
    init_queue(&low_prev);
    init_queue(&med_prev);
    init_queue(&high_prev);
    
    UART_1_UartPutString("Malloc Succesful.\r\n");
    
    while(i < SIZE) {
        
        if (sample_lines) {
            sample_lines = 0;
            
            reading = LOW_FILTER_INPUT_Read();
            temp = (int)reading;
            temp += sum(&low_prev);
            temp >>= 2;
            low_val[i] = (uint8)temp;
            push(&low_prev, reading);
            
            reading = MED_FILTER_INPUT_Read();
            temp = (int)reading;
            temp += sum(&med_prev);
            temp >>= 2;
            med_val[i] = (uint8)temp;
            push(&med_prev, reading);
            
            reading = HIGH_FILTER_INPUT_Read();
            temp = (int)reading;
            temp += sum(&high_prev);
            temp >>= 2;
            high_val[i] = (uint8)temp;
            push(&high_prev, reading);
                 
            i++;
        }       
    }
    
    UART_1_UartPutString("Loop Broken.\r\n");
    
    for (i = 0; i < SIZE; i++) { 
        print_int(i);
        
        UART_1_UartPutString(" ");
        UART_1_UartPutChar(low_val[i] + '0');
        UART_1_UartPutString(" ");
        UART_1_UartPutChar(med_val[i] + '0');
        UART_1_UartPutString(" ");
        UART_1_UartPutChar(high_val[i] + '0');
        UART_1_UartPutString("\r\n");
    }
    
    return 1;
}



/* [] END OF FILE */
