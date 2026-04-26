#ifndef INPUT_ENGINE_H
#define INPUT_ENGINE_H

#define INPUT_KEY_NONE (-1)
#define INPUT_KEY_BACKSPACE 8
#define INPUT_KEY_ENTER 10
#define INPUT_KEY_ESCAPE 27

int input_engine_prepare_session(int capacity);
void input_engine_reset_session(void);
int input_engine_poll_stdin(void);
int input_has_pending(void);
int input_peek(void);
int input_read(void);
void input_flush(void);
int input_buffer_capacity(void);
int input_buffer_usage(void);

#endif
