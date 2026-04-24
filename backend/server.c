#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

/* Include custom libraries */
#include "include/memory.h"
#include "include/mystring.h"
#include "include/math.h"

#define PORT 3000
#define BUFFER_SIZE 8192

void send_response(int client_socket, const char *header, const char *body) {
    write(client_socket, header, strlen(header));
    write(client_socket, body, strlen(body));
    close(client_socket);
}

void serve_file(int client_socket, const char *path, const char *content_type) {
    char full_path[256];
    sprintf(full_path, "frontend/%s", path);
    
    int fd = open(full_path, O_RDONLY);
    if (fd < 0) {
        send_response(client_socket, "HTTP/1.1 404 Not Found\r\n\r\n", "File not found");
        return;
    }

    char header[256];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", content_type);
    write(client_socket, header, strlen(header));

    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        write(client_socket, buffer, bytes_read);
    }
    close(fd);
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server running at http://localhost:%d\n", PORT);

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            continue;
        }

        char buffer[BUFFER_SIZE] = {0};
        read(client_socket, buffer, BUFFER_SIZE);

        if (strncmp(buffer, "GET / ", 6) == 0 || strncmp(buffer, "GET /index.html", 15) == 0) {
            serve_file(client_socket, "index.html", "text/html");
        } else if (strncmp(buffer, "GET /style.css", 14) == 0) {
            serve_file(client_socket, "style.css", "text/css");
        } else if (strncmp(buffer, "GET /app.js", 11) == 0) {
            serve_file(client_socket, "app.js", "application/javascript");
        } else if (strncmp(buffer, "GET /api/sentence", 17) == 0) {
            /* Generate sentence using custom logic */
            my_reset();
            char *sentence = generate_sentence(10);
            char json[1024];
            sprintf(json, "{\"sentence\": \"%s\"}", sentence);
            send_response(client_socket, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n", json);
        } else {
            send_response(client_socket, "HTTP/1.1 404 Not Found\r\n\r\n", "404 Not Found");
        }
    }

    return 0;
}
