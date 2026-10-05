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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "paxutils.h"

int pax_touch(int argc, char **argv) {
    // Rule of Repair: Fail noisily and early if arguments are missing
    if (argc < 2) {
        fprintf(stderr, "usage: touch file...\n");
        return 1;
    }

    int status = 0;

    // Iterate across the remaining target filename arguments matrix array
    for (int i = 1; i < argc; i++) {
        // Step 1: Force immediate file record initialization if path does not exist
        // Open with O_CREAT (create if missing), O_WRONLY (write-only interface), and standard 0644 bits
        int fd = open(argv[i], O_CREAT | O_WRONLY, 0644);
        if (fd < 0) {
            perror(argv[i]); // Scream loud via system macros if path access is blocked
            status = 1;
            continue;
        }
        close(fd); // Instantly drop the descriptor block to keep system limits clean

        // Step 2: Flush the current microsecond timestamp down the filesystem matrix registers
        // Passing NULL as the timespec array instructs the kernel to use the current system clock cycle
        if (utimensat(AT_FDCWD, argv[i], NULL, 0) != 0) {
            perror(argv[i]);
            status = 1;
        }
    }

    // Rule of Silence: Successful timestamp updates output zero administrative text
    return status;
}