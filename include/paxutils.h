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

#ifndef PAXUTILS_H
#define PAXUTILS_H

// Core Sub-utility function prototypes
int pax_cat(int argc, char **argv);
int pax_echo(int argc, char **argv);
int pax_pwd(int argc, char **argv);
int pax_truncate(int argc, char **argv);
int pax_cp(int argc, char **argv);
int pax_mv(int argc, char **argv);
int pax_mkdir(int argc, char **argv);
int pax_rm(int argc, char **argv);
int pax_rmdir(int argc, char **argv);
int pax_head(int argc, char **argv);
int pax_tail(int argc, char **argv);
int pax_sleep(int argc, char **argv);
int pax_ls(int argc, char **argv);
int pax_ln(int argc, char **argv);
int pax_clear(int argc, char **argv);
int pax_uname(int argc, char **argv);
int pax_yes(int argc, char **argv);
int pax_chmod(int argc, char **argv);
int pax_chown(int argc, char **argv);
int pax_grep(int argc, char **argv);
int pax_tee(int argc, char **argv);
int pax_ps(int argc, char **argv);
int pax_kill(int argc, char **argv);
int pax_env(int argc, char **argv);
int pax_touch(int argc, char **argv);
int pax_less(int argc, char **argv);

#endif