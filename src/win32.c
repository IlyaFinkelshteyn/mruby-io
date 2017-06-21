/* MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "mruby.h"
#include "mruby/string.h"
#include "mruby/data.h"
#include "io.h"

#include <windows.h>
#include <process.h>
#include <errno.h>

#include <tchar.h>
#include <stdio.h>

#define BUFSIZE 4096

#define MAXCHILDNUM 256 /* max num of child processes */

#ifndef P_OVERLAY
# define P_OVERLAY 2
#endif

#ifndef P_NOWAIT
# define P_NOWAIT 1
#endif

/* License: Ruby's */
static struct ChildRecord {
    HANDLE hProcess;
    pid_t pid;
} ChildRecord[MAXCHILDNUM];

/* License: Ruby's */
#define FOREACH_CHILD(v) do { \
    struct ChildRecord* v; \
    for (v = ChildRecord; v < ChildRecord + sizeof(ChildRecord) / sizeof(ChildRecord[0]); ++v)
#define END_FOREACH_CHILD } while (0)

/** Method Definitions */


/** Method Definitions */

mrb_value
mrb_argv0(mrb_state *mrb)
{
    TCHAR argv0[MAX_PATH + 1];

    GetModuleFileName(NULL, argv0, MAX_PATH + 1);

    return mrb_str_new_cstr(mrb, argv0);
}



static char*
argv_to_str(char* const* argv)
{
    char args[8191];
    int i     = 0;
    char* arg = argv[i];

    while (arg != NULL) {
        if (i == 0)
            sprintf(args, "%s", arg);
        else
            sprintf(args, "%s %s", args, arg);

        i++;
        arg = argv[i];
    }

    return strdup(args);
}

static WCHAR*
str_to_wstr(const char *utf8, int mlen)
{
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, mlen, NULL, 0);
    wchar_t* utf16 = (wchar_t*)malloc((wlen+1) * sizeof(wchar_t));

    if (utf16 == NULL)
        return NULL;

    if (MultiByteToWideChar(CP_UTF8, 0, utf8, mlen, utf16, wlen) > 0)
        utf16[wlen] = 0;

    return utf16;
}

off_t
lseek(int fd, mrb_int offset, mrb_int whence){
  /**
  * lseek is currently not defined under windows
  */
  return -1;
}

off_t
close(int fd){
  /**
  * close is currently not defined under windows
  */
  return -1;
}

off_t
isatty(int fd){
  /**
  * isatty is currently not defined under windows
  */
  return -1;
}

off_t
read(int fd, char *buf, mrb_int whence){
  /**
  * read is currently not defined under windows
  */
  return -1;
}

off_t
write(int fd, char *buf, mrb_int whence){
  /**
  * write is currently not defined under windows
  */
  return -1;
}
