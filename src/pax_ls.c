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
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <stdbool.h>
#include <unistd.h>
#include "paxutils.h"

// Terminal color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_DIR     "\033[1;34m"
#define COLOR_EXEC    "\033[1;32m"

static void print_permissions(mode_t mode) {
    printf((S_ISDIR(mode)) ? "d" : (S_ISLNK(mode)) ? "l" : "-");
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
}

static void list_directory(const char *path, bool show_all, bool long_format) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror(path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files unless -a is set
        if (!show_all && entry->d_name[0] == '.') {
            continue;
        }

        char full_path[1024];
        size_t path_len = strlen(path);
        
        // Prevent duplicate slashes if path already ends in a slash
        if (path_len > 0 && path[path_len - 1] == '/') {
            snprintf(full_path, sizeof(full_path), "%s%s", path, entry->d_name);
        } else {
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        }

        struct stat st;
        if (lstat(full_path, &st) < 0) {
            continue;
        }

        if (long_format) {
            print_permissions(st.st_mode);
            printf(" %2lu", (unsigned long)st.st_nlink);

            struct passwd *pw = getpwuid(st.st_uid);
            struct group  *gr = getgrgid(st.st_gid);
            printf(" %s %s", pw ? pw->pw_name : "unknown", gr ? gr->gr_name : "unknown");

            // Base-10 width formatting flag
            printf(" %8lld", (long long)st.st_size);

            char timebuf[64];
            struct tm *tm_info = localtime(&st.st_mtime);
            strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);
            printf(" %s ", timebuf);
        }

        // Color coding output based on file type
        if (S_ISDIR(st.st_mode)) {
            printf(COLOR_DIR "%s" COLOR_RESET, entry->d_name);
        } else if (st.st_mode & S_IXUSR) {
            printf(COLOR_EXEC "%s" COLOR_RESET, entry->d_name);
        } else {
            printf("%s", entry->d_name);
        }

        if (long_format || !isatty(STDOUT_FILENO)) {
            printf("\n");
        } else {
            printf("  ");
        }
    }

    if (!long_format && isatty(STDOUT_FILENO)) {
        printf("\n");
    }

    closedir(dir);
}

int pax_ls(int argc, char **argv) {
    bool show_all = false;
    bool long_format = false;
    int start_idx = 1;

    while (start_idx < argc && argv[start_idx][0] == '-') {
        for (size_t j = 1; argv[start_idx][j] != '\0'; j++) {
            if (argv[start_idx][j] == 'a') show_all = true;
            if (argv[start_idx][j] == 'l') long_format = true;
        }
        start_idx++;
    }

    if (start_idx >= argc) {
        list_directory(".", show_all, long_format);
    } else {
        for (int i = start_idx; i < argc; i++) {
            if (argc - start_idx > 1) {
                printf("%s:\n", argv[i]);
            }
            list_directory(argv[i], show_all, long_format);
            if (i < argc - 1) printf("\n");
        }
    }

    return 0;
}
