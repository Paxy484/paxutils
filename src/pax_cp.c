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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include "paxutils.h"

#define BUFFER_SIZE 65536

static int copy_single_file(const char *src, const char *dst, bool force) {
    int src_fd = open(src, O_RDONLY);
    if (src_fd < 0) {
        perror(src);
        return 1;
    }

    struct stat st;
    if (fstat(src_fd, &st) < 0) {
        perror(src);
        close(src_fd);
        return 1;
    }

    int dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
    
    // If open fails and force flag is enabled, unlink the target and try again
    if (dst_fd < 0 && force) {
        unlink(dst); 
        dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
    }

    if (dst_fd < 0) {
        perror(dst);
        close(src_fd);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    int status = 0;

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        ssize_t bytes_written = 0;
        while (bytes_written < bytes_read) {
            ssize_t res = write(dst_fd, buffer + bytes_written, bytes_read - bytes_written);
            if (res < 0) {
                perror(dst);
                status = 1;
                break;
            }
            bytes_written += res;
        }
        if (status != 0) break;
    }

    if (bytes_read < 0) {
        perror(src);
        status = 1;
    }

    // Preserve original permissions
    fchmod(dst_fd, st.st_mode);

    close(src_fd);
    close(dst_fd);
    return status;
}

int pax_cp(int argc, char **argv) {
    bool force = false;
    int opt_idx = 1;

    // Zero-dependency POSIX flag loop handler
    while (opt_idx < argc && argv[opt_idx][0] == '-') {
        if (strcmp(argv[opt_idx], "--") == 0) {
            opt_idx++;
            break;
        }
        if (argv[opt_idx][1] == '\0') {
            break; // Stop if it's just a single dash filename
        }

        for (size_t j = 1; argv[opt_idx][j] != '\0'; j++) {
            if (argv[opt_idx][j] == 'f') {
                force = true;
            } else {
                fprintf(stderr, "cp: unknown option: '-%c'\n", argv[opt_idx][j]);
                return 1;
            }
        }
        opt_idx++;
    }

    // Count remaining non-flag arguments
    int file_argc = argc - opt_idx;
    if (file_argc < 2) {
        fprintf(stderr, "usage: cp [-f] src dst\n       cp [-f] src... dest_dir\n");
        return 1;
    }

    const char *target = argv[argc - 1];
    struct stat st;
    int is_dir = (stat(target, &st) == 0 && S_ISDIR(st.st_mode));

    // Multiple sources require destination to be a directory
    if (file_argc > 2 && !is_dir) {
        fprintf(stderr, "cp: target '%s' is not a directory\n", target);
        return 1;
    }

    int status = 0;
    for (int i = opt_idx; i < argc - 1; i++) {
        char dst_path[PATH_MAX];
        if (is_dir) {
            char src_copy[PATH_MAX];
            strncpy(src_copy, argv[i], sizeof(src_copy) - 1);
            src_copy[sizeof(src_copy) - 1] = '\0';
            snprintf(dst_path, sizeof(dst_path), "%s/%s", target, basename(src_copy));
        } else {
            strncpy(dst_path, target, sizeof(dst_path) - 1);
            dst_path[sizeof(dst_path) - 1] = '\0';
        }

        if (copy_single_file(argv[i], dst_path, force) != 0) {
            status = 1;
        }
    }

    return status;
}