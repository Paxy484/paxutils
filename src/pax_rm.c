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
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include "paxutils.h"

static int remove_recursive(const char *path, bool force) {
    struct stat st;
    if (lstat(path, &st) < 0) {
        if (!force) perror(path);
        return force ? 0 : 1;
    }

    if (!S_ISDIR(st.st_mode)) {
        if (unlink(path) < 0 && !force) {
            perror(path);
            return 1;
        }
        return 0;
    }

    DIR *d = opendir(path);
    if (!d) {
        if (!force) perror(path);
        return force ? 0 : 1;
    }

    struct dirent *p;
    int status = 0;
    while ((p = readdir(d)) != NULL) {
        if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0) continue;

        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "%s/%s", path, p->d_name);
        if (remove_recursive(buf, force) != 0) status = 1;
    }
    closedir(d);

    if (rmdir(path) < 0 && !force) {
        perror(path);
        return 1;
    }

    return status;
}

int pax_rm(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: rm [-rf] file...\n");
        return 1;
    }

    bool recursive = false;
    bool force = false;
    int start_idx = 1;

    while (start_idx < argc && argv[start_idx][0] == '-') {
        // Handle explicit option terminator "--"
        if (strcmp(argv[start_idx], "--") == 0) {
            start_idx++;
            break;
        }
        // If it's just a single "-", it's a filename parameter, not an option flag
        if (argv[start_idx][1] == '\0') {
            break;
        }

        for (size_t j = 1; argv[start_idx][j] != '\0'; j++) {
            if (argv[start_idx][j] == 'r' || argv[start_idx][j] == 'R') recursive = true;
            if (argv[start_idx][j] == 'f') force = true;
        }
        start_idx++;
    }

    int status = 0;
    for (int i = start_idx; i < argc; i++) {
        // --- RAFAEL'S ANTI-ROOT DELETION SHIELD ---
        // Strip trailing slashes to catch sneaky inputs like "///" or "//"
        size_t len = strlen(argv[i]);
        while (len > 1 && argv[i][len - 1] == '/') {
            len--;
        }

        // Check if the argument is targeting the root directory
        if (len == 1 && argv[i][0] == '/') {
            fprintf(stderr, "No.\n");
            status = 1;
            continue;
        }

        struct stat st;
        if (lstat(argv[i], &st) < 0) {
            if (!force) {
                perror(argv[i]);
                status = 1;
            }
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (!recursive) {
                fprintf(stderr, "rm: %s: is a directory\n", argv[i]);
                status = 1;
                continue;
            }
            if (remove_recursive(argv[i], force) != 0) status = 1;
        } else {
            if (unlink(argv[i]) < 0 && !force) {
                perror(argv[i]);
                status = 1;
            }
        }
    }

    return status;
}