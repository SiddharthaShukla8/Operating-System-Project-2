#ifndef MYSTRING_H
#define MYSTRING_H

typedef struct SentenceGenerationInfo {
    int requested_words;
    int generated_words;
    int sentence_length;
    int dataset_words;
    int duplicates_avoided;
    int punctuation_enabled;
    int numbers_enabled;
    int input_buffer_capacity;
    int input_buffer_usage;
    int session_memory_footprint;
    char difficulty[16];
} SentenceGenerationInfo;

int my_strlen(const char *s);
int my_strcmp(const char *a, const char *b);
int my_strncmp(const char *a, const char *b, int count);
char *my_strcpy(char *dest, const char *src);
char *my_strncpy(char *dest, const char *src, int count);
char *my_strcat(char *dest, const char *src);
char *my_strstr(const char *haystack, const char *needle);
int my_atoi(const char *text);
void my_itoa(int value, char *buffer);
int my_append_char(char *buffer, int *position, int limit, char value);
int my_append_text(char *buffer, int *position, int limit, const char *text);
int my_append_int(char *buffer, int *position, int limit, int value);
char *generate_sentence(int num_words);
char *generate_sentence_configured(int num_words, const char *difficulty, int punctuation_enabled, int numbers_enabled);
const SentenceGenerationInfo *get_last_sentence_info(void);

#endif
