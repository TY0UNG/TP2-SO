#include "tronInputQueue.h"

static InputQueue pendingEvents;

void inputQueueClear(void) {
    pendingEvents.head = 0;
    pendingEvents.tail = 0;
    pendingEvents.size = 0;
}

void inputQueuePush(const KeyEvent *event) {
    pendingEvents.items[pendingEvents.head] = *event;
    pendingEvents.head = (pendingEvents.head + 1) % INPUT_QUEUE_CAPACITY;
    if (pendingEvents.size == INPUT_QUEUE_CAPACITY) {
        pendingEvents.tail = (pendingEvents.tail + 1) % INPUT_QUEUE_CAPACITY;
    } else {
        pendingEvents.size++;
    }
}

bool inputQueuePop(KeyEvent *event) {
    if (pendingEvents.size == 0) {
        return false;
    }
    *event = pendingEvents.items[pendingEvents.tail];
    pendingEvents.tail = (pendingEvents.tail + 1) % INPUT_QUEUE_CAPACITY;
    pendingEvents.size--;
    return true;
}

void inputQueueFetch(int limit) {
    for (int count = 0; count < limit; count++) {
        KeyEvent *raw = getKey();
        if (raw == NULL) {
            break;
        }
        inputQueuePush(raw);
    }
}