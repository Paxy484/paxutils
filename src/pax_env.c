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
#include <unistd.h>
#include <stdbool.h>
#include "paxutils.h"

// Access the global operating system environment pointer array close to the metal
extern char **environ;

int pax_env(int argc, char **argv) {
    // Rule 1: The Viewer Plane (No arguments passed)
    if (argc == 1) {
        // Iterate down the global string vector matrix until hitting the null terminator
        for (char **env = environ; *env != NULL; env++) {
            // Rule of Transparency: Stream out the raw configuration pairs cleanly
            printf("%s\n", *env);
        }
        return 0;
    }

    int arg_idx = 1;
    bool clear_env = false;

    // Rule of Repair: Check for standard flags early and noisily if malformed
    if (argc > 1 && (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--ignore-environment") == 0)) {
        clear_env = true;
        arg_idx = 2;
    }

    // If running in isolated mode, we wipe the environment frame immediately
    // BEFORE tracking local override assignments to prevent leaking parent data
    if (clear_env) {
        // THE AIRTIGHT C99 WORKAROUND: Direct variable pointer assignment to NULL.
        // This clears the entire environment block instantly close to the bare metal,
        // bypassing glibc's hidden macro check constraints entirely!
        environ = NULL; 
    }

    // Step 1: Handle localized temporary variable modifications (e.g., env VAR=VAL command)
    while (arg_idx < argc && strchr(argv[arg_idx], '=') != NULL) {
        char *assignment = argv[arg_idx];
        char *equal_sign = strchr(assignment, '=');
        
        *equal_sign = '\0';
        char *name = assignment;
        char *value = equal_sign + 1;

        // Modify, append, or populate changes straight onto the active vector stream
        setenv(name, value, 1);
        
        *equal_sign = '='; // Restore string geometry for tracking logs
        arg_idx++;
    }

    // Step 2: The Execution Plane
    if (arg_idx >= argc) {
        // If variable modifiers were passed but no trailing target binary was defined, dump the modified map
        if (environ != NULL) {
            for (char **env = environ; *env != NULL; env++) {
                printf("%s\n", *env);
            }
        }
        return 0;
    }

    // Direct Kernel Hand-off: Replace the current process image with the target binary loop
    execvp(argv[arg_idx], &argv[arg_idx]);

    // If execvp returns, an execution failure occurred (e.g., binary not discovered in PATH)
    perror(argv[arg_idx]);
    return 1;
}