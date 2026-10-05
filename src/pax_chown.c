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
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include "paxutils.h"

int pax_chown(int argc, char **argv) {
    // Rule of Repair: Fail noisily and early if arguments are missing
    if (argc < 3) {
        fprintf(stderr, "usage: chown user[:group] file...\n");
        return 1;
    }

    // TAUP Parameter Validation Gate: Stop immediately if inputs are blank or broken
    if (strlen(argv[1]) == 0 || strcmp(argv[1], ":") == 0) {
        fprintf(stderr, "chown: invalid user/group specification\n");
        return 1;
    }

    char *owner_group = argv[1];
    char *group_str = strchr(owner_group, ':');
    
    uid_t uid = (uid_t)-1; // -1 instructs the system call to leave UID unchanged
    gid_t gid = (gid_t)-1; // -1 instructs the system call to leave GID unchanged

    // Handle user/owner parsing if a string precedes the colon or no colon exists
    if (group_str != owner_group) {
        char user_buf[256];
        size_t user_len = group_str ? (size_t)(group_str - owner_group) : strlen(owner_group);
        if (user_len >= sizeof(user_buf)) {
            fprintf(stderr, "chown: username too long\n");
            return 1;
        }
        strncpy(user_buf, owner_group, user_len);
        user_buf[user_len] = '\0';

        struct passwd *pwd = getpwnam(user_buf);
        if (!pwd) {
            fprintf(stderr, "chown: unknown user: '%s'\n", user_buf);
            return 1;
        }
        uid = pwd->pw_uid;
    }

    // Handle group parsing if the colon separator is discovered
    if (group_str) {
        group_str++; // Step past the ':' character byte
        if (*group_str != '\0') {
            struct group *grp = getgrnam(group_str);
            if (!grp) {
                fprintf(stderr, "chown: unknown group: '%s'\n", group_str);
                return 1;
            }
            gid = grp->gr_gid;
        }
    }

    int status = 0;
    // Flush changes down the file list matrix array
    for (int i = 2; i < argc; i++) {
        if (chown(argv[i], uid, gid) != 0) {
            perror(argv[i]);
            status = 1;
        }
    }

    // Rule of Silence: Clean execution outputs zero text streams
    return status;
}