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
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include "paxutils.h"

int pax_ps(int argc, char **argv) {
    // Rule of Silence: Silence positional argument constraints if passed incorrectly
    (void)argc;
    (void)argv;

    // Rule of Transparency: Print explicit, beautifully aligned structural column headers
    printf("%-7s %-16s %-5s %s\n", "PID", "COMMAND", "STATE", "PPID");

    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("ps: cannot open /proc");
        return 1;
    }

    struct dirent *entry;
    // Iterate over the live directory mapping loops inside the proc subsystem
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden navigation pointers and verify if directory name is entirely numeric
        if (entry->d_name[0] == '.') continue;
        
        bool is_pid = true;
        for (int i = 0; entry->d_name[i] != '\0'; i++) {
            if (!isdigit((unsigned char)entry->d_name[i])) {
                is_pid = false;
                break;
            }
        }
        if (!is_pid) continue;

        // Construct the precision absolute path string to access the process stats file
        char stat_path[PATH_MAX];
        snprintf(stat_path, sizeof(stat_path), "/proc/%s/stat", entry->d_name);

        FILE *f = fopen(stat_path, "r");
        if (!f) continue; // Skip tasks that vanish or escape during scanning loops

        char buf[2048];
        if (fgets(buf, sizeof(buf), f) != NULL) {
            char *open_paren = strchr(buf, '(');
            char *close_paren = strrchr(buf, ')');

            if (open_paren && close_paren && close_paren > open_paren) {
                // Precision Extract Token 1: The Process Identification Number (PID)
                *open_paren = '\0';
                int pid = atoi(buf);

                // Precision Extract Token 2: The Command string isolated safely within parentheses
                *close_paren = '\0';
                char *comm = open_paren + 1;

                // Step past the closing parenthesis to parse the tracking metadata block
                char *meta_ptr = close_paren + 1;
                char state;
                int ppid;

                // Parse out the remaining high-priority system state columns directly
                if (sscanf(meta_ptr, " %c %d", &state, &ppid) == 2) {
                    // Flush the finalized data layout directly down the terminal output stream
                    printf("%-7d %-16s %-5c %d\n", pid, comm, state, ppid);
                }
            }
        }
        fclose(f);
    }
    closedir(dir);

    return 0;
}