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

#include "filter.h"
#include "print.h"
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



uint8 filter_sample(uint8 reading, queue_t *prev);

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
    
    
    // The state of the FSM.
    uint8 state = 0;
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
    
    queue_t low_prev;
    queue_t med_prev;
    queue_t high_prev;
    int low_count;
    int med_count;
    int high_count;
    int liveness_count;
    
    init_queue(&low_prev);
    init_queue(&med_prev);
    init_queue(&high_prev);
    
    for(;;) {
        if (sample_lines) {
            sample_lines = 0;
            
            switch(state) {
                
                /* This is the state that is entered upon reset.
                 * In the future, this state should be returned to upon 
                 * event detection if the user applies an input (possibly a
                 * pushbutton to start with, and a BLE signal in the future).
                 */
                case 0:
                    low_count = 0;
                    med_count = 0;
                    high_count = 0;
                    LED_GREEN_Write(LIGHT_OFF);
                    LED_BLUE_Write(LIGHT_OFF);
                    LED_RED_Write(LIGHT_OFF);
                    state = 1;
                    break;
                
                /* In this state, keep a running count of each filtered input.
                 * If the counts exceed certain thresholds, proceed to either the
                 * alarm recognition state or the siren recognition state.
                 */
                // TODO break up count into helper functions, make new header file.
                case 1:
                    low_count = filter_count(LOW_FILTER_INPUT_Read(), &low_prev, low_count);
                    med_count = filter_count(MED_FILTER_INPUT_Read(), &med_prev, med_count);
                    high_count = filter_count(HIGH_FILTER_INPUT_Read(), &high_prev, high_count); 
                    UART_1_UartPutString("S1 ");
                    print_int(low_count);
                    UART_1_UartPutString(" ");
                    print_int(med_count);
                    UART_1_UartPutString(" ");
                    print_int(high_count);
                    UART_1_UartPutString("\r\n");
                    
                    /* Check for fire alarm threshold. */
                    if ((low_count > 300) && (med_count > 300) && (high_count > 300)) {
                        state = 5;
                        low_count = 0;
                        med_count = 0;
                        high_count = 0;
                        liveness_count = 0;
                        LED_RED_Write(LIGHT_ON);
                    }
                    /* Check for siren threshold. */
                    else if ((low_count > 40) && (med_count < 25) && (high_count < 10)) {
                        state = 2;
                        low_count = 0;
                        med_count = 0;
                        high_count = 0;
                        liveness_count = 0;
                        LED_BLUE_Write(LIGHT_ON);
                    }
                    break;
                
                /* In this state, check to see if the medium frequency range has been
                 * captured. If the signal remains too low for too long, return back to the 
                 * initial stage.
                 */
                case 2:
                    liveness_count++;
                    low_count = filter_count(LOW_FILTER_INPUT_Read(), &low_prev, low_count);
                    med_count = filter_count(MED_FILTER_INPUT_Read(), &med_prev, med_count);
                    high_count = filter_count(HIGH_FILTER_INPUT_Read(), &high_prev, high_count); 
                    UART_1_UartPutString("S2 ");
                    print_int(low_count);
                    UART_1_UartPutString(" ");
                    print_int(med_count);
                    UART_1_UartPutString(" ");
                    print_int(high_count);
                    UART_1_UartPutString(" ");
                    print_int(liveness_count);
                    UART_1_UartPutString("\r\n");
                    
                    if (liveness_count > 120) {
                        state = 0;
                        low_count = 0;
                        med_count = 0;
                        high_count = 0;
                        liveness_count = 0;                        
                        LED_BLUE_Write(LIGHT_OFF);
                    }
                    else if ((low_count > 40) && (med_count > 50) && (high_count < 25)) {
                        state = 3;
                        low_count = 0;
                        med_count = 0;
                        high_count = 0;
                        liveness_count = 0;
                    }                                            
                    break;
                    
                case 3:
                    liveness_count++;
                    low_count = filter_count(LOW_FILTER_INPUT_Read(), &low_prev, low_count);
                    med_count = filter_count(MED_FILTER_INPUT_Read(), &med_prev, med_count);
                    high_count = filter_count(HIGH_FILTER_INPUT_Read(), &high_prev, high_count);
                    
                    UART_1_UartPutString("S3 ");
                    print_int(low_count);
                    UART_1_UartPutString(" ");
                    print_int(med_count);
                    UART_1_UartPutString(" ");
                    print_int(high_count);
                    UART_1_UartPutString(" ");
                    print_int(liveness_count);
                    UART_1_UartPutString("\r\n");
                    
                    if (liveness_count > 140) {
                        state = 0;
                        low_count = 0;
                        med_count = 0;
                        high_count = 0;
                        liveness_count = 0;
                        LED_BLUE_Write(LIGHT_OFF);
                    }
                    else if ((low_count < 35) && (med_count > 30) && (high_count > 50)) {
                        state = 4;
                        low_count = 0;
                        med_count = 0;
                        high_count = 0;
                        liveness_count = 0;
                        LED_BLUE_Write(LIGHT_OFF);
                        LED_GREEN_Write(LIGHT_ON);
                        UART_1_UartPutString("Siren Detected\r\n");
                    }
                    break;
                /* Wait for user input to reset state machine. */
                case 4:
                    while(USER_INPUT_Read());
                    while(!USER_INPUT_Read());
                    state = 0;
                    LED_GREEN_Write(LIGHT_OFF);
                    break;
                    
                /* In this state, begin detection of the fire alarm.
                 */
                case 5:
                    LED_RED_Write(LIGHT_ON);
                    while(USER_INPUT_Read());
                    while(!USER_INPUT_Read());
                    LED_RED_Write(LIGHT_OFF);
                    state = 0;                   
                    break;
                    
                /* In this state, wait until user input to reset the
                 * FSM.
                 */
                case 6:
                    break;
                    
                default:
                    break;
            }        
        }
    }
    
    return 1;
}




/* [] END OF FILE */
