#include "../include/keyboard.h"
#include "../include/input_engine.h"
#include <unistd.h>

int keyboard_readline(char *buf, int len) {
    int i = 0;

    if (!input_engine_prepare_session(len)) {
        if (len > 0) buf[0] = '\0';
        return 0;
    }

    input_flush();

    while (i < len - 1) {
        int key;

        input_engine_poll_stdin();
        if (!input_has_pending()) {
            usleep(1000);
            continue;
        }

        key = input_read();
        if (key == INPUT_KEY_NONE) continue;
        if (key == INPUT_KEY_ENTER) break;
        if (key == INPUT_KEY_BACKSPACE) {
            if (i > 0) i--;
            continue;
        }

        if (key >= 32 && key <= 126) {
            buf[i++] = (char)key;
        }
    }

    buf[i] = '\0';
    return i;
}

int keyboard_key_pressed(void) {
    input_engine_poll_stdin();
    return input_has_pending();
}
