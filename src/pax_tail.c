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
#include "paxutils.h"

static void tail_stream(FILE *fp, int lines) {
    char **line_buf = calloc(lines, sizeof(char *));
    char *line = NULL;
    size_t len = 0;
    int total_lines = 0;

    while (getline(&line, &len, fp) != -1) {
        int idx = total_lines % lines;
        free(line_buf[idx]);
        line_buf[idx] = strdup(line);
        total_lines++;
    }
    free(line);

    int start = (total_lines > lines) ? (total_lines % lines) : 0;
    int count = (total_lines > lines) ? lines : total_lines;

    for (int i = 0; i < count; i++) {
        int idx = (start + i) % lines;
        if (line_buf[idx]) {
            fputs(line_buf[idx], stdout);
            free(line_buf[idx]);
        }
    }
    free(line_buf);
}

int pax_tail(int argc, char **argv) {
    int lines = 10;
    int start_idx = 1;

    if (argc > 2 && strcmp(argv[1], "-n") == 0) {
        lines = atoi(argv[2]);
        start_idx = 3;
    }

    if (start_idx >= argc) {
        tail_stream(stdin, lines);
        return 0;
    }

    int status = 0;
    for (int i = start_idx; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (!fp) {
            perror(argv[i]);
            status = 1;
            continue;
        }
        tail_stream(fp, lines);
        fclose(fp);
    }

    return status;
}