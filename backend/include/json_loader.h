#ifndef JSON_LOADER_H
#define JSON_LOADER_H

typedef struct WordDataset {
    char **words;
    int count;
    int shortest_word_length;
    int longest_word_length;
} WordDataset;

int json_loader_init(const char *path);
const WordDataset *json_loader_get_dataset(void);
const char *json_loader_last_error(void);

#endif
