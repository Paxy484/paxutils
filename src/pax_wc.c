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
#include <ctype.h>
#include "paxutils.h"

// Core metric aggregation layout
static void count_stream(FILE *stream, unsigned long *lines, unsigned long *words, unsigned long *bytes) {
    char buf[16384]; // Optimized 16-KiB processing chunk buffer to saturate CPU caches
    size_t bytes_read;
    bool in_word = false;

    while ((bytes_read = fread(buf, 1, sizeof(buf), stream)) > 0) {
        *bytes += bytes_read;
        for (size_t i = 0; i < bytes_read; i++) {
            char c = buf[i];

            // Rule of Modularity: Precision newline scanning
            if (c == '\n') {
                (*lines)++;
            }

            // Word isolation state tracking via standard character classes
            if (isspace((unsigned char)c)) {
                in_word = false;
            } else if (!in_word) {
                in_word = true;
                (*words)++;
            }
        }
    }
}

int pax_wc(int argc, char **argv) {
    unsigned long total_lines = 0;
    unsigned long total_words = 0;
    unsigned long total_bytes = 0;

    // Rule of Composition: If no files are specified, default to processing standard input (stdin)
    if (argc == 1) {
        count_stream(stdin, &total_lines, &total_words, &total_bytes);
        printf(" %7lu %7lu %7lu\n", total_lines, total_words, total_bytes);
        return 0;
    }

    int status = 0;
    bool multiple_files = (argc > 2);

    // Iterate over the target filename vector matrix array
    for (int i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
        if (!f) {
            perror(argv[i]); // Rule of Repair: Fail noisily if a target file vanishes
            status = 1;
            continue;
        }

        unsigned long file_lines = 0;
        unsigned long file_words = 0;
        unsigned long file_bytes = 0;

        count_stream(f, &file_lines, &file_words, &file_bytes);
        fclose(f);

        // Rule of Transparency: Format clean columns alongside the asset label
        printf(" %7lu %7lu %7lu %s\n", file_lines, file_words, file_bytes, argv[i]);

        total_lines += file_lines;
        total_words += file_words;
        total_bytes += file_bytes;
    }

    // Output the unified summation column row if scanning a batch allocation
    if (multiple_files) {
        printf(" %7lu %7lu %7lu total\n", total_lines, total_words, total_bytes);
    }

    return status;
}