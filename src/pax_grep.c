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
#include <stdbool.h>
#include "paxutils.h"

// Precision line scanner engine
static void scan_stream(FILE *stream, const char *pattern, const char *filename, bool show_filename) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned long line_num = 0;

    // Use high-performance POSIX getline to stream character arrays dynamically
    while ((read = getline(&line, &len, stream)) != -1) {
        line_num++;
        
        // Scan the raw memory block for the pattern pointer string
        if (strstr(line, pattern) != NULL) {
            if (show_filename && filename != NULL) {
                printf("%s:", filename);
            }
            // Rule of Transparency: Output the clean matching string data
            printf("%lu:%s", line_num, line);
        }
    }
    free(line); // Free the localized tracking stack buffer allocation
}

int pax_grep(int argc, char **argv) {
    // Fail noisily and early if arguments are missing (Rule of Repair)
    if (argc < 2) {
        fprintf(stderr, "usage: grep pattern [file...]\n");
        return 1;
    }

    const char *pattern = argv[1];
    
    // Rule of Composition: If no files are specified, default to scanning standard input (stdin)
    if (argc == 2) {
        scan_stream(stdin, pattern, NULL, false);
        return 0;
    }

    int status = 0;
    bool multiple_files = (argc > 3);

    // Iterate over the target filename arguments matrix array
    for (int i = 2; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
        if (!f) {
            perror(argv[i]); // Speak up via standard kernel error macros if target vanishes
            status = 1;
            continue;
        }
        
        scan_stream(f, pattern, argv[i], multiple_files);
        fclose(f);
    }

    return status;
}