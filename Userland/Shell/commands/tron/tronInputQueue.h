#ifndef troninputqueue_h
    #define troninputqueue_h

    #include <inout.h>

    #define INPUT_QUEUE_CAPACITY 32
    #define INPUT_FETCH_LIMIT 12
    #define INPUT_PROCESS_LIMIT 16

    typedef struct {
        KeyEvent items[INPUT_QUEUE_CAPACITY];
        int head;
        int tail;
        int size;
    } InputQueue;

    void inputQueueClear(void);

    bool inputQueuePop(KeyEvent *event);

    void inputQueueFetch(int limit);

#endif