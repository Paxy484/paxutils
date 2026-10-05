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
#include <unistd.h>
#include <fcntl.h>
#include "paxutils.h"

int pax_truncate(int argc, char **argv) {
    // 1. Safe argument parsing using standard POSIX getopt
    int opt;
    off_t size = -1;
    int has_s_flag = 0;

    // Reset getopt environment in case main() called other commands first
    optind = 1; 

    while ((opt = getopt(argc, argv, "s:")) != -1) {
        switch (opt) {
            case 's':
                has_s_flag = 1;
                char *endptr;
                // Safe string-to-long-long conversion
                size = (off_t)strtoll(optarg, &endptr, 10);
                if (*endptr != '\0' || size < 0) {
                    fprintf(stderr, "truncate: invalid size '%s'\n", optarg);
                    return 1;
                }
                break;
            default:
                fprintf(stderr, "usage: truncate -s size file...\n");
                return 1;
        }
    }

    // If they didn't provide -s, or didn't provide a file path
    if (!has_s_flag || optind >= argc) {
        fprintf(stderr, "usage: truncate -s size file...\n");
        return 1;
    }

    int status = 0;
    // 2. Loop through all remaining file arguments safely
    for (int i = optind; i < argc; i++) {
        // Using O_RDWR ensures ftruncate never complains about permissions
        int fd = open(argv[i], O_RDWR | O_CREAT, 0666);
        if (fd < 0) {
            perror(argv[i]);
            status = 1;
            continue;
        }

        if (ftruncate(fd, size) < 0) {
            perror(argv[i]);
            status = 1;
        }
        close(fd);
    }

    return status;
}
