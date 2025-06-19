#ifndef QUEUE_H
#define QUEUE_H

#include <simulate.h>

#include <stdint.h>
#include <stdbool.h>

extern uint32_t waitingQueue[MAX_SENSOR];
extern int front, rear;
extern bool queuedItems[MAX_SENSOR];

bool isQueueEmpty(void);
bool isQueueFull(void);
void enqueue(uint32_t anomalyIndex);
uint32_t dequeue(void);
void printQueue();

#endif // QUEUE_H
