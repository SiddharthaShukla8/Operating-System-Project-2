#include "../include/math.h"

int my_multiply(int a, int b) {
    int result = 0;
    int neg = 0;

    if (a < 0) { a = -a; neg = !neg; }
    if (b < 0) { b = -b; neg = !neg; }

    while (b--) result += a;

    return neg ? -result : result;
}

int my_divide(int a, int b) {
    if (b == 0) return 0;

    int neg = 0;

    if (a < 0) { a = -a; neg = !neg; }
    if (b < 0) { b = -b; neg = !neg; }

    int q = 0;

    while (a >= b) {
        a -= b;
        q++;
    }

    return neg ? -q : q;
}

int my_modulo(int a, int b) {
    int quotient;

    if (b == 0) return 0;

    quotient = my_divide(a, b);
    return a - my_multiply(quotient, b);
}

int my_abs(int value) {
    return value < 0 ? -value : value;
}

int my_min(int a, int b) {
    return a < b ? a : b;
}

int my_max(int a, int b) {
    return a > b ? a : b;
}

int my_clamp(int value, int min_value, int max_value) {
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}
