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
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "paxutils.h"

int pax_ln(int argc, char **argv) {
    bool symbolic = false;
    bool force = false;
    int start_idx = 1;

    while (start_idx < argc && argv[start_idx][0] == '-') {
        for (size_t j = 1; argv[start_idx][j] != '\0'; j++) {
            if (argv[start_idx][j] == 's') symbolic = true;
            if (argv[start_idx][j] == 'f') force = true;
        }
        start_idx++;
    }

    if (argc - start_idx < 2) {
        fprintf(stderr, "usage: ln [-s] [-f] target link_name\n");
        return 1;
    }

    const char *target = argv[start_idx];
    const char *link_name = argv[start_idx + 1];

    if (force) {
        unlink(link_name); // Remove existing destination if -f flag set
    }

    int ret = symbolic ? symlink(target, link_name) : link(target, link_name);
    if (ret < 0) {
        perror("ln");
        return 1;
    }

    return 0;
}