#include "../include/mystring.h"
#include "../include/json_loader.h"
#include "../include/memory.h"
#include "../include/math.h"
#include "../include/input_engine.h"
#include <stdlib.h>

#define MAX_GENERATED_WORDS 600

static SentenceGenerationInfo last_generation_info;

static int my_is_digit(char value) {
    return value >= '0' && value <= '9';
}

static int my_recent_contains(const int *recent, int recent_count, int candidate) {
    int index = 0;

    while (index < recent_count) {
        if (recent[index] == candidate) return 1;
        index++;
    }

    return 0;
}

static void my_shift_recent(int *recent, int recent_count, int value) {
    int index = recent_count - 1;

    while (index > 0) {
        recent[index] = recent[index - 1];
        index--;
    }

    recent[0] = value;
}

static int word_matches_difficulty(const char *word, const char *difficulty) {
    int length;

    if (!difficulty || my_strcmp(difficulty, "mixed") == 0) return 1;

    length = my_strlen(word);
    if (my_strcmp(difficulty, "easy") == 0) {
        return length <= 4;
    }

    if (my_strcmp(difficulty, "medium") == 0) {
        return length >= 5 && length <= 7;
    }

    if (my_strcmp(difficulty, "hard") == 0) {
        return length >= 8;
    }

    return 1;
}

static int pick_dataset_index(const WordDataset *dataset, const char *difficulty, int *recent_words, int *duplicates_avoided) {
    int attempt = 0;
    int index = 0;
    int scan = 0;
    int start = 0;

    if (!dataset || dataset->count <= 0) return -1;

    start = my_modulo(rand(), dataset->count);

    while (attempt < 48) {
        index = my_modulo(rand(), dataset->count);
        if (!word_matches_difficulty(dataset->words[index], difficulty)) {
            attempt++;
            continue;
        }

        if (my_recent_contains(recent_words, 3, index)) {
            (*duplicates_avoided)++;
            attempt++;
            continue;
        }

        return index;
    }

    while (scan < dataset->count) {
        index = my_modulo(start + scan, dataset->count);
        if (word_matches_difficulty(dataset->words[index], difficulty) && !my_recent_contains(recent_words, 3, index)) {
            return index;
        }
        scan++;
    }

    scan = 0;
    while (scan < dataset->count) {
        index = my_modulo(start + scan, dataset->count);
        if (word_matches_difficulty(dataset->words[index], difficulty)) {
            return index;
        }
        scan++;
    }

    return start;
}

static int generated_number_value(void) {
    static const int values[] = { 7, 12, 24, 32, 42, 64, 72, 96, 108, 128, 256, 512, 1024 };
    int count = (int)(sizeof(values) / sizeof(values[0]));
    return values[my_modulo(rand(), count)];
}

int my_strlen(const char *s) {
    int len = 0;
    if (!s) return 0;

    while (s[len] != '\0') len++;
    return len;
}

int my_strcmp(const char *a, const char *b) {
    int index = 0;

    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;

    while (a[index] != '\0' && b[index] != '\0') {
        if (a[index] != b[index]) return a[index] - b[index];
        index++;
    }

    return a[index] - b[index];
}

int my_strncmp(const char *a, const char *b, int count) {
    int index = 0;

    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;

    while (index < count && a[index] != '\0' && b[index] != '\0') {
        if (a[index] != b[index]) return a[index] - b[index];
        index++;
    }

    if (index == count) return 0;
    return a[index] - b[index];
}

char *my_strcpy(char *dest, const char *src) {
    int index = 0;

    if (!dest) return dest;
    if (!src) {
        dest[0] = '\0';
        return dest;
    }

    while (src[index] != '\0') {
        dest[index] = src[index];
        index++;
    }

    dest[index] = '\0';
    return dest;
}

char *my_strncpy(char *dest, const char *src, int count) {
    int index = 0;

    if (!dest || count <= 0) return dest;
    if (!src) {
        dest[0] = '\0';
        return dest;
    }

    while (index < count - 1 && src[index] != '\0') {
        dest[index] = src[index];
        index++;
    }

    dest[index] = '\0';
    return dest;
}

char *my_strcat(char *dest, const char *src) {
    int dest_len;
    int index = 0;

    if (!dest || !src) return dest;

    dest_len = my_strlen(dest);
    while (src[index] != '\0') {
        dest[dest_len + index] = src[index];
        index++;
    }

    dest[dest_len + index] = '\0';
    return dest;
}

char *my_strstr(const char *haystack, const char *needle) {
    int hay_index = 0;
    int needle_len = my_strlen(needle);

    if (!haystack || !needle) return 0;
    if (needle_len == 0) return (char *)haystack;

    while (haystack[hay_index] != '\0') {
        if (my_strncmp(haystack + hay_index, needle, needle_len) == 0) {
            return (char *)(haystack + hay_index);
        }
        hay_index++;
    }

    return 0;
}

int my_atoi(const char *text) {
    int sign = 1;
    int value = 0;
    int index = 0;

    if (!text) return 0;

    if (text[index] == '-') {
        sign = -1;
        index++;
    }

    while (my_is_digit(text[index])) {
        value = my_multiply(value, 10) + (text[index] - '0');
        index++;
    }

    return value * sign;
}

void my_itoa(int value, char *buffer) {
    int index = 0;
    int start = 0;
    int end = 0;

    if (!buffer) return;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    if (value < 0) {
        buffer[index++] = '-';
        value = -value;
        start = 1;
    }

    while (value > 0) {
        buffer[index++] = (char)('0' + my_modulo(value, 10));
        value = my_divide(value, 10);
    }

    buffer[index] = '\0';
    end = index - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }
}

int my_append_char(char *buffer, int *position, int limit, char value) {
    if (!buffer || !position || *position >= limit - 1) return 0;

    buffer[*position] = value;
    (*position)++;
    buffer[*position] = '\0';
    return 1;
}

int my_append_text(char *buffer, int *position, int limit, const char *text) {
    int index = 0;

    if (!buffer || !position || !text) return 0;

    while (text[index] != '\0') {
        if (!my_append_char(buffer, position, limit, text[index])) {
            return 0;
        }
        index++;
    }

    return 1;
}

int my_append_int(char *buffer, int *position, int limit, int value) {
    char digits[32];

    my_itoa(value, digits);
    return my_append_text(buffer, position, limit, digits);
}

char *generate_sentence(int num_words) {
    return generate_sentence_configured(num_words, "mixed", 0, 0);
}

char *generate_sentence_configured(int num_words, const char *difficulty, int punctuation_enabled, int numbers_enabled) {
    const WordDataset *dataset = json_loader_get_dataset();
    int selected_indexes[MAX_GENERATED_WORDS];
    int number_values[MAX_GENERATED_WORDS];
    int number_lengths[MAX_GENERATED_WORDS];
    int number_flags[MAX_GENERATED_WORDS];
    char punctuation_marks[MAX_GENERATED_WORDS];
    int recent_words[3] = { -1, -1, -1 };
    int total_length = 0;
    int generated_words = 0;
    int duplicates_avoided = 0;
    int index = 0;
    char *sentence = 0;
    char number_buffer[32];

    if (num_words <= 0) num_words = 1;
    if (num_words > MAX_GENERATED_WORDS) num_words = MAX_GENERATED_WORDS;

    if (!dataset || dataset->count <= 0) {
        return 0;
    }

    while (generated_words < num_words) {
        int dataset_index = pick_dataset_index(dataset, difficulty, recent_words, &duplicates_avoided);
        int use_number = numbers_enabled && generated_words > 0 && my_modulo(generated_words + 1, 7) == 0;

        selected_indexes[generated_words] = dataset_index;
        number_flags[generated_words] = use_number;
        punctuation_marks[generated_words] = 0;
        number_lengths[generated_words] = 0;
        number_values[generated_words] = 0;

        if (use_number) {
            number_values[generated_words] = generated_number_value();
            my_itoa(number_values[generated_words], number_buffer);
            number_lengths[generated_words] = my_strlen(number_buffer);
            total_length += number_lengths[generated_words];
        } else {
            total_length += my_strlen(dataset->words[dataset_index]);
            my_shift_recent(recent_words, 3, dataset_index);
        }

        if (punctuation_enabled) {
            if (generated_words == num_words - 1) {
                punctuation_marks[generated_words] = '.';
                total_length++;
            } else if (my_modulo(generated_words + 1, 9) == 0) {
                punctuation_marks[generated_words] = ',';
                total_length++;
            }
        }

        if (generated_words > 0) total_length++;
        generated_words++;
    }

    sentence = (char *)memory_alloc_owned(total_length + 1, MEMORY_OWNER_SESSION);
    if (!sentence) return 0;

    total_length = 0;
    index = 0;
    while (index < generated_words) {
        if (index > 0) {
            sentence[total_length++] = ' ';
        }

        if (number_flags[index]) {
            int digit_index = 0;
            my_itoa(number_values[index], number_buffer);
            while (number_buffer[digit_index] != '\0') {
                sentence[total_length++] = number_buffer[digit_index++];
            }
        } else {
            const char *word = dataset->words[selected_indexes[index]];
            int char_index = 0;
            while (word[char_index] != '\0') {
                sentence[total_length++] = word[char_index++];
            }
        }

        if (punctuation_marks[index] != 0) {
            sentence[total_length++] = punctuation_marks[index];
        }

        index++;
    }
    sentence[total_length] = '\0';

    if (input_engine_prepare_session(total_length + num_words + 32)) {
        input_flush();
    }

    last_generation_info.requested_words = num_words;
    last_generation_info.generated_words = generated_words;
    last_generation_info.sentence_length = total_length;
    last_generation_info.dataset_words = dataset->count;
    last_generation_info.duplicates_avoided = duplicates_avoided;
    last_generation_info.punctuation_enabled = punctuation_enabled;
    last_generation_info.numbers_enabled = numbers_enabled;
    last_generation_info.input_buffer_capacity = input_buffer_capacity();
    last_generation_info.input_buffer_usage = input_buffer_usage();
    last_generation_info.session_memory_footprint = memory_usage().session_used_bytes;
    my_strncpy(last_generation_info.difficulty, difficulty ? difficulty : "mixed", sizeof(last_generation_info.difficulty));

    return sentence;
}

const SentenceGenerationInfo *get_last_sentence_info(void) {
    return &last_generation_info;
}
