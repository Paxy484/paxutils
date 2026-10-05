/*
 * Copyright 2026 Rafael Peixoto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "paxutils.h"

#define BUFFER_SIZE 65536 // 64KB I/O buffer for high throughput

static int stream_fd(int src_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        ssize_t bytes_written = 0;
        while (bytes_written < bytes_read) {
            ssize_t res = write(STDOUT_FILENO, buffer + bytes_written, bytes_read - bytes_written);
            if (res < 0) {
                perror("cat: write error");
                return 1;
            }
            bytes_written += res;
        }
    }

    if (bytes_read < 0) {
        perror("cat: read error");
        return 1;
    }

    return 0;
}

int pax_cat(int argc, char **argv) {
    // Read from stdin if no file arguments provided
    if (argc <= 1) {
        return stream_fd(STDIN_FILENO);
    }

    int status = 0;
    int start_idx = 1;

    // Check for flags so they don't break our open() loop
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            // If it's a double dash '--', ignore it and start files after it
            if (argv[i][1] == '-' && argv[i][2] == '\0') {
                start_idx = i + 1;
                break;
            }
            // For now, reject other flags gracefully (like -n, -v)
            fprintf(stderr, "cat: invalid option '%s'\n", argv[i]);
            return 1;
        } else {
            // First argument that isn't a flag means file args have started
            start_idx = i;
            break;
        }
    }

    // If they typed flags but no actual files, default back to stdin
    if (start_idx >= argc) {
        return stream_fd(STDIN_FILENO);
    }

    for (int i = start_idx; i < argc; i++) {
        // Safe check for lone '-' string matching
        if (strcmp(argv[i], "-") == 0) {
            if (stream_fd(STDIN_FILENO) != 0) {
                status = 1;
            }
            continue;
        }

        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            perror(argv[i]);
            status = 1;
            continue;
        }

        if (stream_fd(fd) != 0) {
            status = 1;
        }
        close(fd);
    }

    return status;
}
