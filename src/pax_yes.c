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

#define BUF_SIZE 65536 // 64KB output buffer for maximum throughput

int pax_yes(int argc, char **argv) {
    const char *msg = "y";
    if (argc > 1) {
        msg = argv[1];
    }

    size_t len = strlen(msg);
    char buf[BUF_SIZE];
    size_t pos = 0;

    // Fill the 64KB buffer with repeated copies of "string\n"
    while (pos + len + 1 <= BUF_SIZE) {
        memcpy(buf + pos, msg, len);
        pos += len;
        buf[pos++] = '\n';
    }

    // Continuously write the chunked buffer directly to stdout
    while (true) {
        size_t total_written = 0;
        
        // Loop until the entire 64KB chunk is completely pushed into the pipe
        while (total_written < pos) {
            ssize_t written = write(STDOUT_FILENO, buf + total_written, pos - total_written);
            if (written < 0) {
                return 0; // Exit cleanly on closed pipe or error
            }
            if (written == 0) {
                break; // Prevent infinite stalls if nothing can be written right now
            }
            total_written += written;
        }
    }

    return 0;
}