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
#ifndef QUEUE_H
#define QUEUE_H
#define QUEUE_SIZE 6
#include <project.h>
    
    
typedef struct {
    int head;
    char data[QUEUE_SIZE];
} queue_t;

int sum(queue_t *queue);
void push(queue_t *queue, uint8 val);
void init_queue(queue_t *queue);
    
    
#endif


/* [] END OF FILE */
