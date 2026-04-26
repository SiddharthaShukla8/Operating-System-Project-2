#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>

#include "include/input_engine.h"
#include "include/json_loader.h"
#include "include/memory.h"
#include "include/mystring.h"

#define PORT 3000
#define BUFFER_SIZE 16384
#define HEADER_SIZE 256
#define JSON_BODY_SIZE 32768
#define FILE_CHUNK_SIZE 8192

static int starts_with(const char *text, const char *prefix) {
    return my_strncmp(text, prefix, my_strlen(prefix)) == 0;
}

static void socket_write_text(int client_socket, const char *text) {
    write(client_socket, text, my_strlen(text));
}

static void send_response(int client_socket, const char *status, const char *content_type, const char *body) {
    char header[HEADER_SIZE];

    snprintf(
        header,
        sizeof(header),
        "HTTP/1.1 %s\r\nContent-Type: %s\r\nCache-Control: no-store\r\n\r\n",
        status,
        content_type
    );

    socket_write_text(client_socket, header);
    socket_write_text(client_socket, body);
    close(client_socket);
}

static void serve_file(int client_socket, const char *path, const char *content_type) {
    char full_path[256];
    char header[HEADER_SIZE];
    char buffer[FILE_CHUNK_SIZE];
    int file_descriptor;
    int bytes_read;

    my_strcpy(full_path, "frontend/");
    my_strcat(full_path, path);

    file_descriptor = open(full_path, O_RDONLY);
    if (file_descriptor < 0) {
        send_response(client_socket, "404 Not Found", "text/plain", "File not found");
        return;
    }

    snprintf(
        header,
        sizeof(header),
        "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nCache-Control: no-store\r\n\r\n",
        content_type
    );

    socket_write_text(client_socket, header);
    while ((bytes_read = (int)read(file_descriptor, buffer, sizeof(buffer))) > 0) {
        write(client_socket, buffer, bytes_read);
    }

    close(file_descriptor);
    close(client_socket);
}

static int request_query_value(const char *request, const char *key, char *destination, int destination_size) {
    const char *path;
    int key_length;
    int index = 0;

    if (!request || !key || !destination || destination_size <= 0) return 0;

    destination[0] = '\0';
    path = request;
    key_length = my_strlen(key);

    while (path[index] != '\0' && path[index] != ' ' && path[index] != '\r' && path[index] != '\n') {
        int boundary = (path[index] == '?' || path[index] == '&');
        if (boundary && my_strncmp(path + index + 1, key, key_length) == 0 && path[index + key_length + 1] == '=') {
            int source_index = index + key_length + 2;
            int dest_index = 0;

            while (
                path[source_index] != '\0' &&
                path[source_index] != '&' &&
                path[source_index] != ' ' &&
                path[source_index] != '\r' &&
                path[source_index] != '\n'
            ) {
                if (dest_index < destination_size - 1) {
                    destination[dest_index++] = path[source_index] == '+' ? ' ' : path[source_index];
                }
                source_index++;
            }

            destination[dest_index] = '\0';
            return 1;
        }
        index++;
    }

    return 0;
}

static int request_query_int(const char *request, const char *key, int default_value, int minimum, int maximum) {
    char buffer[32];
    int value = default_value;

    if (request_query_value(request, key, buffer, sizeof(buffer))) {
        value = my_atoi(buffer);
    }

    if (value < minimum) value = minimum;
    if (value > maximum) value = maximum;
    return value;
}

static int request_query_bool(const char *request, const char *key) {
    char buffer[16];

    if (!request_query_value(request, key, buffer, sizeof(buffer))) {
        return 0;
    }

    return my_strcmp(buffer, "true") == 0 || my_strcmp(buffer, "1") == 0 || my_strcmp(buffer, "yes") == 0;
}

static void request_query_string(const char *request, const char *key, char *destination, int destination_size, const char *fallback) {
    if (!request_query_value(request, key, destination, destination_size)) {
        my_strncpy(destination, fallback, destination_size);
    }
}

static void append_json_escaped(char *buffer, int *position, int limit, const char *text) {
    int index = 0;

    if (!buffer || !position || !text) return;

    while (text[index] != '\0' && *position < limit - 1) {
        char current = text[index++];

        if (current == '"' || current == '\\') {
            my_append_char(buffer, position, limit, '\\');
            my_append_char(buffer, position, limit, current);
            continue;
        }

        if (current == '\n') {
            my_append_text(buffer, position, limit, "\\n");
            continue;
        }

        if (current == '\r') {
            my_append_text(buffer, position, limit, "\\r");
            continue;
        }

        if (current == '\t') {
            my_append_text(buffer, position, limit, "\\t");
            continue;
        }

        my_append_char(buffer, position, limit, current);
    }
}

static void append_json_bool(char *buffer, int *position, int limit, int value) {
    my_append_text(buffer, position, limit, value ? "true" : "false");
}

static void build_sentence_response(
    char *buffer,
    int limit,
    const char *sentence,
    const SentenceGenerationInfo *info,
    MemoryStats stats
) {
    int position = 0;

    buffer[0] = '\0';

    my_append_text(buffer, &position, limit, "{\"sentence\":\"");
    append_json_escaped(buffer, &position, limit, sentence);
    my_append_text(buffer, &position, limit, "\",\"meta\":{");

    my_append_text(buffer, &position, limit, "\"source\":\"backend json dataset\",");
    my_append_text(buffer, &position, limit, "\"requestedWords\":");
    my_append_int(buffer, &position, limit, info->requested_words);
    my_append_text(buffer, &position, limit, ",\"generatedWords\":");
    my_append_int(buffer, &position, limit, info->generated_words);
    my_append_text(buffer, &position, limit, ",\"sentenceLength\":");
    my_append_int(buffer, &position, limit, info->sentence_length);
    my_append_text(buffer, &position, limit, ",\"datasetWords\":");
    my_append_int(buffer, &position, limit, info->dataset_words);
    my_append_text(buffer, &position, limit, ",\"duplicatesAvoided\":");
    my_append_int(buffer, &position, limit, info->duplicates_avoided);
    my_append_text(buffer, &position, limit, ",\"difficulty\":\"");
    append_json_escaped(buffer, &position, limit, info->difficulty);
    my_append_text(buffer, &position, limit, "\",\"punctuationEnabled\":");
    append_json_bool(buffer, &position, limit, info->punctuation_enabled);
    my_append_text(buffer, &position, limit, ",\"numbersEnabled\":");
    append_json_bool(buffer, &position, limit, info->numbers_enabled);
    my_append_text(buffer, &position, limit, ",\"inputBufferCapacity\":");
    my_append_int(buffer, &position, limit, info->input_buffer_capacity);
    my_append_text(buffer, &position, limit, ",\"inputBufferUsage\":");
    my_append_int(buffer, &position, limit, info->input_buffer_usage);
    my_append_text(buffer, &position, limit, ",\"sessionMemoryFootprint\":");
    my_append_int(buffer, &position, limit, info->session_memory_footprint);

    my_append_text(buffer, &position, limit, ",\"memory\":{");
    my_append_text(buffer, &position, limit, "\"totalPool\":");
    my_append_int(buffer, &position, limit, stats.total_pool);
    my_append_text(buffer, &position, limit, ",\"usedBytes\":");
    my_append_int(buffer, &position, limit, stats.used_bytes);
    my_append_text(buffer, &position, limit, ",\"freeBytes\":");
    my_append_int(buffer, &position, limit, stats.free_bytes);
    my_append_text(buffer, &position, limit, ",\"allocations\":");
    my_append_int(buffer, &position, limit, stats.allocations);
    my_append_text(buffer, &position, limit, ",\"deallocations\":");
    my_append_int(buffer, &position, limit, stats.deallocations);
    my_append_text(buffer, &position, limit, ",\"activeAllocations\":");
    my_append_int(buffer, &position, limit, stats.active_allocations);
    my_append_text(buffer, &position, limit, ",\"fragmentationBlocks\":");
    my_append_int(buffer, &position, limit, stats.fragmentation_blocks);
    my_append_text(buffer, &position, limit, ",\"largestFreeBlock\":");
    my_append_int(buffer, &position, limit, stats.largest_free_block);
    my_append_text(buffer, &position, limit, ",\"sessionAllocations\":");
    my_append_int(buffer, &position, limit, stats.session_allocations);
    my_append_text(buffer, &position, limit, ",\"sessionDeallocations\":");
    my_append_int(buffer, &position, limit, stats.session_deallocations);
    my_append_text(buffer, &position, limit, ",\"sessionUsedBytes\":");
    my_append_int(buffer, &position, limit, stats.session_used_bytes);
    my_append_text(buffer, &position, limit, "}}}");
}

static void handle_sentence_request(int client_socket, const char *request) {
    char difficulty[16];
    char body[JSON_BODY_SIZE];
    int word_count = request_query_int(request, "count", 25, 1, 300);
    int punctuation_enabled = request_query_bool(request, "punctuation");
    int numbers_enabled = request_query_bool(request, "numbers");
    char *sentence;
    const SentenceGenerationInfo *info;
    MemoryStats stats;

    request_query_string(request, "difficulty", difficulty, sizeof(difficulty), "mixed");

    memory_reset();
    input_engine_reset_session();

    sentence = generate_sentence_configured(word_count, difficulty, punctuation_enabled, numbers_enabled);
    if (!sentence) {
        send_response(client_socket, "500 Internal Server Error", "application/json", "{\"error\":\"sentence generation failed\"}");
        return;
    }

    info = get_last_sentence_info();
    stats = memory_usage();
    build_sentence_response(body, sizeof(body), sentence, info, stats);
    send_response(client_socket, "200 OK", "application/json", body);
}

int main(void) {
    int server_fd;
    int client_socket;
    int opt = 1;
    int addrlen;
    int port = PORT;
    struct sockaddr_in address;
    const char *port_value = getenv("PORT");

    if (port_value && my_atoi(port_value) > 0) {
        port = my_atoi(port_value);
    }

    memory_init();
    srand((unsigned int)time(0));

    if (json_loader_init("backend/data/words.json") < 500) {
        fprintf(stderr, "Failed to load backend/data/words.json: %s\n", json_loader_last_error());
        return EXIT_FAILURE;
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        return EXIT_FAILURE;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    addrlen = sizeof(address);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        return EXIT_FAILURE;
    }

    printf("Server running at http://localhost:%d\n", port);
    printf("Loaded %d words from backend/data/words.json\n", json_loader_get_dataset()->count);

    while (1) {
        char request[BUFFER_SIZE];
        int bytes_read;

        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_socket < 0) {
            continue;
        }

        bytes_read = (int)read(client_socket, request, sizeof(request) - 1);
        if (bytes_read <= 0) {
            close(client_socket);
            continue;
        }

        request[bytes_read] = '\0';

        if (starts_with(request, "GET / ") || starts_with(request, "GET /index.html")) {
            serve_file(client_socket, "index.html", "text/html");
        } else if (starts_with(request, "GET /style.css")) {
            serve_file(client_socket, "style.css", "text/css");
        } else if (starts_with(request, "GET /app.js")) {
            serve_file(client_socket, "app.js", "application/javascript");
        } else if (starts_with(request, "GET /api/sentence")) {
            handle_sentence_request(client_socket, request);
        } else {
            send_response(client_socket, "404 Not Found", "text/plain", "404 Not Found");
        }
    }

    return 0;
}
