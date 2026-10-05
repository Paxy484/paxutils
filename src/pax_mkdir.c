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
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include "paxutils.h"

// High-performance recursive directory mapper loop
static int create_dir_recursive(const char *path) {
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (len == 0) return 0;

    // Strip trailing slash to preserve correct path geometry boundaries
    if (tmp[len - 1] == '/') tmp[len - 1] = '\0';

    // Traverse down the path string allocation array free of heap thrashing
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            // Standard POSIX behavior: create parents and ignore EEXIST errors
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                perror(tmp);
                return 1;
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        perror(tmp);
        return 1;
    }

    return 0;
}

int pax_mkdir(int argc, char **argv) {
    // Rule of Repair: Fail noisily and early if arguments are missing
    if (argc < 2) {
        fprintf(stderr, "usage: mkdir [-p | -P | --panic] directory...\n");
        return 1;
    }

    bool panic_on_exist = false;
    bool standard_recursive = false;
    int start_idx = 1;

    // Clean Parameter Routing Interface Gate
    while (start_idx < argc && argv[start_idx][0] == '-') {
        if (strcmp(argv[start_idx], "-p") == 0) {
            standard_recursive = true;
        } else if (strcmp(argv[start_idx], "-P") == 0 || strcmp(argv[start_idx], "--panic") == 0) {
            panic_on_exist = true;
        } else {
            fprintf(stderr, "mkdir: invalid option: '%s'\n", argv[start_idx]);
            fprintf(stderr, "usage: mkdir [-p | -P | --panic] directory...\n");
            return 1;
        }
        start_idx++;
    }

    if (start_idx >= argc) {
        fprintf(stderr, "mkdir: missing directory operand\n");
        return 1;
    }

    int status = 0;
    // Iterate through the target file operand array matrix list
    for (int i = start_idx; i < argc; i++) {
        if (panic_on_exist) {
            // CRITICAL SYSTEM GATE: Direct kernel call that throws a hard error if EEXIST triggers
            if (mkdir(argv[i], 0755) != 0) {
                perror(argv[i]); // Fail noisily immediately!
                status = 1;
            }
        } else if (standard_recursive) {
            // Standard POSIX compilation-safe execution path string
            if (create_dir_recursive(argv[i]) != 0) {
                status = 1;
            }
        } else {
            // Raw single directory node creation block
            if (mkdir(argv[i], 0755) != 0) {
                perror(argv[i]);
                status = 1;
            }
        }
    }

    return status;
}