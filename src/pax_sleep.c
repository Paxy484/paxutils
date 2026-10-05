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
#include <termios.h>
#include <stdbool.h>
#include <signal.h>
#include "paxutils.h"

// Keep old terminal state global to this file so the signal handler can access it
static struct termios oldt;

// Signal handler to gracefully restore terminal state if Ctrl+C is pressed
static void handle_sigint(int sig) {
    (void)sig;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    exit(130); // 130 is the standard Unix exit status code for SIGINT (128 + 2)
}

// Read a single character instantly without waiting for newline
static void wait_for_keypress(void) {
    struct termios newt;
    
    // Save current configuration parameters
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    
    // Disable canonical mode (line buffering) and text echo
    newt.c_lflag &= ~(ICANON | ECHO);
    
    // Register the custom signal trap to prevent a broken terminal state on interrupt
    signal(SIGINT, handle_sigint);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    printf("Press any key to continue . . . ");
    fflush(stdout);

    getchar();
    printf("\n");

    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    // Restore default SIGINT behavior
    signal(SIGINT, SIG_DFL);
}

int pax_sleep(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: sleep [-w] [seconds]\n");
        return 1;
    }

    bool wait_key = false;
    int start_idx = 1;

    if (strcmp(argv[1], "-w") == 0 || strcmp(argv[1], "--wait") == 0) {
        wait_key = true;
        start_idx = 2;
    }

    // If -w is passed without seconds, wait indefinitely for keypress (DOS pause mode)
    if (wait_key && start_idx >= argc) {
        wait_for_keypress();
        return 0;
    }

    if (start_idx < argc) {
        int seconds = atoi(argv[start_idx]);
        if (seconds > 0) {
            sleep(seconds);
        }
    }

    if (wait_key) {
        wait_for_keypress();
    }

    return 0;
}
