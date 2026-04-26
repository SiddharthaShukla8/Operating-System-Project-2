#include "../include/input_engine.h"
#include "../include/memory.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static char *input_buffer = 0;
static int input_capacity = 0;
static int input_head = 0;
static int input_tail = 0;
static int input_used = 0;
static int stdin_configured = 0;

static void input_configure_stdin(void) {
    int flags;

    if (stdin_configured) return;

    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }

    stdin_configured = 1;
}

static int input_push(int value) {
    if (!input_buffer || input_used >= input_capacity) return 0;

    input_buffer[input_tail] = (char)value;
    input_tail = (input_tail + 1) % input_capacity;
    input_used++;
    return 1;
}

static int input_normalize_key(int value) {
    if (value == 127) return INPUT_KEY_BACKSPACE;
    if (value == '\r') return INPUT_KEY_ENTER;
    return value;
}

int input_engine_prepare_session(int capacity) {
    if (capacity <= 0) capacity = 64;

    input_configure_stdin();

    if (input_buffer && input_capacity >= capacity) {
        input_flush();
        return 1;
    }

    if (input_buffer) {
        memory_free(input_buffer);
        input_buffer = 0;
        input_capacity = 0;
    }

    input_buffer = (char *)memory_alloc_owned(capacity, MEMORY_OWNER_SESSION);
    if (!input_buffer) return 0;

    input_capacity = capacity;
    input_head = 0;
    input_tail = 0;
    input_used = 0;
    return 1;
}

void input_engine_reset_session(void) {
    input_buffer = 0;
    input_capacity = 0;
    input_head = 0;
    input_tail = 0;
    input_used = 0;
}

int input_engine_poll_stdin(void) {
    char read_buffer[64];
    int bytes_read;
    int pushed = 0;
    int index = 0;

    if (!input_buffer) return 0;

    while (1) {
        bytes_read = (int)read(STDIN_FILENO, read_buffer, sizeof(read_buffer));
        if (bytes_read <= 0) {
            if (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                return pushed;
            }
            break;
        }

        index = 0;
        while (index < bytes_read) {
            if (input_push(input_normalize_key((unsigned char)read_buffer[index]))) {
                pushed++;
            }
            index++;
        }
    }

    return pushed;
}

int input_has_pending(void) {
    return input_used > 0;
}

int input_peek(void) {
    if (!input_buffer || input_used <= 0) return INPUT_KEY_NONE;
    return (unsigned char)input_buffer[input_head];
}

int input_read(void) {
    int value;

    if (!input_buffer || input_used <= 0) return INPUT_KEY_NONE;

    value = (unsigned char)input_buffer[input_head];
    input_head = (input_head + 1) % input_capacity;
    input_used--;
    return value;
}

void input_flush(void) {
    input_head = 0;
    input_tail = 0;
    input_used = 0;
}

int input_buffer_capacity(void) {
    return input_capacity;
}

int input_buffer_usage(void) {
    return input_used;
}
