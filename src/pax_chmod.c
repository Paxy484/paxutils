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
#include <string.h>
#include "paxutils.h"

int pax_chmod(int argc, char **argv) {
    // Rule of Repair: Fail noisily and early if arguments are missing
    if (argc < 3) {
        fprintf(stderr, "usage: chmod [octal_mode | +/-x] file...\n");
        return 1;
    }

    char *mode_str = argv[1];
    int is_symbolic = (mode_str[0] == '+' || mode_str[0] == '-');
    mode_t symbolic_mode = 0;

    // 1. Handle Symbolic Notation Layer (+x or -x)
    if (is_symbolic) {
        if (strcmp(mode_str, "+x") == 0 || strcmp(mode_str, "-x") == 0) {
            // Standard POSIX bitwise masks for User, Group, and Other execute permissions
            symbolic_mode = S_IXUSR | S_IXGRP | S_IXOTH;
        } else {
            fprintf(stderr, "chmod: unsupported symbolic mode: '%s'\n", mode_str);
            return 1;
        }
    } 
    // 2. Handle Octal Notation Layer (e.g., 0755)
    else {
        char *endptr;
        long mode_val = strtol(mode_str, &endptr, 8);
        if (*endptr != '\0' || mode_val < 0 || mode_val > 07777) {
            fprintf(stderr, "chmod: invalid octal mode: '%s'\n", mode_str);
            return 1;
        }
        symbolic_mode = (mode_t)mode_val;
    }

    int status = 0;
    
    // Iterate over all target file arguments passed down the command array
    for (int i = 2; i < argc; i++) {
        mode_t target_mode = 0;

        if (is_symbolic) {
            struct stat st;
            // Fetch the file's current filesystem attributes 
            if (stat(argv[i], &st) != 0) {
                perror(argv[i]);
                status = 1;
                continue;
            }
            
            // Mask out the file type bits to only keep permission bits
            mode_t current_perms = st.st_mode & 07777;

            if (mode_str[0] == '+') {
                target_mode = current_perms | symbolic_mode;  // Bitwise OR to ADD execute bits
            } else {
                target_mode = current_perms & ~symbolic_mode; // Bitwise AND-NOT to STRIP execute bits
            }
        } else {
            target_mode = symbolic_mode; // Use raw octal mode assignment directly
        }

        // Execute the direct kernel system call
        if (chmod(argv[i], target_mode) != 0) {
            perror(argv[i]);
            status = 1; // Explicitly flag to the parent shell that an error occurred
        }
    }

    // Rule of Silence: If everything compiles and shifts perfectly, return 0 quietly
    return status;
}