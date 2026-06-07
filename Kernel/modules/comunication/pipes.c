#include <pipes.h>
#include <files.h>
#include <memory.h>
#include <processes.h>
#include <stddef.h>
#include <stdbool.h>

#define PIPE_BUFFER_SIZE 4096
#define MAX_PIPE_WAITERS 16

typedef struct pipe_t {
    char buffer[PIPE_BUFFER_SIZE];
    size_t read_idx;
    size_t write_idx;
    size_t count;

    int readers;
    int writers;

    pid_t read_waiters[MAX_PIPE_WAITERS];
    size_t read_waiter_count;
    pid_t write_waiters[MAX_PIPE_WAITERS];
    size_t write_waiter_count;
} pipe_t;

extern Process processes[];
extern size_t actual_index;

static int pipe_read(file_t *f, char *buf, int count);
static int pipe_write(file_t *f, const char *buf, int count);
static int pipe_close_read(file_t *f);
static int pipe_close_write(file_t *f);

static file_ops_t pipe_read_ops = {
    .read  = pipe_read,
    .write = NULL,
    .close = pipe_close_read,
};

static file_ops_t pipe_write_ops = {
    .read  = NULL,
    .write = pipe_write,
    .close = pipe_close_write,
};

static void enqueue_waiter(pid_t *queue, size_t *count, pid_t pid) {
    for (size_t i = 0; i < *count; i++) {
        if (queue[i] == pid) return;
    }
    if (*count < MAX_PIPE_WAITERS) {
        queue[(*count)++] = pid;
    }
}

static void wake_all(pid_t *queue, size_t *count) {
    size_t n = *count;
    *count = 0;
    for (size_t i = 0; i < n; i++) {
        unblock_process(queue[i]);
    }
}

static void block_self_on(pid_t *queue, size_t *count) {
    enqueue_waiter(queue, count, get_actual_pid());
    block_current(WAIT_PIPE);
}

static void try_free_pipe(pipe_t *p) {
    if (p->readers == 0 && p->writers == 0) {
        free(p);
    }
}

int create_pipe(file_t **read_end, file_t **write_end) {
    if (read_end == NULL || write_end == NULL) return -1;

    pipe_t *p = (pipe_t *) malloc(sizeof(pipe_t));
    if (p == NULL) return -1;

    file_t *rf = (file_t *) malloc(sizeof(file_t));
    file_t *wf = (file_t *) malloc(sizeof(file_t));
    if (rf == NULL || wf == NULL) {
        if (rf) free(rf);
        if (wf) free(wf);
        free(p);
        return -1;
    }

    p->read_idx = 0;
    p->write_idx = 0;
    p->count = 0;
    p->readers = 1;
    p->writers = 1;
    p->read_waiter_count = 0;
    p->write_waiter_count = 0;

    rf->ops = &pipe_read_ops;
    rf->data = p;
    rf->ref_count = 1;

    wf->ops = &pipe_write_ops;
    wf->data = p;
    wf->ref_count = 1;

    *read_end = rf;
    *write_end = wf;
    return 0;
}

static int pipe_read(file_t *f, char *buf, int count) {
    if (f == NULL || buf == NULL || count <= 0) return 0;
    pipe_t *p = (pipe_t *) f->data;

    // Esperar a que haya datos. Si no quedan writers y el buffer esta vacio -> EOF.
    while (p->count == 0) {
        if (p->writers == 0) return 0;
        block_self_on(p->read_waiters, &p->read_waiter_count);
    }

    int copied = 0;
    while (copied < count && p->count > 0) {
        buf[copied++] = p->buffer[p->read_idx];
        p->read_idx = (p->read_idx + 1) % PIPE_BUFFER_SIZE;
        p->count--;
    }

    // Liberamos espacio -> despertamos a los escritores bloqueados.
    if (copied > 0) {
        wake_all(p->write_waiters, &p->write_waiter_count);
    }

    return copied;
}

static int pipe_write(file_t *f, const char *buf, int count) {
    if (f == NULL || buf == NULL || count <= 0) return 0;
    pipe_t *p = (pipe_t *) f->data;

    int written = 0;
    while (written < count) {
        // Sin lectores no tiene sentido seguir escribiendo: pipe roto.
        if (p->readers == 0) return -1;

        // Bloquearse mientras el buffer este lleno.
        while (p->count == PIPE_BUFFER_SIZE) {
            if (p->readers == 0) return written > 0 ? written : -1;
            block_self_on(p->write_waiters, &p->write_waiter_count);
        }

        while (written < count && p->count < PIPE_BUFFER_SIZE) {
            p->buffer[p->write_idx] = buf[written++];
            p->write_idx = (p->write_idx + 1) % PIPE_BUFFER_SIZE;
            p->count++;
        }

        // Hay datos nuevos -> despertar lectores.
        wake_all(p->read_waiters, &p->read_waiter_count);
    }

    return written;
}

static int pipe_close_read(file_t *f) {
    if (f == NULL) return -1;
    pipe_t *p = (pipe_t *) f->data;

    if (--f->ref_count > 0) return 0;

    p->readers--;
    if (p->readers == 0) {
        // Escritores bloqueados se despiertan: van a ver readers == 0 y fallan.
        wake_all(p->write_waiters, &p->write_waiter_count);
    }
    try_free_pipe(p);
    free(f);
    return 0;
}

static int pipe_close_write(file_t *f) {
    if (f == NULL) return -1;
    pipe_t *p = (pipe_t *) f->data;

    if (--f->ref_count > 0) return 0;

    p->writers--;
    if (p->writers == 0) {
        // Lectores bloqueados se despiertan: ven count == 0 y writers == 0 -> EOF.
        wake_all(p->read_waiters, &p->read_waiter_count);
    }
    try_free_pipe(p);
    free(f);
    return 0;
}
