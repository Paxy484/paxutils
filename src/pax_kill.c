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
#include <strings.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include "paxutils.h"

int pax_kill(int argc, char **argv) {
    // Rule of Repair: Fail noisily and early if host parameters are missing
    if (argc < 2) {
        fprintf(stderr, "usage: kill [-signal] pid...\n");
        return 1;
    }

    int sig = SIGTERM; // Default to standard graceful termination (Signal 15)
    int start_idx = 1;

    // Check if the user specified a custom target signal flag (e.g., "-9" or "-SIGKILL")
    if (argv[1][0] == '-') {
        char *sig_str = argv[1] + 1;

        if (isdigit((unsigned char)sig_str[0])) {
            // Numeric tracking array parsing (e.g., "-9")
            sig = atoi(sig_str);
        } else {
            // Textual alias mapping for the absolute essentials (Rule of Parsimony)
            if (strcasecmp(sig_str, "KILL") == 0 || strcasecmp(sig_str, "SIGKILL") == 0) {
                sig = SIGKILL; // Signal 9: Absolute, hardware-level unblockable termination
            } else if (strcasecmp(sig_str, "TERM") == 0 || strcasecmp(sig_str, "SIGTERM") == 0) {
                sig = SIGTERM; // Signal 15: Graceful stop signal
            } else if (strcasecmp(sig_str, "HUP") == 0 || strcasecmp(sig_str, "SIGHUP") == 0) {
                sig = SIGHUP;  // Signal 1: Hangup configuration vector reload
            } else if (strcasecmp(sig_str, "INT") == 0 || strcasecmp(sig_str, "SIGINT") == 0) {
                sig = SIGINT;  // Signal 2: Keyboard interrupt (Ctrl+C interception)
            } else {
                fprintf(stderr, "kill: unknown signal specification: '%s'\n", sig_str);
                return 1;
            }
        }
        start_idx = 2;
    }

    // Ensure we still have target PIDs to transmit signals to
    if (start_idx >= argc) {
        fprintf(stderr, "kill: missing process identifier operand\n");
        return 1;
    }

    int status = 0;
    // Iterate across the remaining target PID arguments array matrix
    for (int i = start_idx; i < argc; i++) {
        char *pid_str = argv[i];
        
        // Validate that the target PID string is entirely numeric before executing system loops
        for (int j = 0; pid_str[j] != '\0'; j++) {
            if (j == 0 && pid_str[j] == '-') continue; // Allow negative indexes for process groups
            if (!isdigit((unsigned char)pid_str[j])) {
                fprintf(stderr, "kill: invalid process identifier: '%s'\n", pid_str);
                status = 1;
                break;
            }
        }
        if (status == 1) continue;

        pid_t pid = (pid_t)atoi(pid_str);

        // Execute the direct kernel system call register trigger
        if (kill(pid, sig) != 0) {
            perror(argv[i]); // Scream loud if permission is denied or process vanishes
            status = 1;
        }
    }

    // Rule of Silence: Successful termination loops remain completely quiet
    return status;
}
