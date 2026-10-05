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

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "paxutils.h"

int pax_tee(int argc, char **argv) {
    bool append_mode = false;
    int start_idx = 1;

    // Rule of Repair: Check for standard flags early and noisily if malformed
    while (start_idx < argc && argv[start_idx][0] == '-') {
        // Handle standard -a (append) or long-form --append flags
        if (strcmp(argv[start_idx], "-a") == 0 || strcmp(argv[start_idx], "--append") == 0) {
            append_mode = true;
        } else {
            fprintf(stderr, "tee: invalid option: '%s'\n", argv[start_idx]);
            fprintf(stderr, "usage: tee [-a] [file...]\n");
            return 1;
        }
        start_idx++;
    }

    int num_files = argc - start_idx;
    FILE **files = NULL;

    // If target files are specified, safely allocate a local tracking stack array
    if (num_files > 0) {
        files = malloc(num_files * sizeof(FILE *));
        if (!files) {
            perror("tee: memory allocation failed");
            return 1;
        }

        const char *mode = append_mode ? "a" : "w";
        for (int i = 0; i < num_files; i++) {
            files[i] = fopen(argv[start_idx + i], mode);
            if (!files[i]) {
                perror(argv[start_idx + i]); // Fail noisily if file node is locked
                // Clean up previously opened file nodes before escaping
                for (int j = 0; j < i; j++) {
                    fclose(files[j]);
                }
                free(files);
                return 1;
            }
        }
    }

    // High-performance 4-KiB stream chunk buffer array to maximize data throughput
    char buffer[4096];
    size_t bytes_read;

    // Continuous processing plane loop reading straight from standard input (stdin)
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), stdin)) > 0) {
        // Direct Split Step 1: Echo bytes right down standard output (stdout)
        if (fwrite(buffer, 1, bytes_read, stdout) != bytes_read) {
            fprintf(stderr, "tee: error writing to stdout\n");
            break;
        }

        // Direct Split Step 2: Flush copies down across all file pointer outputs simultaneously
        for (int i = 0; i < num_files; i++) {
            if (fwrite(buffer, 1, bytes_read, files[i]) != bytes_read) {
                perror("tee: file write error");
                break;
            }
        }
    }

    // Graceful teardown cleanup loop
    if (num_files > 0) {
        for (int i = 0; i < num_files; i++) {
            fclose(files[i]);
        }
        free(files);
    }

    // Rule of Silence: Success outputs zero extra administrative text
    return 0;
}