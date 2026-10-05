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
#include <unistd.h>
#include <termios.h>
#include "paxutils.h"

#define MAX_LINES 2048
#define MAX_LINE_LEN 2048
#define SCREEN_HEIGHT 24 // 25 lines minus 1 status footer cell row
#define COLS_LIMIT 85    // Strictly matches your Alacritty column matrix constraint

static struct termios orig_termios;

static void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    printf("\033[?25h\033[0m"); // Show cursor and clear formatting on task exit
}

static void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); // Kill line-buffering and text echo loops
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Ultra-Smart ANSI Character Length Estimator Engine
// Counts ONLY physical printing characters to prevent layout column mangling
static int get_printable_width(const char *str) {
    int count = 0;
    bool in_ansi = false;
    
    while (*str) {
        if (*str == '\033' || (*str == '\\' && *(str + 1) == '0' && *(str + 2) == '3' && *(str + 3) == '3')) {
            in_ansi = true;
            if (*str == '\\') str += 3; // Step past octal sequence indices
        } else if (in_ansi && *str == 'm') {
            in_ansi = false;
        } else if (!in_ansi) {
            count++;
        }
        str++;
    }
    return count;
}

// Precision Line Wrap Formatter Plane
// Auto-splits text loops right at the 85-column window mark without splitting ANSI words
static void append_smart_line(char **lines, int *total_lines, const char *raw_buf) {
    if (*total_lines >= MAX_LINES) return;

    int print_width = 0;
    char chunk[MAX_LINE_LEN];
    int c_idx = 0;
    bool in_ansi = false;

    for (int i = 0; raw_buf[i] != '\0'; i++) {
        char c = raw_buf[i];

        // Track state shifts inside ANSI escape color codes
        if (c == '\033') {
            in_ansi = true;
        }

        chunk[c_idx++] = c;

        if (in_ansi && c == 'm') {
            in_ansi = false;
        } else if (!in_ansi) {
            print_width++;
        }

        // AUTO-WRAP GATE: If printable characters hit your Alacritty 85-col limit, split the array
        if (print_width >= COLS_LIMIT) {
            // Append terminal color reset string to ensure style containment boundaries leak nothing
            strcpy(&chunk[c_idx], "\033[0m");
            chunk[c_idx + 4] = '\0';
            
            lines[*total_lines] = strdup(chunk);
            (*total_lines)++;
            if (*total_lines >= MAX_LINES) return;

            // Reset loop tracking indices for the upcoming word wrap chunk split
            print_width = 0;
            c_idx = 0;
            in_ansi = false;
        }
    }

    if (c_idx > 0) {
        chunk[c_idx] = '\0';
        lines[*total_lines] = strdup(chunk);
        (*total_lines)++;
    }
}

int pax_less(int argc, char **argv) {
    char *lines[MAX_LINES];
    int total_lines = 0;
    FILE *input = stdin;

    if (argc > 1) {
        input = fopen(argv[1], "r");
        if (!input) {
            perror(argv[1]);
            return 1;
        }
    }

    char buf[MAX_LINE_LEN];
    while (total_lines < MAX_LINES && fgets(buf, sizeof(buf), input) != NULL) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
        
        // Pass stream arrays down the smart layout wrap allocator
        append_smart_line(lines, &total_lines, buf);
    }

    if (input != stdin) fclose(input);
    if (total_lines == 0) return 0;

    enable_raw_mode();
    printf("\033[?25l"); // Hide blinking cursor during viewport screen refreshes

    int current_row = 0;
    bool running = true;

    while (running) {
        printf("\033[H"); // Home cursor position reset hack to kill screen tear

        // Render viewport layout lines bound to current timeline view offsets
        for (int i = 0; i < SCREEN_HEIGHT; i++) {
            int target_line = current_row + i;
            if (target_line < total_lines) {
                // Clear entire row buffer trailing blocks with ANSI \033[K
                printf("%s\033[K\n", lines[target_line]);
            } else {
                printf("~\033[K\n");
            }
        }

        // Render Pager Status Footer Menu Interface
        if (current_row + SCREEN_HEIGHT >= total_lines) {
            printf("\033[1;7m(END)\033[0m\033[K"); // Inverted cell rendering colors
        } else {
            printf("\033[1;7m:\033[0m\033[K");
        }
        fflush(stdout);

        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == 'j' || c == '\n') {
                if (current_row + SCREEN_HEIGHT < total_lines) current_row++;
            } else if (c == 'k') {
                if (current_row > 0) current_row--;
            } else if (c == ' ') {
                if (current_row + SCREEN_HEIGHT < total_lines) {
                    current_row += SCREEN_HEIGHT;
                } else {
                    current_row = total_lines - SCREEN_HEIGHT;
                    if (current_row < 0) current_row = 0;
                }
            } else if (c == 'b') {
                current_row -= SCREEN_HEIGHT;
                if (current_row < 0) current_row = 0;
            } else if (c == 'q') {
                running = false;
            }
        }
    }

    for (int i = 0; i < total_lines; i++) free(lines[i]);
    printf("\033[2J\033[H"); // Wipe console allocation plane area cleanly on task exit
    return 0;
}