#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Reads one line into buf (max len-1 chars), returns number of chars read */
int keyboard_readline(char *buf, int len);
int keyboard_key_pressed(void);

#endif
