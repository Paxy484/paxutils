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
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include "paxutils.h"

int pax_mv(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: mv src dst\n       mv src... dest_dir\n");
        return 1;
    }

    const char *target = argv[argc - 1];
    struct stat st;
    int is_dir = (stat(target, &st) == 0 && S_ISDIR(st.st_mode));

    if (argc > 3 && !is_dir) {
        fprintf(stderr, "mv: target '%s' is not a directory\n", target);
        return 1;
    }

    int status = 0;
    for (int i = 1; i < argc - 1; i++) {
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

        // Fast path: Atomic kernel rename
        if (rename(argv[i], dst_path) == 0) {
            continue;
        }

        // Fallback for cross-filesystem moves (EXDEV)
        if (errno == EXDEV) {
            char *cp_args[] = {"cp", argv[i], dst_path, NULL};
            if (pax_cp(3, cp_args) == 0) {
                if (unlink(argv[i]) != 0) {
                    perror(argv[i]);
                    status = 1;
                }
            } else {
                status = 1;
            }
        } else {
            perror(argv[i]);
            status = 1;
        }
    }

    return status;
}