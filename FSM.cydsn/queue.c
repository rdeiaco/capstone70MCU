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
#include "queue.h"

int sum(queue_t *queue) {
    int sum = 0;
    int i;
    for(i = 0; i < QUEUE_SIZE; i++) {
        sum += (int)(queue->data[i]);
    }
    return sum;
}

void push(queue_t *queue, uint8 val) {
    queue->data[queue->head] = val;
    queue->head = (queue->head + 1) % QUEUE_SIZE;
}

void init_queue(queue_t *queue) {
    int i;
    queue->head = 0;
    for (i = 0; i < QUEUE_SIZE; i++) {
        queue->data[i] = 0;
    }
}



/* [] END OF FILE */
