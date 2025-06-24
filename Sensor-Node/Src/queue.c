
#include <queue.h>
#include <simulate.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f446xx.h"

// Simple Circular Queue Implementation to queue the anomaly detection
uint32_t waitingQueue[MAX_SENSOR];
int front = -1, rear = -1;

bool isQueueEmpty() {
    return (front == -1);
}

bool isQueueFull() {
    return ((rear + 1) % MAX_SENSOR == front);
}

void enqueue(uint32_t anomalyIndex) {
    // Check if already in Queue
    if (queuedItems[anomalyIndex]) {
        printf("Anomaly index %u already in queue.\n\r", anomalyIndex);
        return;
    }

    if (!isQueueFull()) {
        if (front == -1) front = 0;
        rear = (rear + 1) % MAX_SENSOR;
        waitingQueue[rear] = anomalyIndex;
        queuedItems[anomalyIndex] = true;       // Mark as queued
        printf("Enqueued anomaly index: %u\n\r", anomalyIndex);
    } else {
        printf("Queue is full! Cannot enqueue anomaly index: %u\n\r", anomalyIndex);
    }
}

uint32_t dequeue() {
    if (isQueueEmpty()) {
        printf("Queue is empty! Cannot dequeue.\n\r");
        return -1;
    }
    uint32_t anomalyIndex = waitingQueue[front];
    queuedItems[anomalyIndex] = false;              // Reset the flag

    if (front == rear) {
        front = rear = -1;
    } else {
        front = (front + 1) % MAX_SENSOR;
    }

    // printf("Dequeued anomaly index: %u\n\r", anomalyIndex);
    return anomalyIndex;   
}

void printQueue() {
    if (isQueueEmpty()) {
        printf("Queue is empty\n\r");
        return;
    }
    
    printf("Queue contents: [");
    int current = front;
    while (true) {
        printf("%u", waitingQueue[current]);
        if (current == rear) break;
        printf(", ");
        current = (current + 1) % MAX_SENSOR;
    }
    printf("]\n\r");
}