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
#include <stdbool.h>
#include <sys/utsname.h>
#include "paxutils.h"

int pax_uname(int argc, char **argv) {
    struct utsname buf;
    if (uname(&buf) < 0) {
        perror("uname");
        return 1;
    }

    bool print_sysname = false;  // -s
    bool print_nodename = false; // -n
    bool print_release = false;  // -r
    bool print_version = false;  // -v
    bool print_machine = false;  // -m

    if (argc == 1) {
        print_sysname = true; // Default with no args is -s
    } else {
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] == '-') {
                for (size_t j = 1; argv[i][j] != '\0'; j++) {
                    switch (argv[i][j]) {
                        case 'a':
                            print_sysname = print_nodename = print_release = true;
                            print_version = print_machine = true;
                            break;
                        case 's': print_sysname = true; break;
                        case 'n': print_nodename = true; break;
                        case 'r': print_release = true; break;
                        case 'v': print_version = true; break;
                        case 'm': print_machine = true; break;
                        default:
                            fprintf(stderr, "uname: invalid option -- '%c'\n", argv[i][j]);
                            return 1;
                    }
                }
            }
        }
    }

    // Default back to -s if options were passed but none matched s/n/r/v/m/a
    if (!print_sysname && !print_nodename && !print_release && !print_version && !print_machine) {
        print_sysname = true;
    }

    bool need_space = false;

    if (print_sysname) {
        printf("%s", buf.sysname);
        need_space = true;
    }
    if (print_nodename) {
        printf("%s%s", need_space ? " " : "", buf.nodename);
        need_space = true;
    }
    if (print_release) {
        printf("%s%s", need_space ? " " : "", buf.release);
        need_space = true;
    }
    if (print_version) {
        printf("%s%s", need_space ? " " : "", buf.version);
        need_space = true;
    }
    if (print_machine) {
        printf("%s%s", need_space ? " " : "", buf.machine);
    }
    printf("\n");

    return 0;
}