#include "../include/json_loader.h"
#include "../include/memory.h"
#include "../include/mystring.h"
#include <fcntl.h>
#include <unistd.h>

typedef struct JsonCursor {
    const char *source;
    int length;
    int index;
} JsonCursor;

static WordDataset loaded_dataset;
static char last_error[160] = "dataset not loaded";
static int dataset_loaded = 0;

static void json_set_error(const char *message) {
    my_strncpy(last_error, message, sizeof(last_error));
}

static int json_is_whitespace(char value) {
    return value == ' ' || value == '\n' || value == '\r' || value == '\t';
}

static void json_skip_whitespace(JsonCursor *cursor) {
    while (cursor->index < cursor->length && json_is_whitespace(cursor->source[cursor->index])) {
        cursor->index++;
    }
}

static int json_consume_char(JsonCursor *cursor, char expected) {
    json_skip_whitespace(cursor);
    if (cursor->index >= cursor->length || cursor->source[cursor->index] != expected) {
        return 0;
    }
    cursor->index++;
    return 1;
}

static int json_parse_string(JsonCursor *cursor, char *destination, int destination_size) {
    int length = 0;

    json_skip_whitespace(cursor);
    if (cursor->index >= cursor->length || cursor->source[cursor->index] != '"') {
        return -1;
    }

    cursor->index++;
    while (cursor->index < cursor->length) {
        char current = cursor->source[cursor->index++];

        if (current == '"') {
            if (destination && destination_size > 0) {
                destination[length < destination_size ? length : destination_size - 1] = '\0';
            }
            return length;
        }

        if (current == '\\') {
            if (cursor->index >= cursor->length) return -1;
            current = cursor->source[cursor->index++];

            if (current == 'n') current = '\n';
            else if (current == 'r') current = '\r';
            else if (current == 't') current = '\t';
            else if (current == '"' || current == '\\' || current == '/') { }
            else return -1;
        }

        if (destination && destination_size > 0 && length < destination_size - 1) {
            destination[length] = current;
        }
        length++;
    }

    return -1;
}

static char *json_read_file(const char *path, int *file_size) {
    int file_descriptor;
    int size;
    int offset = 0;
    char *buffer;

    *file_size = 0;
    file_descriptor = open(path, O_RDONLY);
    if (file_descriptor < 0) {
        json_set_error("unable to open words.json");
        return 0;
    }

    size = (int)lseek(file_descriptor, 0, SEEK_END);
    if (size <= 0) {
        close(file_descriptor);
        json_set_error("words.json is empty");
        return 0;
    }

    lseek(file_descriptor, 0, SEEK_SET);
    buffer = (char *)memory_alloc_owned(size + 1, MEMORY_OWNER_TEMP);
    if (!buffer) {
        close(file_descriptor);
        json_set_error("memory allocation failed while loading json");
        return 0;
    }

    while (offset < size) {
        int bytes_read = (int)read(file_descriptor, buffer + offset, size - offset);
        if (bytes_read <= 0) {
            memory_free(buffer);
            close(file_descriptor);
            json_set_error("failed while reading words.json");
            return 0;
        }
        offset += bytes_read;
    }

    buffer[size] = '\0';
    close(file_descriptor);
    *file_size = size;
    return buffer;
}

static int json_find_words_array(JsonCursor *cursor) {
    char *words_key = my_strstr(cursor->source, "\"words\"");
    if (!words_key) {
        json_set_error("json missing words key");
        return 0;
    }

    cursor->index = (int)(words_key - cursor->source) + my_strlen("\"words\"");
    while (cursor->index < cursor->length && cursor->source[cursor->index] != '[') {
        cursor->index++;
    }

    if (cursor->index >= cursor->length) {
        json_set_error("json words array not found");
        return 0;
    }

    return 1;
}

int json_loader_init(const char *path) {
    JsonCursor cursor;
    JsonCursor counting_cursor;
    int file_size = 0;
    int word_count = 0;
    int shortest = 0;
    int longest = 0;
    int word_index = 0;
    char *file_buffer;

    if (dataset_loaded && loaded_dataset.count > 0) {
        return loaded_dataset.count;
    }

    file_buffer = json_read_file(path, &file_size);

    if (!file_buffer) return 0;

    cursor.source = file_buffer;
    cursor.length = file_size;
    cursor.index = 0;

    if (!json_find_words_array(&cursor) || !json_consume_char(&cursor, '[')) {
        memory_free(file_buffer);
        return 0;
    }

    counting_cursor = cursor;
    json_skip_whitespace(&counting_cursor);
    if (counting_cursor.index < counting_cursor.length && counting_cursor.source[counting_cursor.index] == ']') {
        memory_free(file_buffer);
        json_set_error("json words array is empty");
        return 0;
    }

    while (counting_cursor.index < counting_cursor.length) {
        int parsed_length = json_parse_string(&counting_cursor, 0, 0);
        if (parsed_length < 0) {
            memory_free(file_buffer);
            json_set_error("invalid word string in json");
            return 0;
        }

        word_count++;
        json_skip_whitespace(&counting_cursor);
        if (counting_cursor.index >= counting_cursor.length) break;
        if (counting_cursor.source[counting_cursor.index] == ',') {
            counting_cursor.index++;
            continue;
        }
        if (counting_cursor.source[counting_cursor.index] == ']') {
            break;
        }

        memory_free(file_buffer);
        json_set_error("invalid separator in words array");
        return 0;
    }

    if (word_count <= 0) {
        memory_free(file_buffer);
        json_set_error("no words parsed from dataset");
        return 0;
    }

    loaded_dataset.words = (char **)memory_alloc_owned((int)(sizeof(char *) * word_count), MEMORY_OWNER_PERSISTENT);
    if (!loaded_dataset.words) {
        memory_free(file_buffer);
        json_set_error("failed to allocate persistent word table");
        return 0;
    }

    loaded_dataset.count = word_count;
    loaded_dataset.shortest_word_length = 0;
    loaded_dataset.longest_word_length = 0;

    while (word_index < word_count) {
        JsonCursor word_cursor = cursor;
        int parsed_length = json_parse_string(&word_cursor, 0, 0);
        char *stored_word;

        if (parsed_length < 0) {
            memory_free(file_buffer);
            json_set_error("failed while parsing persistent words");
            return 0;
        }

        stored_word = (char *)memory_alloc_owned(parsed_length + 1, MEMORY_OWNER_PERSISTENT);
        if (!stored_word) {
            memory_free(file_buffer);
            json_set_error("failed to allocate persistent word");
            return 0;
        }

        if (json_parse_string(&cursor, stored_word, parsed_length + 1) < 0) {
            memory_free(file_buffer);
            json_set_error("failed while copying persistent word");
            return 0;
        }

        loaded_dataset.words[word_index] = stored_word;

        if (word_index == 0 || parsed_length < shortest) shortest = parsed_length;
        if (parsed_length > longest) longest = parsed_length;

        json_skip_whitespace(&cursor);
        if (cursor.index < cursor.length && cursor.source[cursor.index] == ',') {
            cursor.index++;
        }

        word_index++;
    }

    loaded_dataset.shortest_word_length = shortest;
    loaded_dataset.longest_word_length = longest;

    memory_free(file_buffer);
    dataset_loaded = 1;
    json_set_error("ok");
    return loaded_dataset.count;
}

const WordDataset *json_loader_get_dataset(void) {
    return loaded_dataset.count > 0 ? &loaded_dataset : 0;
}

const char *json_loader_last_error(void) {
    return last_error;
}
