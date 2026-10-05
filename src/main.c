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

#include <stdio.h>
#include <string.h>
#include "paxutils.h"

int main(int argc, char **argv) {
    // Defensive check against empty/malicious execution environments
    if (argc == 0 || argv == NULL || argv[0] == NULL) {
        return 1;
    }

    // Pure pointer arithmetic replacement for basename()
    // Finds the last forward slash inside the binary's execution name string
    char *prog = strrchr(argv[0], '/');
    if (prog != NULL) {
        prog++; // Move past the slash to isolate the binary name
    } else {
        prog = argv[0]; // No slash found, the raw name is already the basename
    }

    // Symlink / Multi-call routing block
    if (strcmp(prog, "cat") == 0)      return pax_cat(argc, argv);
    if (strcmp(prog, "echo") == 0)     return pax_echo(argc, argv);
    if (strcmp(prog, "pwd") == 0)      return pax_pwd(argc, argv);
    if (strcmp(prog, "truncate") == 0) return pax_truncate(argc, argv);
    if (strcmp(prog, "cp") == 0)       return pax_cp(argc, argv);
    if (strcmp(prog, "mv") == 0)       return pax_mv(argc, argv);
    if (strcmp(prog, "mkdir") == 0)    return pax_mkdir(argc, argv);
    if (strcmp(prog, "rm") == 0)       return pax_rm(argc, argv);
    if (strcmp(prog, "rmdir") == 0)    return pax_rmdir(argc, argv);
    if (strcmp(prog, "head") == 0)     return pax_head(argc, argv);
    if (strcmp(prog, "tail") == 0)     return pax_tail(argc, argv);
    if (strcmp(prog, "sleep") == 0)    return pax_sleep(argc, argv);
    if (strcmp(prog, "ls") == 0)       return pax_ls(argc, argv);
    if (strcmp(prog, "ln") == 0)       return pax_ln(argc, argv);
    if (strcmp(prog, "clear") == 0)    return pax_clear(argc, argv);
    if (strcmp(prog, "uname") == 0)    return pax_uname(argc, argv);
    if (strcmp(prog, "yes") == 0)      return pax_yes(argc, argv);
    if (strcmp(prog, "chmod") == 0)    return pax_chmod(argc, argv);
    if (strcmp(prog, "chown") == 0)    return pax_chown(argc, argv);
    if (strcmp(prog, "grep") == 0)     return pax_grep(argc, argv);
    if (strcmp(prog, "tee") == 0)      return pax_tee(argc, argv);
    if (strcmp(prog, "ps") == 0)       return pax_ps(argc, argv);
    if (strcmp(prog, "kill") == 0)     return pax_kill(argc, argv);
    if (strcmp(prog, "env") == 0)      return pax_env(argc, argv);
    if (strcmp(prog, "touch") == 0)    return pax_touch(argc, argv);
    if (strcmp(prog, "less") == 0)     return pax_less(argc, argv);

    // Direct invocation mode fallback: ./paxutils <command> [args]
    if (argc > 1 && argv[1] != NULL) {
        if (strcmp(argv[1], "cat") == 0)      return pax_cat(argc - 1, argv + 1);
        if (strcmp(argv[1], "echo") == 0)     return pax_echo(argc - 1, argv + 1);
        if (strcmp(argv[1], "pwd") == 0)      return pax_pwd(argc - 1, argv + 1);
        if (strcmp(argv[1], "truncate") == 0) return pax_truncate(argc - 1, argv + 1);
        if (strcmp(argv[1], "cp") == 0)       return pax_cp(argc - 1, argv + 1);
        if (strcmp(argv[1], "mv") == 0)       return pax_mv(argc - 1, argv + 1);
        if (strcmp(argv[1], "mkdir") == 0)    return pax_mkdir(argc - 1, argv + 1);
        if (strcmp(argv[1], "rm") == 0)       return pax_rm(argc - 1, argv + 1);
        if (strcmp(argv[1], "rmdir") == 0)    return pax_rmdir(argc - 1, argv + 1);
        if (strcmp(argv[1], "head") == 0)     return pax_head(argc - 1, argv + 1);
        if (strcmp(argv[1], "tail") == 0)     return pax_tail(argc - 1, argv + 1);
        if (strcmp(argv[1], "sleep") == 0)    return pax_sleep(argc - 1, argv + 1);
        if (strcmp(argv[1], "ls") == 0)       return pax_ls(argc - 1, argv + 1);
        if (strcmp(argv[1], "ln") == 0)       return pax_ln(argc - 1, argv + 1);
        if (strcmp(argv[1], "clear") == 0)    return pax_clear(argc - 1, argv + 1);
        if (strcmp(argv[1], "uname") == 0)    return pax_uname(argc - 1, argv + 1);
        if (strcmp(argv[1], "yes") == 0)      return pax_yes(argc - 1, argv + 1);
        if (strcmp(argv[1], "chmod") == 0)    return pax_chmod(argc - 1, argv + 1);
        if (strcmp(argv[1], "chown") == 0)    return pax_chown(argc - 1, argv + 1);
        if (strcmp(argv[1], "grep") == 0)     return pax_grep(argc - 1, argv + 1);
        if (strcmp(argv[1], "tee") == 0)      return pax_tee(argc - 1, argv + 1);
        if (strcmp(argv[1], "ps") == 0)       return pax_ps(argc - 1, argv + 1);
        if (strcmp(argv[1], "kill") == 0)     return pax_kill(argc - 1, argv + 1);
        if (strcmp(argv[1], "env") == 0)      return pax_env(argc - 1, argv + 1);
        if (strcmp(argv[1], "touch") == 0)    return pax_touch(argc - 1, argv + 1);
        if (strcmp(argv[1], "less") == 0)     return pax_less(argc - 1, argv + 1);
    }

    fprintf(stderr, "paxutils: unknown command '%s'\n", prog); //
    return 1;
}